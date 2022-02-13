#define MS_CLASS "RTC::PushTransport"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/PushTransport.hpp"
#include "Logger.hpp"
#include "MediaSoupErrors.hpp"
#include "Utils.hpp"
#include "Channel/Notifier.hpp"

extern "C" {
#include "libavcodec/avcodec.h"
#include "libavcodec/bytestream.h"
#include "libavutil/bprint.h"
#include "libavutil/opt.h"
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

	std::string head = "OpusHead";
	bs = (uint8_t*)codecpar->extradata;

	/* Opus magic */
	bytestream_put_buffer(&bs, (uint8_t *)head.c_str(), 8);
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

		if(packet->GetPayloadType() == 100)
			AudioProcessPacket(packet);

		//const uint8_t* data = packet->GetData();
		//size_t len = packet->GetSize();

		// Increase send transmission.
		//RTC::Transport::DataSent(len);
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

		avio_closep(&m_audioFormatCtx->pb);
		avformat_free_context(m_audioFormatCtx);
		m_audioFormatCtx = nullptr;

		m_audioProcessPacket = false;
		m_audioRefTimestamp = 0;
	}

	void PushTransport::InitIncomingParameters() {
		auto acodec = avcodec_find_decoder_by_name(m_audioDecoderName.c_str());
		if (!acodec) {
			MS_THROW_ERROR("error finding the audio decoder");
		}

		if(m_audioDecodeCtx)
			avcodec_free_context(&m_audioDecodeCtx);

		m_audioDecodeCtx = avcodec_alloc_context3(acodec);
		if (!m_audioDecodeCtx) {
			MS_THROW_ERROR("could not allocate a decoding context"); 
		}

		AVCodecParameters *para = avcodec_parameters_alloc();
		para->codec_type = AVMEDIA_TYPE_AUDIO;
		para->codec_id = ChooseAudioCodecId(m_audioDecoderName);
		para->channels = 1;
		para->channel_layout = 4; // maybe need fix
		para->sample_rate = m_audioSampleRate;
		para->format = AV_SAMPLE_FMT_FLTP;

		if (m_audioDecoderName == "opus")
			opus_write_extradata(para);

		int ret = avcodec_parameters_to_context(m_audioDecodeCtx, para);
		if(ret < 0) {
			avcodec_free_context(&m_audioDecodeCtx);
			m_audioDecodeCtx = nullptr;
			MS_THROW_ERROR("error for avcodec_parameters_to_context");
		}
		avcodec_parameters_free(&para);

		ret = avcodec_open2(m_audioDecodeCtx, acodec, NULL);
		if (ret < 0) {
			avcodec_free_context(&m_audioDecodeCtx);
			m_audioDecodeCtx = nullptr;
			MS_THROW_ERROR("could not open decoder");
		}
	}

	void PushTransport::InitOutgoingParameters() {
		avformat_alloc_output_context2(&m_audioFormatCtx, NULL, m_formatName.c_str(), m_url.c_str());
		if (!m_audioFormatCtx) {
			MS_THROW_ERROR("error allocating the avformat context");
		}

		auto st = avformat_new_stream(m_audioFormatCtx, NULL);

		m_audioIdx = m_audioFormatCtx->nb_streams - 1;

		st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;

		auto acodec = avcodec_find_encoder_by_name(RTMP_AUDIO_CODEC);
		if (!acodec) {
			MS_THROW_TYPE_ERROR("missing audio encoder");
		}
		
		m_audioEncodeCtx = avcodec_alloc_context3(acodec);
		if (!m_audioEncodeCtx) {
			MS_THROW_ERROR("error allocating the audio encoding context");
		}
		m_audioEncodeCtx->codec_type = st->codecpar->codec_type;
		m_audioEncodeCtx->channels = m_audioDecodeCtx->channels;
		m_audioEncodeCtx->channel_layout = m_audioDecodeCtx->channel_layout;
		m_audioEncodeCtx->sample_rate = m_audioDecodeCtx->sample_rate;
		m_audioEncodeCtx->sample_fmt = m_audioDecodeCtx->sample_fmt;
		m_audioEncodeCtx->bit_rate = m_audioDecodeCtx->bit_rate;
		// for official ffmpeg aac encoder
		m_audioEncodeCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

		if (m_audioFormatCtx->oformat->flags & AVFMT_GLOBALHEADER)
			m_audioEncodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		av_opt_set(m_audioEncodeCtx->priv_data, "tune", "zerolatency", 0);

		int ret = avcodec_open2(m_audioEncodeCtx, acodec, NULL);
		if (ret < 0) {
			avcodec_free_context(&m_audioEncodeCtx);
			m_audioEncodeCtx = nullptr;
			MS_THROW_ERROR("could not open encoder");
		}

		m_audioMuteFrame = av_frame_alloc();
		m_audioMuteFrame->sample_rate = m_audioEncodeCtx->sample_rate;
		m_audioMuteFrame->format = m_audioEncodeCtx->sample_fmt;
		m_audioMuteFrame->channel_layout = m_audioEncodeCtx->channel_layout;
		m_audioMuteFrame->channels = m_audioEncodeCtx->channels;
		m_audioMuteFrame->nb_samples = m_audioEncodeCtx->frame_size;
		ret = av_frame_get_buffer(m_audioMuteFrame, 0);
		if (ret < 0) {
			avcodec_free_context(&m_audioEncodeCtx);
			m_audioEncodeCtx = nullptr;
			MS_THROW_ERROR("could not create mute frame");
		}
		av_samples_set_silence(m_audioMuteFrame->data, 0, m_audioMuteFrame->nb_samples, m_audioMuteFrame->channels, (AVSampleFormat)m_audioMuteFrame->format);


		ret = avcodec_parameters_from_context(st->codecpar, m_audioEncodeCtx);
		if (ret < 0) {
			MS_THROW_ERROR("error for avcodec_parameters_to_context");
		}

		st->time_base.den = m_audioDecodeCtx->sample_rate;
		st->time_base.num = 1;

		ret = avio_open2(&(m_audioFormatCtx->pb), m_url.c_str(), AVIO_FLAG_WRITE, NULL, NULL);
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

		m_audioFifo = av_audio_fifo_alloc(m_audioEncodeCtx->sample_fmt, m_audioEncodeCtx->channels, 1);
		if (!m_audioFifo) {
			MS_THROW_ERROR("could not allocate fifo");
		}

		int ret = avformat_write_header(m_audioFormatCtx, nullptr);
		if (ret < 0) {
			MS_THROW_ERROR("error writting header");
		}
	}

	void PushTransport::AudioProcessPacket(RTC::RtpPacket* packet) {
		if (!m_audioProcessPacket) {
			m_audioRefTimestamp = packet->GetTimestamp();
			m_audioNextTimestamp = 0;
			m_audioProcessPacket = true;
		}

		m_audioCurTimestamp = packet->GetTimestamp() - m_audioRefTimestamp; 
		if (m_audioCurTimestamp < 0) {
			m_audioCurTimestamp += UINT32_MAX;
		}

		auto cmp = m_audioCurTimestamp - m_audioNextTimestamp;		
		if (cmp < 0)
			return;
		else if(cmp > 0){
			int size = av_audio_fifo_size(m_audioFifo);
			int needSamples = m_audioEncodeCtx->frame_size - size;
			if (cmp >= needSamples) {
				av_audio_fifo_write(m_audioFifo, (void**)m_audioMuteFrame->extended_data, needSamples);
				m_audioNextTimestamp = m_audioCurTimestamp;
				m_audioPtsTimestamp = m_audioCurTimestamp;
			}
			else {
				cmp = cmp; //fixme
			}
			AudioEncodeAndSend();
			//PacketFree();
		}
		
		int ret = AudioDecodeAndFifo(packet);
		PacketFree();
		if (ret < 0)
			return;

		AudioEncodeAndSend();
		PacketFree();
	}

	int PushTransport::AudioDecodeAndFifo(RTC::RtpPacket* packet) {
		m_packet = av_packet_alloc();
		if (!m_packet)
			return -1;

		m_packet->size = packet->GetPayloadLength();
		m_packet->data = (uint8_t*)malloc(m_packet->size);
		memcpy(m_packet->data, packet->GetPayload(), m_packet->size);

		int ret = avcodec_send_packet(m_audioDecodeCtx, m_packet);
		if (ret < 0)
			return -1;

		m_frame = av_frame_alloc();
		if (!m_frame)
			return -1;

		ret = avcodec_receive_frame(m_audioDecodeCtx, m_frame);
		if (ret < 0)
			return -1;

		ret = av_audio_fifo_realloc(m_audioFifo, av_audio_fifo_size(m_audioFifo) + m_frame->nb_samples);
		if (ret < 0)
			return -1;

		ret = av_audio_fifo_write(m_audioFifo, (void**)m_frame->extended_data, m_frame->nb_samples);
		if (ret < m_frame->nb_samples)
			return -1;

		m_audioNextTimestamp += m_frame->nb_samples;
		
		return 0;
	}

	int PushTransport::AudioEncodeAndSend() {
		auto size = av_audio_fifo_size(m_audioFifo);
		while (av_audio_fifo_size(m_audioFifo) >= m_audioEncodeCtx->frame_size) {
			m_frame = av_frame_alloc();
			if (!m_frame)
				return -1;

			m_frame->nb_samples = m_audioEncodeCtx->frame_size;
			m_frame->channel_layout = m_audioEncodeCtx->channel_layout;
			m_frame->format = m_audioEncodeCtx->sample_fmt;
			m_frame->sample_rate = m_audioEncodeCtx->sample_rate;

			int ret = av_frame_get_buffer(m_frame, 0);
			if (ret < 0)
				return -1;

			ret = av_audio_fifo_read(m_audioFifo, (void**)m_frame->data, m_audioEncodeCtx->frame_size);
			if (ret < m_audioEncodeCtx->frame_size)
				return -1;

			m_frame->pts = m_audioPtsTimestamp;
			m_audioPtsTimestamp += m_audioEncodeCtx->frame_size;

			ret = avcodec_send_frame(m_audioEncodeCtx, m_frame);
			if (ret < 0)
				return -1;

			m_packet = av_packet_alloc();
			if (!m_packet)
				return -1;

			ret = avcodec_receive_packet(m_audioEncodeCtx, m_packet);
			if (ret < 0)
				return -1;

			av_packet_rescale_ts(m_packet, m_audioEncodeCtx->time_base, m_audioFormatCtx->streams[m_audioIdx]->time_base);

			ret = av_write_frame(m_audioFormatCtx, m_packet);
			if (ret < 0)
				return -1;

			PacketFree();
		}

		return 0;
	}

	void PushTransport::PacketFree() {
		if (m_packet)
			av_packet_free(&m_packet);
		if (m_frame)
			av_frame_free(&m_frame);
	}
} // namespace RTC
