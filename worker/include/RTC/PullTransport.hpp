#ifndef MS_RTC_PULL_TRANSPORT_HPP
#define MS_RTC_PULL_TRANSPORT_HPP

#include "RTC/Transport.hpp"
#include "RTC/TransportTuple.hpp"
#include <map>
#include <thread>

extern "C" {
#include "libavformat/avformat.h"
#include "libavutil/audio_fifo.h"
}

#define RTMP_PROTO_NAME "rtmp"
#define RTMP_PROTO_FORMAT_NAME "flv"
#define RTMP_DEFAULT_PORT 1935
#define RTMP_DEFAULT_SUFFIX "/live"
#define RTMP_GENERATE_URL \ 
	m_url = RTMP_PROTO_NAME; \
	m_url += "://" + ip + suffix + "/" + stream;
	//m_url += "://" + ip + ":" + std::to_string(port) + suffix + "/" + id;
#define RTMP_AUDIO_CODEC "aac"
#define RTMP_AUDIO_FRAME_FORMAT AV_SAMPLE_FMT_FLTP
#define RTMP_AUDIO_FRAME_FORMAT_NAME "fltp"
#define RTMP_AUDIO_CHANNEL_LAYOUT 4

namespace RTC
{
	class PullTransport : public RTC::Transport, public RTC::UdpSocket::Listener
	{
	public:
		PullTransport(const std::string& id, RTC::Transport::Listener* listener, json& data);
		~PullTransport() override;

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

		void Connect();
		void Disconnect();

		void ThreadError(int id);

		void InitIncomingParameters();
		void InitVideoStream();
		void InitAudioStream();
		void InitOutgoingParameters();
		AVCodecID ChooseVideoCodecId(std::string name);
		AVCodecID ChooseAudioCodecId(std::string name);
		void AudioProcessPacket(RTC::RtpPacket* packet);
		void VideoProcessPacket(RTC::RtpPacket* packet);
		int AudioDecodeAndFifo(RTC::RtpPacket* packet);
		int AudioEncodeAndSend();
		void PacketFree();
		void ProbePayload(RTC::RtpPacket* packet);
		void ProcessPayloadStapA(RTC::RtpPacket* packet);
		void ProcessPayloadFuA(RTC::RtpPacket* packet);
		int VideoSend();
		int TryDecodeFrame();

		/* Pure virtual methods inherited from RTC::UdpSocket::Listener. */
	public:
		void OnUdpSocketPacketReceived(
			RTC::UdpSocket* socket, const uint8_t* data, size_t len, const struct sockaddr* remoteAddr) override;

	private:
		// Allocated by this
		RTC::UdpSocket* udpSocket{ nullptr };
		RTC::UdpSocket* rtcpUdpSocket{ nullptr };

		RTC::TransportTuple* tuple{ nullptr };
		RTC::TransportTuple* rtcpTuple{ nullptr };

		bool rtcpMux{ true };
		bool comedia{ false };

		std::string m_url, m_formatName;
		std::thread* m_pThread{ nullptr };
		// Others.
		struct sockaddr_storage remoteAddrStorage;
		bool connectCalled{ false }; // Whether connect() was succesfully called.
		bool connected{ false };
		//AVFilterGraph * m_audioGraph{nullptr};
		//AVFilterContext* m_audioFilterIn{ nullptr }, * m_audioFilterOut{ nullptr };
		int m_audioSampleRate{ 48000 }, m_audioFormat{ 0 };
		uint64_t m_audioChannelLayout{ 4 };
		int m_videoRateClock{ 90000 };

		AVFormatContext *m_formatCtx{ nullptr };
		std::string m_videoDecoderName{ "h264" }, m_audioDecoderName{ "opus" };
		AVCodecContext *m_audioDecodeCtx{ nullptr }, *m_audioEncodeCtx{ nullptr };
		AVCodecContext* m_videoDecodeCtx{ nullptr }, * m_videoEncodeCtx{ nullptr };
		AVAudioFifo *m_audioFifo{ nullptr };
		bool m_audioProcessPacket{ false };
		unsigned int m_videoIdx{ 0 }, m_audioIdx{ 0 };
		uint32_t m_audioRefTimestamp{ 0 }, m_audioCurTimestamp{ 0 }, m_audioNextTimestamp{ 0 }, m_audioPtsTimestamp{ 0 };
		AVPacket *m_packet{ nullptr };
		AVFrame *m_frame{ nullptr }, *m_audioMuteFrame{ nullptr };
		uint8_t *m_videoCurPacket{ nullptr }, * m_videoSpsPacket{ nullptr };
		size_t m_videoCurPacketLen{ 0 }, m_videoSpsPacketLen{ 0 };
		uint32_t m_videoRefTimestamp{ 0 }, m_videoCurPacketTs{ 0 };
		uint16_t m_videoCurSeqNumber{ 0 };
		bool m_videoUpdateSps{ false }, m_videoSendData{ false };
	};
} // namespace RTC

#endif
