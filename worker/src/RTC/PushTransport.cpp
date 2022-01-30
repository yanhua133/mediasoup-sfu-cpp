#define MS_CLASS "RTC::PushTransport"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/PushTransport.hpp"
#include "Logger.hpp"
#include "MediaSoupErrors.hpp"
#include "Utils.hpp"
#include "Channel/Notifier.hpp"

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

				try
				{
					if (!this->comedia)
					{
						auto jsonIpIt = request->data.find("ip");

						if (jsonIpIt == request->data.end() || !jsonIpIt->is_string())
							MS_THROW_TYPE_ERROR("missing ip");

						ip = jsonIpIt->get<std::string>();

						// This may throw.
						Utils::IP::NormalizeIp(ip);

						auto jsonPortIt = request->data.find("port");

						if (
							jsonPortIt == request->data.end() ||
							!Utils::Json::IsPositiveInteger(*jsonPortIt)
						)
						{
							MS_THROW_TYPE_ERROR("missing port");
						}

						port = jsonPortIt->get<uint16_t>();

						auto jsonProtoTypeIt = request->data.find("protoType");

						if (jsonProtoTypeIt == request->data.end() || !jsonProtoTypeIt->is_string())
						{
							MS_THROW_TYPE_ERROR("missing protocol type");
						}

						protoType = jsonProtoTypeIt->get<std::string>();
						if (protoType != "rtmp") {
							MS_THROW_TYPE_ERROR("unknown protocol");
						}

						auto jsonSuffixIt = request->data.find("suffix");

						if (jsonSuffixIt != request->data.end() && jsonSuffixIt->is_string())
						{
							suffix = jsonSuffixIt->get<std::string>();
						}

						std::string url = "rtmp://" + ip + ":" + std::to_string(port) + suffix + "/1";// +id;

						m_context = avformat_alloc_context();
						avformat_alloc_output_context2(&m_context, NULL, "flv", url.c_str());
						if (!m_context) {
							MS_THROW_TYPE_ERROR("allocate output context failed");
						}

						if (!(m_context->oformat->flags & AVFMT_NOFILE)) {
							if (avio_open(&m_context->pb, m_context->url, AVIO_FLAG_WRITE) < 0) {
								avformat_free_context(m_context);
								m_context = NULL;
								MS_THROW_TYPE_ERROR("open avio failed");
							}
						}
					}
				}
				catch (const MediaSoupError& error)
				{
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
		if (m_videoStream == NULL) {
			addVideoStream(AV_CODEC_ID_H264, 1920, 1080);

			if (avformat_write_header(m_context, NULL) < 0) {
				if (cb)
				{
					(*cb)(false);

					delete cb;
				}

				return;
			}
		}
		int64_t ts = packet->GetTimestamp();
		if (m_videoStartTs == 0) {
			m_videoStartTs = ts;
			m_videoPrevTs = m_videoStartTs;
			m_videoOffsetTs = 0;
		}

		if (m_videoPrevTs < ts) {
			m_videoOffsetTs = ts - m_videoStartTs;
			m_videoPrevTs = ts;
		}
		
		AVPacket pkt;
		av_init_packet(&pkt);
		pkt.data = packet->GetPayload();
		pkt.size = packet->GetPayloadLength();
		pkt.dts = m_videoOffsetTs;
		pkt.pts = pkt.dts;
		//pkt.duration = 3600;
		pkt.stream_index = m_videoStream->index;

		int ret = av_interleaved_write_frame(m_context, &pkt);
		if (ret < 0)
			return;

		return ;

		//const uint8_t* data = packet->GetData();
		//size_t len          = packet->GetSize();
		//
		//// Increase send transmission.
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

	bool PushTransport::addVideoStream(AVCodecID codec_id, uint32_t width, uint32_t height)
	{
		m_videoStream = avformat_new_stream(m_context, NULL);
		if (!m_videoStream) {
			return false;
		}

		m_videoStream->time_base.num = 1;
		m_videoStream->time_base.den = 90000;

		AVCodecParameters* par = m_videoStream->codecpar;
		par->codec_type = AVMEDIA_TYPE_VIDEO;
		par->codec_id = codec_id;
		par->width = width;
		par->height = height;
		//if (codec_id == AV_CODEC_ID_H264 || codec_id == AV_CODEC_ID_H265) { //extradata
		//	AVCodecParserContext* parser = av_parser_init(codec_id);
		//	if (!parser) {
		//		return false;
		//	}

		//	int size = parser->parser->split(NULL, m_videoKeyFrame->m_frame.payload, m_videoKeyFrame->m_frame.length);
		//	if (size > 0) {
		//		par->extradata_size = size;
		//		par->extradata = (uint8_t*)av_malloc(par->extradata_size + AV_INPUT_BUFFER_PADDING_SIZE);
		//		memcpy(par->extradata, m_videoKeyFrame->m_frame.payload, par->extradata_size);
		//	}
		//	else {
		//	}

		//	av_parser_close(parser);
		//}

		//if (codec_id == AV_CODEC_ID_H265) {
		//	par->codec_tag = 0x31637668; //hvc1
		//}

		return true;
	}
} // namespace RTC
