#define MS_CLASS "RTC::Codecs::H265"
// #define MS_LOG_DEV_LEVEL 3

#include "RTC/Codecs/H265.hpp"
#include "Logger.hpp"
#include "Utils.hpp"

#define H265_TYPE(v) ((v >> 1) & 0x3f)
#define FU_START(v) (v & 0x80)
#define FU_END(v)    (v & 0x40)
#define FU_NAL(v)    (v & 0x3F)

//RFC7798 RTP Payload Format for High Efficiency Video Coding (HEVC)
/*
0               1
0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5
+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
|F|    Type   |  LayerId  | TID |
+-------------+-----------------+
Forbidden zero(F) : 1 bit
NAL unit type(Type) : 6 bits
NUH layer ID(LayerId) : 6 bits
NUH temporal ID plus 1 (TID) : 3 bits
*/

namespace RTC
{
    namespace Codecs
    {
        /* Class methods. */

        H265::PayloadDescriptor* H265::Parse(
          const uint8_t* data, size_t len, RTC::RtpPacket::FrameMarking* frameMarking, uint8_t frameMarkingLen)
        {
            MS_TRACE();

            if (len < 2)
                return nullptr;

            std::unique_ptr<PayloadDescriptor> payloadDescriptor(new PayloadDescriptor());

            // Use frame-marking.
            if (frameMarking)
            {
                // Read fields.
                payloadDescriptor->s   = frameMarking->start;
                payloadDescriptor->e   = frameMarking->end;
                payloadDescriptor->i   = frameMarking->independent;
                payloadDescriptor->d   = frameMarking->discardable;
                payloadDescriptor->b   = frameMarking->base;
                payloadDescriptor->tid = frameMarking->tid;

                payloadDescriptor->hasTid = true;

                if (frameMarkingLen >= 2)
                {
                    payloadDescriptor->hasLid = true;
                    payloadDescriptor->lid    = frameMarking->lid;
                }

                if (frameMarkingLen == 3)
                {
                    payloadDescriptor->hasTl0picidx = true;
                    payloadDescriptor->tl0picidx    = frameMarking->tl0picidx;
                }

                // Detect key frame.
                if (frameMarking->start && frameMarking->independent)
                    payloadDescriptor->isKeyFrame = true;
            }

            // NOTE: Unfortunately libwebrtc produces wrong Frame-Marking (without i=1 in
            // keyframes) when it uses H265 hardware encoder (at least in Mac):
            //   https://bugs.chromium.org/p/webrtc/issues/detail?id=10746
            //
            // As a temporal workaround, always do payload parsing to detect keyframes if
            // there is no frame-marking or if there is but keyframe was not detected above.
            if (!frameMarking || !payloadDescriptor->isKeyFrame)
            {
                //H265 nal should between [0 63], now lower than 50
                uint8_t nal = (*data & 0x7E) >> 1;
                
                switch (nal)
                {
                    // Single NAL unit packet.
                    // IDR (instantaneous decoding picture).
                    case 19://idrwradl
                    case 20://idrnlp
                    case 21://cra
                    {
                        //payloadDescriptor->isKeyFrame = true;
                        break;
                    }
                    // VPS
                    case 32:
                    {
                        // TODO: why janus think vps and sps as keyframe? https://github.com/meetecho/janus-gateway/blob/f988aa31880051aa72014affc613020b50512be8/utils.c    janus_h265_is_keyframe
                        payloadDescriptor->isKeyFrame = true;
                        break;
                    }
                    // SPS
                    case 33:
                    {
                        // TODO: why janus think vps and sps as keyframe? https://github.com/meetecho/janus-gateway/blob/f988aa31880051aa72014affc613020b50512be8/utils.c    janus_h265_is_keyframe
                        payloadDescriptor->isKeyFrame = true;
                        break;
                    }
                    // PPS
                    case 34:
                    {
                        // do nothing
                        break;
                    }
                    // SEI
                    case 39:
                    case 40:
                    {
                        // do nothing
                        break;
                    }
                    // aggregated packet (AP) - with two or more NAL units.
                    case 48:
                    {
                        size_t offset{ 1 };

                        len -= 1;

                        // Iterate NAL units.
                        while (len >= 3)
                        {
                            auto naluSize  = Utils::Byte::Get2Bytes(data, offset);
                            uint8_t subnal = (*(data + offset + sizeof(naluSize)) & 0x7E) >> 1;
                            bool subnal_isKeyFrame = (subnal == 32 || subnal == 33); //|| subnal == 32 || subnal == 31);
                            if (subnal_isKeyFrame)
                            {
                                payloadDescriptor->isKeyFrame = true;
                                break;
                            }

                            // Check if there is room for the indicated NAL unit size.
                            if (len < (naluSize + sizeof(naluSize)))
                                break;

                            offset += naluSize + sizeof(naluSize);
                            len -= naluSize + sizeof(naluSize);
                        }

                        break;
                    }

                    // fragmentation unit (FU).
                        // 4.4.3. Fragmentation Units (p29)
                        /*
                         0               1               2               3
                         0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                        |     PayloadHdr (Type=49)      |    FU header  |  DONL (cond)  |
                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-|
                        |  DONL (cond)  |                                               |
                        |-+-+-+-+-+-+-+-+                                               |
                        |                           FU payload                          |
                        |                                                               |
                        |                               +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                        |                               :    ...OPTIONAL RTP padding    |
                        +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
                        +---------------+
                        |0|1|2|3|4|5|6|7|
                        +-+-+-+-+-+-+-+-+
                        |S|E|   FuType  |
                        +---------------+
                        */
                    case 49:
                    {
                        uint8_t subnal   = *(data + 1) & 0x3F;
                        uint8_t startBit = *(data + 1) & 0x80;//S bit==1 means nal begin
                        uint8_t endBit = *(data + 1) & 0x40;
                        bool subnal_isKeyFrame = (subnal == 32 || subnal == 33); //|| subnal == 32 || subnal == 31);
                        // TODO: startBit == 128 suitabke for 265??
                        if (subnal_isKeyFrame && startBit == 128)
                            payloadDescriptor->isKeyFrame = true;
                        break;
                    }
                }
            }

            return payloadDescriptor.release();
        }

        void H265::ProcessRtpPacket(RTC::RtpPacket* packet)
        {
            MS_TRACE();

            auto* data = packet->GetPayload();
            auto len   = packet->GetPayloadLength();
            RtpPacket::FrameMarking* frameMarking{ nullptr };
            uint8_t frameMarkingLen{ 0 };

            // Read frame-marking.
            packet->ReadFrameMarking(&frameMarking, frameMarkingLen);

            PayloadDescriptor* payloadDescriptor = H265::Parse(data, len, frameMarking, frameMarkingLen);

            if (!payloadDescriptor)
                return;

            auto* payloadDescriptorHandler = new PayloadDescriptorHandler(payloadDescriptor);

            packet->SetPayloadDescriptorHandler(payloadDescriptorHandler);
        }

        /* Instance methods. */

        void H265::PayloadDescriptor::Dump() const
        {
            MS_TRACE();

            MS_DUMP("<PayloadDescriptor>");
            MS_DUMP(
              "  s:%" PRIu8 "|e:%" PRIu8 "|i:%" PRIu8 "|d:%" PRIu8 "|b:%" PRIu8,
              this->s,
              this->e,
              this->i,
              this->d,
              this->b);
            if (this->hasTid)
                MS_DUMP("  tid        : %" PRIu8, this->tid);
            if (this->hasLid)
                MS_DUMP("  lid        : %" PRIu8, this->lid);
            if (this->hasTl0picidx)
                MS_DUMP("  tl0picidx  : %" PRIu8, this->tl0picidx);
            MS_DUMP("  isKeyFrame : %s", this->isKeyFrame ? "true" : "false");
            MS_DUMP("</PayloadDescriptor>");
        }

        H265::PayloadDescriptorHandler::PayloadDescriptorHandler(H265::PayloadDescriptor* payloadDescriptor)
        {
            MS_TRACE();

            this->payloadDescriptor.reset(payloadDescriptor);
        }

        bool H265::PayloadDescriptorHandler::Process(
          RTC::Codecs::EncodingContext* encodingContext, uint8_t* /*data*/, bool& /*marker*/)
        {
            MS_TRACE();

            auto* context = static_cast<RTC::Codecs::H265::EncodingContext*>(encodingContext);

            MS_ASSERT(context->GetTargetTemporalLayer() >= 0, "target temporal layer cannot be -1");

            // Check if the payload should contain temporal layer info.
            if (context->GetTemporalLayers() > 1 && !this->payloadDescriptor->hasTid)
            {
                MS_WARN_DEV("stream is supposed to have >1 temporal layers but does not have tid field");
            }

            // clang-format off
            if (
                this->payloadDescriptor->hasTid &&
                this->payloadDescriptor->tid > context->GetTargetTemporalLayer()
            )
            // clang-format on
            {
                return false;
            }
            // Upgrade required. Drop current packet if base flag is not set.
            // NOTE: This is possible once this bug in libwebrtc has been fixed:
            //   https://github.com/versatica/mediasoup/issues/306
            //
            // clang-format off
            else if (
                this->payloadDescriptor->hasTid &&
                this->payloadDescriptor->tid > context->GetCurrentTemporalLayer() &&
                !this->payloadDescriptor->b
            )
            // clang-format on
            {
                return false;
            }

            // Update/fix current temporal layer.
            // clang-format off
            if (
                this->payloadDescriptor->hasTid &&
                this->payloadDescriptor->tid > context->GetCurrentTemporalLayer()
            )
            // clang-format on
            {
                context->SetCurrentTemporalLayer(this->payloadDescriptor->tid);
            }
            else if (!this->payloadDescriptor->hasTid)
            {
                context->SetCurrentTemporalLayer(0);
            }

            if (context->GetCurrentTemporalLayer() > context->GetTargetTemporalLayer())
                context->SetCurrentTemporalLayer(context->GetTargetTemporalLayer());

            return true;
        }

        void H265::PayloadDescriptorHandler::Restore(uint8_t* /*data*/)
        {
            MS_TRACE();
        }
    } // namespace Codecs
} // namespace RTC
