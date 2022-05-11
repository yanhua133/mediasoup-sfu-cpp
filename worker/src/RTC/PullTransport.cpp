#define MS_CLASS "RTC::PullTransport"
// #define MS_LOG_DEV_LEVEL 3
#define FFMPEG_TIMEOUT_SEC 5
#define LISTEN_IP "0.0.0.0"

#include "RTC/PullTransport.hpp"
#include "Logger.hpp"
#include "MediaSoupErrors.hpp"
#include "Utils.hpp"
#include "Channel/Notifier.hpp"

extern "C" {
#include "libavformat/avformat.h"
#include "libavcodec/avcodec.h"
#include "libavcodec/bytestream.h"
#include "libavutil/bprint.h"
#include "libavutil/opt.h"
}
//
//static int ff_alloc_extradata(AVCodecParameters* par, int size)
//{
//	av_freep(&par->extradata);
//	par->extradata_size = 0;
//
//	if (size < 0 || size >= INT32_MAX - AV_INPUT_BUFFER_PADDING_SIZE)
//		return AVERROR(EINVAL);
//
//	par->extradata = (uint8_t *)av_malloc(size + AV_INPUT_BUFFER_PADDING_SIZE);
//	if (!par->extradata)
//		return AVERROR(ENOMEM);
//
//	memset(par->extradata + size, 0, AV_INPUT_BUFFER_PADDING_SIZE);
//	par->extradata_size = size;
//
//	return 0;
//}
//
//static int opus_write_extradata(AVCodecParameters* codecpar)
//{
//	uint8_t* bs;
//	int ret;
//
//	/* This function writes an extradata with a channel mapping family of 0.
//	 * This mapping family only supports mono and stereo layouts. And RFC7587
//	 * specifies that the number of channels in the SDP must be 2.
//	 */
//	if (codecpar->channels > 2) {
//		return AVERROR_INVALIDDATA;
//	}
//
//	ret = ff_alloc_extradata(codecpar, 19);
//	if (ret < 0)
//		return ret;
//
//	std::string head = "OpusHead";
//	bs = (uint8_t*)codecpar->extradata;
//
//	/* Opus magic */
//	bytestream_put_buffer(&bs, (uint8_t *)head.c_str(), 8);
//	/* Version */
//	bytestream_put_byte(&bs, 0x1);
//	/* Channel count */
//	bytestream_put_byte(&bs, codecpar->channels);
//	/* Pre skip */
//	bytestream_put_le16(&bs, 0);
//	/* Input sample rate */
//	bytestream_put_le32(&bs, 48000);
//	/* Output gain */
//	bytestream_put_le16(&bs, 0x0);
//	/* Mapping family */
//	bytestream_put_byte(&bs, 0x0);
//
//	return 0;
//}

namespace RTC
{
	/* Instance methods. */

	// NOLINTNEXTLINE(cppcoreguidelines-pro-type-member-init)
	PullTransport::PullTransport(const std::string& id, RTC::Transport::Listener* listener, json& data)
	  : RTC::Transport::Transport(id, listener, data)
	{
		MS_TRACE();

		auto jsonRtcpMuxIt = data.find("rtcpMux");

		if (jsonRtcpMuxIt != data.end())
		{
			if (!jsonRtcpMuxIt->is_boolean())
				MS_THROW_TYPE_ERROR("wrong rtcpMux (not a boolean)");

			this->rtcpMux = jsonRtcpMuxIt->get<bool>();
		}

		auto jsonComediaIt = data.find("comedia");

		if (jsonComediaIt != data.end())
		{
			if (!jsonComediaIt->is_boolean())
				MS_THROW_TYPE_ERROR("wrong comedia (not a boolean)");

			this->comedia = jsonComediaIt->get<bool>();
		}

		std::string ip = LISTEN_IP;

		try
		{
			// This may throw.
			this->udpSocket = new RTC::UdpSocket(this, ip);

			if (!this->rtcpMux)
			{
				// This may throw.
				this->rtcpUdpSocket = new RTC::UdpSocket(this, ip);
			}
		}
		catch (const MediaSoupError& error)
		{
			delete this->udpSocket;
			this->udpSocket = nullptr;

			delete this->rtcpUdpSocket;
			this->rtcpUdpSocket = nullptr;

			throw;
		}
	}

	PullTransport::~PullTransport()
	{
		MS_TRACE();

		delete this->udpSocket;
		this->udpSocket = nullptr;

		delete this->rtcpUdpSocket;
		this->rtcpUdpSocket = nullptr;

		delete this->tuple;
		this->tuple = nullptr;

		delete this->rtcpTuple;
		this->rtcpTuple = nullptr;

		this->connected = false;
	}

	void PullTransport::FillJson(json& jsonObject) const
	{
		MS_TRACE();

		// Call the parent method.
		RTC::Transport::FillJson(jsonObject);

		// Add comedia.
		jsonObject["comedia"] = this->comedia;
	}

	void PullTransport::FillJsonStats(json& jsonArray)
	{
		MS_TRACE();

		// Call the parent method.
		RTC::Transport::FillJsonStats(jsonArray);

		auto& jsonObject = jsonArray[0];

		// Add type.
		jsonObject["type"] = "pull-rtp-transport";

		// Add comedia.
		jsonObject["comedia"] = this->comedia;
	}

	void PullTransport::HandleRequest(Channel::Request* request)
	{
		MS_TRACE();

		switch (request->methodId)
		{
			case Channel::Request::MethodId::TRANSPORT_CONNECT:
			{
				MS_THROW_ERROR("connect() already called");
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

					auto jsonStreamIt = request->data.find("stream");

					if (jsonStreamIt == request->data.end() || !jsonStreamIt->is_string())
						MS_THROW_TYPE_ERROR("missing stream");

					auto stream = jsonStreamIt->get<std::string>();

					RTMP_GENERATE_URL(m_inputUrl, ip.c_str(), port, suffix.c_str(), stream.c_str());
				} 
				else {
					MS_THROW_ERROR("invalid protoType");
				}

				try
				{
					//Connect();
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
				data["123"] = 5;
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

	void PullTransport::HandleNotification(PayloadChannel::Notification* notification)
	{
		MS_TRACE();

		// Pass it to the parent class.
		RTC::Transport::HandleNotification(notification);
	}

	inline bool PullTransport::IsConnected() const
	{
		return this->connected;
	}

	void PullTransport::SendRtpPacket(
	  RTC::Consumer* /*consumer*/, RTC::RtpPacket* packet, RTC::Transport::onSendCallback* cb)
	{
		MS_TRACE();
	}

	void PullTransport::SendRtcpPacket(RTC::RTCP::Packet* packet)
	{
		MS_TRACE();		
	}

	void PullTransport::SendRtcpCompoundPacket(RTC::RTCP::CompoundPacket* packet)
	{
		MS_TRACE();
	}

	void PullTransport::SendMessage(
	  RTC::DataConsumer* dataConsumer, uint32_t ppid, const uint8_t* msg, size_t len, onQueuedCallback* cb)
	{
		MS_TRACE();
	}

	void PullTransport::SendSctpData(const uint8_t* data, size_t len)
	{
		MS_TRACE();
	}

	void PullTransport::RecvStreamClosed(uint32_t ssrc)
	{
		MS_TRACE();
	}

	void PullTransport::SendStreamClosed(uint32_t ssrc)
	{
		MS_TRACE();
	}

	inline void PullTransport::OnPacketReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
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
		else
		{
			MS_WARN_DEV("ignoring received packet of unknown type");
		}
	}

	inline void PullTransport::OnRtpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();

		RTC::RtpPacket* packet = RTC::RtpPacket::Parse(data, len);

		if (!packet)
		{
			MS_WARN_TAG(rtp, "received data is not a valid RTP packet");

			return;
		}

		// If we don't have a RTP tuple yet, check whether comedia mode is set.
		if (!this->tuple)
		{
			if (!this->comedia)
			{
				MS_DEBUG_TAG(rtp, "ignoring RTP packet while not connected");

				// Remove this SSRC.
				RecvStreamClosed(packet->GetSsrc());

				delete packet;

				return;
			}

			MS_DEBUG_TAG(rtp, "setting RTP tuple (comedia mode enabled)");

			auto wasConnected = IsConnected();

			this->tuple = new RTC::TransportTuple(tuple);


			// If not yet connected do it now.
			if (!wasConnected)
			{
				// Notify the Node PlainTransport.
				json data = json::object();

				this->tuple->FillJson(data["tuple"]);

				Channel::Notifier::Emit(this->id, "tuple", data);

				RTC::Transport::Connected();
			}
		}
		// Otherwise, if RTP tuple is set, verify that it matches the origin
		// of the packet.
		else if (!this->tuple->Compare(tuple))
		{
			MS_DEBUG_TAG(rtp, "ignoring RTP packet from unknown IP:port");

			// Remove this SSRC.
			RecvStreamClosed(packet->GetSsrc());

			delete packet;

			return;
		}
		
		/*uint8_t * p = packet->GetPayload();
		if (p[0] == 0x18) {
			size_t plen = (p[1] << 8) + p[2] + 5;
			packet->ShiftPayload(0, plen, false);
			size_t newlen = packet->GetSize();
			delete packet;
			packet = RTC::RtpPacket::Parse(data, newlen);
		}*/

		// Pass the packet to the parent transport.
		RTC::Transport::ReceiveRtpPacket(packet);
	}

	inline void PullTransport::OnRtcpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();
	}

	inline void PullTransport::OnSctpDataReceived(
	  RTC::TransportTuple* tuple, const uint8_t* data, size_t len)
	{
		MS_TRACE();
	}

	inline void PullTransport::OnUdpSocketPacketReceived(
		RTC::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr)
	{
		MS_TRACE();

		RTC::TransportTuple tuple(socket, remoteAddr);

		OnPacketReceived(&tuple, data, len);
	}

	void PullTransport::Disconnect() {
		connected = false;

		avio_closep(&m_inputCtx->pb);
		avformat_free_context(m_inputCtx);
		m_inputCtx = nullptr;

		avio_closep(&m_outputCtx->pb);
		avformat_free_context(m_outputCtx);
		m_outputCtx = nullptr;

	/*	m_audioProcessPacket = false;
		m_audioRefTimestamp = 0;*/
	}

	void PullTransport::InitIncomingParameters() {
		InitVideoStream();
		InitAudioStream();
	}

	void PullTransport::InitVideoStream() {
		auto codec = avcodec_find_decoder_by_name(m_videoDecoderName.c_str());
		if (!codec) {
			MS_THROW_ERROR("error finding the video decoder");
		}

		if (m_videoDecodeCtx)
			avcodec_free_context(&m_videoDecodeCtx);

		m_videoDecodeCtx = avcodec_alloc_context3(codec);
		if (!m_videoDecodeCtx) {
			MS_THROW_ERROR("could not allocate a decoding context");
		}

		AVCodecParameters* para = avcodec_parameters_alloc();
		para->codec_type = AVMEDIA_TYPE_VIDEO;
		para->codec_id = ChooseVideoCodecId(m_videoDecoderName);

		int ret = avcodec_parameters_to_context(m_videoDecodeCtx, para);
		if (ret < 0) {
			avcodec_free_context(&m_videoDecodeCtx);
			m_videoDecodeCtx = nullptr;
			MS_THROW_ERROR("error for avcodec_parameters_to_context");
		}
		avcodec_parameters_free(&para);
		m_videoDecodeCtx->time_base.den = m_videoRateClock;
		m_videoDecodeCtx->time_base.num = 1;

		m_videoDecodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;		

		ret = avcodec_open2(m_videoDecodeCtx, codec, NULL);
		if (ret < 0) {
			avcodec_free_context(&m_videoDecodeCtx);
			m_videoDecodeCtx = nullptr;
			MS_THROW_ERROR("could not open decoder");
		}
	}

	void PullTransport::InitAudioStream() {
		//auto codec = avcodec_find_decoder_by_name(m_audioDecoderName.c_str());
		//if (!codec) {
		//	MS_THROW_ERROR("error finding the audio decoder");
		//}

		//if (m_audioDecodeCtx)
		//	avcodec_free_context(&m_audioDecodeCtx);

		//m_audioDecodeCtx = avcodec_alloc_context3(codec);
		//if (!m_audioDecodeCtx) {
		//	MS_THROW_ERROR("could not allocate a decoding context");
		//}

		//AVCodecParameters* para = avcodec_parameters_alloc();
		//para->codec_type = AVMEDIA_TYPE_AUDIO;
		//para->codec_id = ChooseAudioCodecId(m_audioDecoderName);
		//para->channels = 1;
		//para->channel_layout = 4; // maybe need fix
		//para->sample_rate = m_audioSampleRate;
		//para->format = AV_SAMPLE_FMT_FLTP;

		//if (m_audioDecoderName == "opus")
		//	opus_write_extradata(para);

		//int ret = avcodec_parameters_to_context(m_audioDecodeCtx, para);
		//if (ret < 0) {
		//	avcodec_free_context(&m_audioDecodeCtx);
		//	m_audioDecodeCtx = nullptr;
		//	MS_THROW_ERROR("error for avcodec_parameters_to_context");
		//}
		//avcodec_parameters_free(&para);

		//ret = avcodec_open2(m_audioDecodeCtx, codec, NULL);
		//if (ret < 0) {
		//	avcodec_free_context(&m_audioDecodeCtx);
		//	m_audioDecodeCtx = nullptr;
		//	MS_THROW_ERROR("could not open decoder");
		//}
	}

	void PullTransport::InitOutgoingParameters() {
		avformat_alloc_output_context2(&m_outputCtx, NULL, RTP_PROTO_FORMAT_NAME, m_outputUrl);
		if (!m_outputCtx) {
			MS_THROW_ERROR("error allocating the avformat context");
		}

		AVStream *st = avformat_new_stream(m_outputCtx, NULL);

		m_videoIdx = m_outputCtx->nb_streams - 1;

		st->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;

		int ret = avcodec_parameters_from_context(st->codecpar, m_videoDecodeCtx);
		if (ret < 0) {
			MS_THROW_ERROR("error for avcodec_parameters_to_context");
		}

		st->time_base.den = m_videoRateClock;
		st->time_base.num = 1;

		//st = avformat_new_stream(m_outputCtx, NULL);

		//m_audioIdx = m_formatCtx->nb_streams - 1;

		//st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;

		//auto acodec = avcodec_find_encoder_by_name(RTMP_AUDIO_CODEC);
		//if (!acodec) {
		//	MS_THROW_TYPE_ERROR("missing audio encoder");
		//}
		//
		//m_audioEncodeCtx = avcodec_alloc_context3(acodec);
		//if (!m_audioEncodeCtx) {
		//	MS_THROW_ERROR("error allocating the audio encoding context");
		//}
		//m_audioEncodeCtx->codec_type = st->codecpar->codec_type;
		//m_audioEncodeCtx->channels = m_audioDecodeCtx->channels;
		//m_audioEncodeCtx->channel_layout = m_audioDecodeCtx->channel_layout;
		//m_audioEncodeCtx->sample_rate = m_audioDecodeCtx->sample_rate;
		//m_audioEncodeCtx->sample_fmt = m_audioDecodeCtx->sample_fmt;
		//m_audioEncodeCtx->bit_rate = m_audioDecodeCtx->bit_rate;
		//// for official ffmpeg aac encoder
		//m_audioEncodeCtx->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;

		//if (m_formatCtx->oformat->flags & AVFMT_GLOBALHEADER)
		//	m_audioEncodeCtx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		//ret = avcodec_open2(m_audioEncodeCtx, acodec, NULL);
		//if (ret < 0) {
		//	avcodec_free_context(&m_audioEncodeCtx);
		//	m_audioEncodeCtx = nullptr;
		//	MS_THROW_ERROR("could not open encoder");
		//}

		//m_audioMuteFrame = av_frame_alloc();
		//m_audioMuteFrame->sample_rate = m_audioEncodeCtx->sample_rate;
		//m_audioMuteFrame->format = m_audioEncodeCtx->sample_fmt;
		//m_audioMuteFrame->channel_layout = m_audioEncodeCtx->channel_layout;
		//m_audioMuteFrame->channels = m_audioEncodeCtx->channels;
		//m_audioMuteFrame->nb_samples = m_audioEncodeCtx->frame_size;
		//ret = av_frame_get_buffer(m_audioMuteFrame, 0);
		//if (ret < 0) {
		//	avcodec_free_context(&m_audioEncodeCtx);
		//	m_audioEncodeCtx = nullptr;
		//	MS_THROW_ERROR("could not create mute frame");
		//}
		//av_samples_set_silence(m_audioMuteFrame->data, 0, m_audioMuteFrame->nb_samples, m_audioMuteFrame->channels, (AVSampleFormat)m_audioMuteFrame->format);


		//ret = avcodec_parameters_from_context(st->codecpar, m_audioEncodeCtx);
		//if (ret < 0) {
		//	MS_THROW_ERROR("error for avcodec_parameters_to_context");
		//}

		//st->time_base.den = m_audioDecodeCtx->sample_rate;
		//st->time_base.num = 1;

		ret = avio_open2(&(m_outputCtx->pb), m_outputUrl, AVIO_FLAG_WRITE, NULL, NULL);
		if (ret < 0) {
			MS_THROW_ERROR("error openning avio");
		}
	}

	AVCodecID PullTransport::ChooseVideoCodecId(std::string name) {
		if (name == "h264")
			return AV_CODEC_ID_H264;
		else
			return AV_CODEC_ID_NONE;
	}

	AVCodecID PullTransport::ChooseAudioCodecId(std::string name) {
		if (name == "opus")
			return AV_CODEC_ID_OPUS;
		else
			return AV_CODEC_ID_NONE;
	}

	static int interrupt_callback(void* p) {
		time_t *t = (time_t*)p;
		if (*t > 0) {
			if (time(NULL) - *t > FFMPEG_TIMEOUT_SEC) {
				return 1;
			}
		}

		return 0;
	}

	void PullTransport::ThreadError(int id) {

	}

	void PullTransport::Connect() {
		if (connected)
			return;

		if (m_pThread)
			MS_THROW_ERROR("pull thread error");

		auto pullThread = [this]() {
			time_t input_time = time(NULL);
			m_inputCtx = avformat_alloc_context();
			m_inputCtx->interrupt_callback.callback = interrupt_callback;
			m_inputCtx->interrupt_callback.opaque = &input_time;

			int ret = avformat_open_input(&m_inputCtx, this->m_inputUrl, NULL, NULL);
			if (ret < 0)
			{
				ThreadError(1);
				return;
			}

			ret = avformat_find_stream_info(m_inputCtx, NULL);
			if (ret < 0)
			{
				ThreadError(2);
				return;
			}

			AVPacket* pkt = av_packet_alloc();
			while (1) {
				ret = av_read_frame(m_inputCtx, pkt);
				if (ret < 0) {
					ThreadError(3);
					break;
				}
			}
			
		};

		m_pThread = new std::thread(pullThread);


		/*InitIncomingParameters();

		InitOutgoingParameters();

		m_audioFifo = av_audio_fifo_alloc(m_audioEncodeCtx->sample_fmt, m_audioEncodeCtx->channels, 1);
		if (!m_audioFifo) {
			MS_THROW_ERROR("could not allocate fifo");
		}*/

		/*int ret = avformat_write_header(m_formatCtx, nullptr);
		if (ret < 0) {
			MS_THROW_ERROR("error writting header");
		}*/
	}

	void PullTransport::AudioProcessPacket(RTC::RtpPacket* packet) {
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
				AudioEncodeAndSend();
				PacketFree();
				m_audioNextTimestamp = m_audioCurTimestamp;
				m_audioPtsTimestamp = m_audioCurTimestamp;
			}
			else {
				av_audio_fifo_write(m_audioFifo, (void**)m_audioMuteFrame->extended_data, cmp);
				m_audioNextTimestamp = m_audioCurTimestamp;
			}
			
		}
		
		if (AudioDecodeAndFifo(packet) >= 0) {
			AudioEncodeAndSend();
		}
		PacketFree();
	}

	void PullTransport::VideoProcessPacket(RTC::RtpPacket* packet) {		
		ProbePayload(packet);

		if (m_videoSendData) {
			VideoSend();
			PacketFree();
			m_videoSendData = false;
		}
	}

	int PullTransport::AudioDecodeAndFifo(RTC::RtpPacket* packet) {
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

	int PullTransport::AudioEncodeAndSend() {
		/*auto size = av_audio_fifo_size(m_audioFifo);
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

			av_packet_rescale_ts(m_packet, m_audioEncodeCtx->time_base, m_formatCtx->streams[m_audioIdx]->time_base);
			m_packet->stream_index = m_audioIdx;

			ret = av_write_frame(m_formatCtx, m_packet);
			if (ret < 0)
				return -1;

			PacketFree();
		}*/

		return 0;
	}

	void PullTransport::PacketFree() {
		if (m_packet)
			av_packet_free(&m_packet);
		if (m_frame)
			av_frame_free(&m_frame);
	}

	void PullTransport::ProbePayload(RTC::RtpPacket* packet) {
		if (packet->GetPayloadLength() <= 1) return;

		const uint8_t* data = packet->GetPayload();
		int rtpType = data[0] & 0x1F;
		switch (rtpType)
		{
		case 24: // STAP-A
			ProcessPayloadStapA(packet);
			return;
		case 28: // FU-A
			ProcessPayloadFuA(packet);
			return;
		default:
			return;
		}
	}

	void PullTransport::ProcessPayloadStapA(RTC::RtpPacket* packet) {
		if (m_videoSpsPacket) return;//for test;
		uint8_t *src = packet->GetPayload() + 1;
		size_t srcLen = packet->GetPayloadLength() - 1;
		uint8_t *tgt = nullptr;
		size_t tgtLen = 0;
		int sLen, tLen, error;

		while (true) {
			if (srcLen == 0) {
				error = 0;
				break;
			}
			else if (srcLen <= 2) {
				error = 1;
				break;
			}
		
			sLen = (src[0] << 8) + src[1];
			srcLen -= 2;
			src += 2;

			if (sLen > srcLen) {
				error = 1;
				break;
			}
			tLen = tgtLen;
			tgtLen += sLen + 4;

			if (tgt)
				tgt = (uint8_t*)realloc(tgt, tgtLen);
			else
				tgt = (uint8_t*)malloc(tgtLen);

			tgt[tLen + 0] = 0x00;
			tgt[tLen + 1] = 0x00;
			tgt[tLen + 2] = 0x00;
			tgt[tLen + 3] = 0x01;
			memcpy(tgt + tLen + 4, src, sLen);

			src += sLen;
			srcLen -= sLen;
		}

		if (error == 0 && tgtLen > 0) {
			if (m_videoSpsPacket)
				free(m_videoSpsPacket);
			m_videoSpsPacket = tgt;
			m_videoSpsPacketLen = tgtLen;
			m_videoUpdateSps = true;
		}
		else {
			free(tgt);
		}
	}

	void PullTransport::ProcessPayloadFuA(RTC::RtpPacket* packet) {
		size_t srcLen = packet->GetPayloadLength();
		if (srcLen <= 2) return;

		uint8_t* src = packet->GetPayload();
		int h1 = src[0] & 0xE0;
		int h2 = src[1] & 0x1F;
		int header = h1 + h2;
		int start = src[1] & 0x80;
		int end = src[1] & 0x40;

		if (!m_videoSpsPacket) return;
		else if (m_videoUpdateSps) {
			if (header != 0x65) { 
				// I-frame maybe same as IDR?
				return;
			}
		}

		if (start != 0) {
			if (m_videoCurPacket) 
				free(m_videoCurPacket);

			m_videoCurPacketTs = packet->GetTimestamp();
			m_videoCurSeqNumber = packet->GetSequenceNumber();
			m_videoCurPacketLen = packet->GetPayloadLength() + 3; // (sc)4 + (hearder)1 + data(len - 2)
			m_videoCurPacket = (uint8_t*)malloc(m_videoCurPacketLen);
			m_videoCurPacket[0] = 0x00;
			m_videoCurPacket[1] = 0x00;
			m_videoCurPacket[2] = 0x00;
			m_videoCurPacket[3] = 0x01;
			m_videoCurPacket[4] = (uint8_t)header;
			memcpy(m_videoCurPacket + 5, src + 2, m_videoCurPacketLen - 5);
		}
		else {
			uint16_t sn = packet->GetSequenceNumber();
			if (sn != m_videoCurSeqNumber + 1) return;
			if (m_videoCurPacketTs != packet->GetTimestamp()) return;
			m_videoCurSeqNumber = sn;
						
			size_t len = m_videoCurPacketLen;
			m_videoCurPacketLen += srcLen - 2;
			m_videoCurPacket = (uint8_t*)realloc(m_videoCurPacket, m_videoCurPacketLen);
			memcpy(m_videoCurPacket + len, src + 2, srcLen - 2);

			if (end != 0) {
				m_videoSendData = true;
				if (m_videoRefTimestamp == 0)
					m_videoRefTimestamp = m_videoCurPacketTs;
			}
		}		
	}

	int PullTransport::VideoSend() {
		//int ret;
		//m_packet = av_packet_alloc();
		//if (!m_packet)
		//	return -1;

		//
		//if (m_videoUpdateSps) {
		//	m_packet->size = m_videoSpsPacketLen + m_videoCurPacketLen;
		//	m_packet->data = (uint8_t*)malloc(m_packet->size);
		//	memcpy(m_packet->data, m_videoSpsPacket, m_videoSpsPacketLen);
		//	memcpy(m_packet->data + m_videoSpsPacketLen, m_videoCurPacket, m_videoCurPacketLen);
		//}
		//else {
		//	m_packet->size = m_videoCurPacketLen;
		//	m_packet->data = m_videoCurPacket;
		//}
		//m_packet->pts = m_videoCurPacketTs - m_videoRefTimestamp;
		//if (m_packet->pts < 0) {
		//	m_packet->pts += UINT32_MAX;
		//}
		//m_packet->dts = m_packet->pts;

		//if (m_videoUpdateSps) {
		//	ret = TryDecodeFrame();
		//	if (ret < 0) return -1;

		//	m_videoDecodeCtx->extradata_size = m_videoSpsPacketLen;
		//	m_videoDecodeCtx->extradata = (uint8_t*)malloc(m_videoDecodeCtx->extradata_size);
		//	memset(m_videoDecodeCtx->extradata, 0, m_videoDecodeCtx->extradata_size);
		//	memcpy(m_videoDecodeCtx->extradata, m_videoSpsPacket + 1, m_videoDecodeCtx->extradata_size - 1);

		//	avcodec_parameters_from_context(m_formatCtx->streams[m_videoIdx]->codecpar, m_videoDecodeCtx);
		//	/*m_formatCtx->streams[m_videoIdx]->r_frame_rate.num = 30;
		//	m_formatCtx->streams[m_videoIdx]->r_frame_rate.den = 1;*/

		//	int ret = avformat_write_header(m_formatCtx, nullptr);
		//	if (ret < 0) 
		//		return -1;

		//	m_videoUpdateSps = false;
		//}
		//
		//AVRational tb;
		//tb.den = 90000;
		//tb.num = 1;
		//av_packet_rescale_ts(m_packet, tb, m_formatCtx->streams[m_videoIdx]->time_base);
		//m_packet->stream_index = m_videoIdx;
		//printf("=====================================>%d\n", m_packet->pts);

		//ret = av_write_frame(m_formatCtx, m_packet);
		//if (m_videoCurPacket) {
		//	free(m_videoCurPacket);
		//	m_videoCurPacketLen = 0;
		//	m_videoCurPacket = nullptr;
		//}
		//if (ret < 0)
		//	return -1;

		return 0;
	}

	int PullTransport::TryDecodeFrame() {
		int ret = avcodec_send_packet(m_videoDecodeCtx, m_packet);
		if (ret < 0 && ret != AVERROR(EAGAIN) && ret != AVERROR_EOF)
			return -1;
		AVFrame* frame = av_frame_alloc();
		ret = avcodec_receive_frame(m_videoDecodeCtx, frame);
		av_frame_free(&frame);
		if (ret >= 0)
			return 0;
		if (ret == AVERROR(EAGAIN) || ret == AVERROR_EOF)
			return -1;
	}
} // namespace RTC
