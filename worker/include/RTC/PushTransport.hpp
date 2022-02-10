#ifndef MS_RTC_PUSH_TRANSPORT_HPP
#define MS_RTC_PUSH_TRANSPORT_HPP

#include "RTC/Transport.hpp"
#include "RTC/TransportTuple.hpp"
#include <map>
extern "C" {
#include <libavformat/avformat.h>
}

extern "C" {
#include "libavformat/avformat.h"
#include "libavfilter/avfilter.h"
}

#define RTMP_PROTO_NAME "rtmp"
#define RTMP_PROTO_FORMAT_NAME "flv"
#define RTMP_DEFAULT_PORT 1935
#define RTMP_DEFAULT_SUFFIX "/live"
#define RTMP_GENERATE_URL \ 
	m_url = RTMP_PROTO_NAME; \
	m_url += "://" + ip + ":" + std::to_string(port) + suffix + "/" + id;
#define RTMP_AUDIO_CODEC "aac"
#define RTMP_AUDIO_FRAME_FORMAT AV_SAMPLE_FMT_FLTP
#define RTMP_AUDIO_FRAME_FORMAT_NAME "fltp"
#define RTMP_AUDIO_CHANNEL_LAYOUT 4

namespace RTC
{
	class PushTransport : public RTC::Transport
	{
	public:
		PushTransport(const std::string& id, RTC::Transport::Listener* listener, json& data);
		~PushTransport() override;

	public:
		void FillJson(json& jsonObject) const override;
		void FillJsonStats(json& jsonArray) override;
		void HandleRequest(Channel::Request* request) override;
		void HandleNotification(PayloadChannel::Notification* notification) override;

	private:
		bool IsConnected() const override;
		void SendRtpPacket(
		  RTC::Consumer* consumer,
		  RTC::RtpPacket* packet,
		  RTC::Transport::onSendCallback* cb = nullptr) override;
		void SendRtcpPacket(RTC::RTCP::Packet* packet) override;
		void SendRtcpCompoundPacket(RTC::RTCP::CompoundPacket* packet) override;
		void SendMessage(
		  RTC::DataConsumer* dataConsumer,
		  uint32_t ppid,
		  const uint8_t* msg,
		  size_t len,
		  onQueuedCallback* cb = nullptr) override;
		void SendSctpData(const uint8_t* data, size_t len) override;
		void RecvStreamClosed(uint32_t ssrc) override;
		void SendStreamClosed(uint32_t ssrc) override;
		void OnPacketReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnRtpDataReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnRtcpDataReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);
		void OnSctpDataReceived(RTC::TransportTuple* tuple, const uint8_t* data, size_t len);

		void connect();
		void disconnect();
		void init_audio_filter();

	private:
		// Allocated by this.
		std::string m_url, m_formatName;
		// Others.
		bool comedia{ false };
		struct sockaddr_storage remoteAddrStorage;
		bool connectCalled{ false }; // Whether connect() was succesfully called.
		bool connected{ false };
		AVFormatContext* m_context{ nullptr };
		AVFilterGraph* m_audioGraph{ nullptr };
		AVFilterContext* m_audioFilterIn{ nullptr }, * m_audioFilterOut{ nullptr };
		int m_audioSampleRate{ 48000 }, m_audioFormat{ 0 };
		uint64_t m_audioChannelLayout{ 4 };
	};
} // namespace RTC

#endif
