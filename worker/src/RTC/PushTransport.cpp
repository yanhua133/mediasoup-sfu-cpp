#define MS_CLASS "RTC::PushTransport"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/PushTransport.hpp"
#include "Logger.hpp"
#include "MediaSoupErrors.hpp"
#include "Utils.hpp"
#include "Channel/Notifier.hpp"

extern "C" {
#include "libavutil/bprint.h"
#include "libavutil/opt.h"
#include "libavfilter/buffersink.h"
#include "libavfilter/buffersrc.h"
#include "libavcodec/bytestream.h"
}

static int ff_alloc_extradata(AVCodecParameters* par, int size)
{
	av_freep(&par->extradata);
	par->extradata_size = 0;

	if (size < 0 || size >= INT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
		return AVERROR(EINVAL);

	par->extradata = (uint8_t *)av_malloc(size + AV_INPUT_BUFFER_PADDING_SIZE);
	if (!par->extradata)
		return AVERROR(ENOMEM);

	memset(par->extradata + size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
	par->extradata_size = size;

	return 0;
}

static int opus_write_extradata(AVCodecParameters* codecpar)
{
	uint8_t* bs;
	int ret;

	/* This function writes an extradata with a channel mapping family of 0.
	 * This mapping family only supports mono and stereo layouts. And RFC7587
	 * specifies that the number of channels in the SDP must be 2.
	 */
	if (codecpar->channels > 2) {
		return AVERROR_INVALIDDATA;
	}

	ret = ff_alloc_extradata(codecpar, 19);
	if (ret < 0)
		return ret;

	bs = (uint8_t*)codecpar->extradata;

	/* Opus magic */
	bytestream_put_buffer(&bs, "OpusHead", 8);
	/* Version */
	bytestream_put_byte(&bs, 0x1);
	/* Channel count */
	bytestream_put_byte(&bs, codecpar->channels);
	/* Pre skip */
	bytestream_put_le16(&bs, 0);
	/* Input sample rate */
	bytestream_put_le32(&bs, 48000);
	/* Output gain */
	bytestream_put_le16(&bs, 0x0);
	/* Mapping family */
	bytestream_put_byte(&bs, 0x0);

	return 0;
}

namespace RTC
{
	/* Instance methods. */

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
	PushTransport::PushTransport(const std::string& id, RTC::Transport::Listener* listener, json& data)
	  : RTC::Transport::Transport(id, listener, data)
	{
		MS_TRACE();

		auto jsonComediaIt = data.find("comedia");

		if (jsonComediaIt != data.end())
		{
			if (!jsonComediaIt->is_boolean())
				MS_THROW_TYPE_ERROR("wrong comedia (not a boolean)");

			this->comedia = jsonComediaIt->get<bool>();
		}
	}

	PushTransport::~PushTransport()
	{
		MS_TRACE();

		this->connected = false;
	}

	void PushTransport::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Call the parent method.
		RTC::Transport::FillJson(jsonObject);

		// Add comedia.
		jsonObject["comedia"] = this->comedia;
	}

	void PushTransport::FillJsonStats(json& jsonArray)
	{
		MS_TRACE();

		// Call the parent method.
		RTC::Transport::FillJsonStats(jsonArray);

		auto& jsonObject = jsonArray[0];

		// Add type.
		jsonObject["type"] = "push-rtp-transport";

		// Add comedia.
		jsonObject["comedia"] = this->comedia;
	}

	void PushTransport::HandleRequest(Channel::Request* request)
	{
		MS_TRACE();

		switch (request->methodId)
		{
			case Channel::Request::MethodId::TRANSPORT_CONNECT:
			{
				// Ensure this method is not called twice.
				if (this->connectCalled)
					MS_THROW_ERROR("connect() already called");

				auto jsonIpIt = request->data.find("ip");

				if (jsonIpIt == request->data.end() || !jsonIpIt->is_string())
					MS_THROW_TYPE_ERROR("missing ip");

				auto ip = jsonIpIt->get<std::string>();

				// This may throw.
				Utils::IP::NormalizeIp(ip);

				uint16_t port{ 0u };
				auto jsonPortIt = request->data.find("port");

				if (
					jsonPortIt != request->data.end() &&
					Utils::Json::IsPositiveInteger(*jsonPortIt)
					)
				{
					port = jsonPortIt->get<uint16_t>();
				}

				auto jsonProtoIt = request->data.find("protoType");

				if (jsonProtoIt == request->data.end() || !jsonProtoIt->is_string())
					MS_THROW_TYPE_ERROR("missing protoType");

				auto type = jsonProtoIt->get<std::string>();

				if (type == RTMP_PROTO_NAME) {
					if (port == 0)
						port = RTMP_DEFAULT_PORT;

					std::string suffix = RTMP_DEFAULT_SUFFIX;
					auto jsonSuffixIt = request->data.find("suffix");

					if (jsonSuffixIt != request->data.end() && jsonSuffixIt->is_string())
						suffix = jsonSuffixIt->get<std::string>();

					RTMP_GENERATE_URL

					m_formatName = RTMP_PROTO_FORMAT_NAME;
				} 
				else {
					MS_THROW_ERROR("invalid protoType");
				}

				try
				{
					Connect();
				}
				catch (const MediaSoupError& error)
				{
					Disconnect();
					throw;
				}

				this->connectCalled = true;
				this->connected = true;

				// Tell the caller about the selected local DTLS role.
				json data = json::object();

		/*		if (this->tuple)
					this->tuple->FillJson(data["tuple"]);*/

				request->Accept(data);

				// Assume we are connected (there is no much more we can do to know it)
				// and tell the parent class.
				RTC::Transport::Connected();

				break;
			}

			default:
			{
				// Pass it to the parent class.
				RTC::Transport::HandleRequest(request);
			}
		}
	}

	void PushTransport::HandleNotification(PayloadChannel::Notification* notification)
	{
		MS_TRACE();

		// Pass it to the parent class.
		RTC::Transport::HandleNotification(notification);
	}

	inline bool PushTransport::IsConnected() const
	{
		return this->connected;
	}

	void PushTransport::SendRtpPacket(
	  RTC::Consumer* /*consumer*/, RTC::RtpPacket* packet, RTC::Transport::onSendCallback* cb)
	{
		MS_TRACE();

		if (!IsConnected())
		{
			if (cb)
			{
				(*cb)(false);

				delete cb;
			}

			return;
		}
		
		const uint8_t* data = packet->GetData();
		size_t len          = packet->GetSize();

		// Increase send transmission.
		RTC::Transport::DataSent(len);
	}

	void PushTransport::SendRtcpPacket(RTC::RTCP::Packet* packet)
	{
		MS_TRACE();		
	}

	void PushTransport::SendRtcpCompoundPacket(RTC::RTCP::CompoundPacket* packet)
	{
		MS_TRACE();
	}

	void PushTransport::SendMessage(
	  RTC::DataConsumer* dataConsumer, uint32_t ppid, const uint8_t* msg, size_t len, onQueuedCallback* cb)
	{
		MS_TRACE();
	}

	void PushTransport::SendSctpData(const uint8_t* data, size_t len)
	{
		MS_TRACE();
	}

	void PushTransport::RecvStreamClosed(uint32_t ssrc)
	{
		MS_TRACE();
	}

	void PushTransport::SendStreamClosed(uint32_t ssrc)
	{
		MS_TRACE();
	}

	inline void PushTransport::OnPacketReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		// Increase receive transmission.
		RTC::Transport::DataReceived(len);

		// Check if it's RTCP.
		if (RTC::RTCP::Packet::IsRtcp(data, len))
		{
			OnRtcpDataReceived(tuple, data, len);
		}
		// Check if it's RTP.
		else if (RTC::RtpPacket::IsRtp(data, len))
		{
			OnRtpDataReceived(tuple, data, len);
		}
		// Check if it's SCTP.
		else if (RTC::SctpAssociation::IsSctp(data, len))
		{
			OnSctpDataReceived(tuple, data, len);
		}
		else
		{
			MS_WARN_DEV("ignoring received packet of unknown type");
		}
	}

	inline void PushTransport::OnRtpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();
	}

	inline void PushTransport::OnRtcpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();
	}

	inline void PushTransport::OnSctpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();
	}

	void PushTransport::Disconnect() {
		connected = false;

		avio_closep(&m_context->pb);
		avformat_free_context(m_context);
		m_context = nullptr;
	}

	void PushTransport::InitIncomingParameters() {
		auto acodec = avcodec_find_decoder_by_name(m_audioDecoderName.c_str());
		if (!acodec) {
			MS_THROW_ERROR("error finding the audio decoder");
		}

		if(m_audioDecodeContext)
			avcodec_free_context(&m_audioDecodeContext);

		m_audioDecodeContext = avcodec_alloc_context3(acodec);
		if (!m_audioDecodeContext) {
			MS_THROW_ERROR("could not allocate a decoding context"); 
		}

		AVCodecParameters para;
		para.codec_type = AVMEDIA_TYPE_AUDIO;
		para.codec_id = ChooseAudioCodecId(m_audioDecoderName);
		para.channels = 1;
		para.channel_layout = 4; // maybe need fix
		para.sample_rate = m_audioSampleRate;
		para.format = AV_SAMPLE_FMT_FLTP;

		if (m_audioDecoderName == "opus")
			opus_write_extradata(&para);

		int ret = avcodec_parameters_to_context(m_audioDecodeContext, &para);
		if(ret < 0) {
			avcodec_free_context(&m_audioDecodeContext);
			m_audioDecodeContext = nullptr;
			MS_THROW_ERROR("error for avcodec_parameters_to_context");
		}

		ret = avcodec_open2(m_audioDecodeContext, acodec, NULL);
		if (ret < 0) {
			avcodec_free_context(&m_audioDecodeContext);
			m_audioDecodeContext = nullptr;
			MS_THROW_ERROR("could not open decoder");
		}
	}

	void PushTransport::InitOutgoingParameters() {
		avformat_alloc_output_context2(&m_context, NULL, m_formatName.c_str(), m_url.c_str());
		if (!m_context) {
			MS_THROW_ERROR("error allocating the avformat context");
		}

		AVStream* st = avformat_new_stream(m_context, NULL);

		st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;

		AVCodec* acodec = avcodec_find_encoder_by_name(RTMP_AUDIO_CODEC);
		if (!acodec) {
			MS_THROW_TYPE_ERROR("missing audio encoder");
		}
		
		m_audioEncodeContext = avcodec_alloc_context3(acodec);
		if (!m_audioEncodeContext) {
			MS_THROW_ERROR("error allocating the audio encoding context");
		}
		m_audioEncodeContext->codec_type = st->codecpar->codec_type;
		m_audioEncodeContext->channels = m_audioDecodeContext->channels;
		m_audioEncodeContext->channel_layout = m_audioDecodeContext->channel_layout;
		m_audioEncodeContext->sample_rate = m_audioDecodeContext->sample_rate;
		m_audioEncodeContext->sample_fmt = m_audioDecodeContext->sample_fmt;
		m_audioEncodeContext->bit_rate = m_audioDecodeContext->bit_rate;
		// for official ffmpeg aac encoder
		m_audioEncodeContext->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

		if (m_context->oformat->flags & AVFMT_GLOBALHEADER)
			m_audioEncodeContext->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		int ret = avcodec_open2(m_audioEncodeContext, acodec, NULL);
		if (ret < 0) {
			avcodec_free_context(&m_audioEncodeContext);
			m_audioEncodeContext = nullptr;
			MS_THROW_ERROR("could not open encoder");
		}

		ret = avcodec_parameters_from_context(st->codecpar, m_audioEncodeContext);
		if (ret < 0) {
			MS_THROW_ERROR("error for avcodec_parameters_to_context");
		}

		st->time_base.den = m_audioDecodeContext->sample_rate;
		st->time_base.num = 1;

		int ret = avio_open2(&m_context->pb, m_url.c_str(), AVIO_FLAG_WRITE, NULL, NULL);
		if (ret < 0) {
			MS_THROW_ERROR("error openning avio");
		}
	}

	AVCodecID PushTransport::ChooseAudioCodecId(std::string name) {
		if (name == "opus")
			return AV_CODEC_ID_OPUS;
		else
			return AV_CODEC_ID_NONE;
	}

	void PushTransport::Connect() {
		if (connected)
			return;

		InitIncomingParameters();

		InitOutgoingParameters();

		m_audioFifo = av_audio_fifo_alloc(m_audioEncodeContext->sample_fmt, m_audioEncodeContext->channels, 1);
		if (!m_audioFifo) {
			MS_THROW_ERROR("could not allocate fifo");
		}

		int ret = avformat_write_header(m_context, nullptr);
		if (ret < 0) {
			MS_THROW_ERROR("error writting header");
		}
	}
} // namespace RTC
