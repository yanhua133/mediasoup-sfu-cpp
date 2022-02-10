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
					connect();
				}
				catch (const MediaSoupError& error)
				{
					disconnect();
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

	void PushTransport::connect() {
		if (connected)
			return;

		avformat_alloc_output_context2(&m_context, NULL, m_formatName.c_str(), m_url.c_str());
		if (!m_context) {
			MS_THROW_ERROR("error allocating the avformat context");
		}


		AVStream* st = avformat_new_stream(m_context, NULL);

		st->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;

		AVCodec* codec = avcodec_find_encoder_by_name(RTMP_AUDIO_CODEC);
		if (!codec) {
			MS_THROW_TYPE_ERROR("missing audio encoder");
		}

		AVCodecContext* enc_ctx = avcodec_alloc_context3(codec);
		if (!enc_ctx) {
			MS_THROW_ERROR("error allocating the audio encoding context");
		}
		enc_ctx->codec_type = st->codecpar->codec_type;

		if (m_context->oformat->flags & AVFMT_GLOBALHEADER)
			enc_ctx->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;

		AVDictionary* enc_opt = NULL;

		// for official ffmpeg aac encoder
		if(RTMP_AUDIO_CODEC == "aac")
			av_dict_set(&enc_opt, "strict", "-2", 0);

		int ret = avio_open2(&m_context->pb, m_url.c_str(), AVIO_FLAG_WRITE, NULL, NULL);
		if (ret < 0) {
			MS_THROW_ERROR("error openning avio");
		}

		ret = avformat_write_header(m_context, &enc_opt);
		if (ret < 0) {
			MS_THROW_ERROR("error writting header");
		}
	}

	void PushTransport::disconnect() {
		connected = false;
	}

	void PushTransport::init_audio_filter() {
		m_audioGraph = avfilter_graph_alloc();
		if(!m_audioGraph)
			MS_THROW_ERROR("error allocating the audio avfilter graph");
		char val[1];
		val[0] = 0;
		av_opt_set(m_audioGraph, "aresample_swr_opts", val, 0);

		AVFilterInOut* inputs, * outputs;
		int ret = avfilter_graph_parse2(m_audioGraph, "anull", &inputs, &outputs);
		if (ret < 0)
			MS_THROW_ERROR("error parsing the audio avfilter graph");

		
		const AVFilter* abuffer_filt = avfilter_get_by_name("abuffer");
		AVBPrint args;
		av_bprint_init(&args, 0, AV_BPRINT_SIZE_AUTOMATIC);
		av_bprintf(&args, "time_base=%d/%d:sample_rate=%d:sample_fmt=%s",
			1, m_audioSampleRate, m_audioSampleRate,
			av_get_sample_fmt_name(RTMP_AUDIO_FRAME_FORMAT));
		av_bprintf(&args, ":channel_layout=0x%llx", RTMP_AUDIO_CHANNEL_LAYOUT);

		ret = avfilter_graph_create_filter(&m_audioFilterIn, abuffer_filt,
			"graph_0_in_0_1", args.str, NULL, m_audioGraph);
		if (ret < 0)
			MS_THROW_ERROR("error creating the audio input filter");

		ret = avfilter_link(m_audioFilterIn, 0, inputs->filter_ctx, inputs->pad_idx);
		if (ret < 0)
			MS_THROW_ERROR("error linking the audio input filter");

		avfilter_inout_free(&inputs);

		ret = avfilter_graph_create_filter(&m_audioFilterOut,
			avfilter_get_by_name("abuffersink"), "out_0_1", NULL, NULL, m_audioGraph);
		if (ret < 0)
			MS_THROW_ERROR("error creating the audio output filter");

		ret = av_opt_set_int(m_audioFilterOut, "all_channel_counts", 1, AV_OPT_SEARCH_CHILDREN);
		if (ret < 0)
			MS_THROW_ERROR("error setting the audio output filter option");

		av_bprint_init(&args, 0, AV_BPRINT_SIZE_UNLIMITED);
		av_bprintf(&args, "sample_fmts=%s:sample_rates=96000|88200|64000|48000|44100|32000|24000|22050|16000|12000|11025|8000|7350:", RTMP_AUDIO_FRAME_FORMAT_NAME);
		if (!av_bprint_is_complete(&args))
			MS_THROW_ERROR("error setting av_bprint");

		AVFilterContext* format;

		ret = avfilter_graph_create_filter(&format,
			avfilter_get_by_name("aformat"),
			"format_out_0_1", args.str, NULL, m_audioGraph);
		if (ret < 0)
			MS_THROW_ERROR("error creating output format filter");

		ret = avfilter_link(outputs->filter_ctx, outputs->pad_idx, format, 0);
		if (ret < 0)
			MS_THROW_ERROR("error linking the audio output format filter");

		ret = avfilter_link(format, 0, m_audioFilterOut, 0);
		if (ret < 0)
			MS_THROW_ERROR("error linking the audio output filter");

		avfilter_inout_free(&outputs);

		ret = avfilter_graph_config(m_audioGraph, NULL);
		if (ret < 0)
			MS_THROW_ERROR("error configing the avfilter graph");

		m_audioFormat = av_buffersink_get_format(m_audioFilterOut);
		m_audioChannelLayout = av_buffersink_get_channel_layout(m_audioFilterOut);

		av_buffersink_set_frame_size(m_audioFilterOut, 0);


		ret = av_buffersrc_add_frame_flags(m_audioFilterIn, frame, AV_BUFFERSRC_FLAG_PUSH);
		if (ret < 0) {
	}
} // namespace RTC
