#ifndef MSC_ORTC_HPP
#define MSC_ORTC_HPP

#include <json.hpp>
#include <string>
#include "RtpParameters.hpp"
using namespace mediasoup;

namespace mediasoupclient
{
    struct RtpMappingCodec
    {
        int payloadType;
        int mappedPayloadType;
    };
    void from_json(const json& j, RtpMappingCodec& st);
    void to_json(json& j, RtpMappingCodec& st) ;

    struct RtpMappingEncoding
    {
        uint32_t ssrc;
        std::string rid;
        std::string scalabilityMode;
        uint32_t mappedSsrc;
    };
    void from_json(const json& j, RtpMappingEncoding& st) ;
    void to_json(json& j, RtpMappingEncoding& st) ;

    struct RtpMapping
    {
      // codecs:
      // {
      //   payloadType: number;
      //   mappedPayloadType: number;
      // }[];
      json codecs;//std::vector<RtpMappingCodec>
      // encodings:
      // {
      //   ssrc?: number;
      //   rid?: string;
      //   scalabilityMode?: string;
      //   mappedSsrc: number;
      // }[];
      json encodings;//std::vector<RtpMappingEncoding>
    };
    void from_json(const json& j, RtpMapping& st) ;
    void to_json(json& j, RtpMapping& st) ;
	namespace ortc
	{
		void validateRtpCapabilities(nlohmann::json& caps);
		void validateRtpCodecCapability(nlohmann::json& codec);
		void validateRtcpFeedback(nlohmann::json& fb);
		void validateRtpHeaderExtension(nlohmann::json& ext);
		void validateRtpParameters(nlohmann::json& params);
		void validateRtpCodecParameters(nlohmann::json& codec);
		void validateRtpHeaderExtensionParameters(nlohmann::json& ext);
		void validateRtpEncodingParameters(nlohmann::json& encoding);
		void validateRtcpParameters(nlohmann::json& rtcp);
		void validateSctpCapabilities(nlohmann::json& caps);
		void validateNumSctpStreams(nlohmann::json& numStreams);
		void validateSctpParameters(nlohmann::json& params);
		void validateSctpStreamParameters(nlohmann::json& params);
		void validateIceParameters(nlohmann::json& params);
		void validateIceCandidates(nlohmann::json& params);
		void validateDtlsParameters(nlohmann::json& params);
		void validateProducerCodecOptions(nlohmann::json& params);
		nlohmann::json getExtendedRtpCapabilities(nlohmann::json& localCaps, nlohmann::json& remoteCaps);
		nlohmann::json getRecvRtpCapabilities(const nlohmann::json& extendedRtpCapabilities);
		nlohmann::json getSendingRtpParameters(
		  const std::string& kind, const nlohmann::json& extendedRtpCapabilities);
		nlohmann::json getSendingRemoteRtpParameters(
		  const std::string& kind, const nlohmann::json& extendedRtpCapabilities);
		const nlohmann::json generateProbatorRtpParameters(const nlohmann::json& videoRtpParameters);
		bool canSend(const std::string& kind, const nlohmann::json& extendedRtpCapabilities);
		bool canReceive(nlohmann::json& rtpParameters, const nlohmann::json& extendedRtpCapabilities);
    
    RtpCapabilities generateRouterRtpCapabilities(
      std::vector<RtpCodecCapability> &mediaCodecs
    );
  
    bool canConsume(
      RtpParameters consumableParams,
      RtpCapabilities caps
    );
   
    
    json getProducerRtpParametersMapping(
      RtpParameters &params,//: RtpParameters,
      RtpCapabilities &caps//: RtpCapabilities
    );
    json getConsumableRtpParameters(
      std::string &kind,
      RtpParameters &params,//: RtpParameters,
      RtpCapabilities &caps,//: RtpCapabilities,
      json &jrtpMapping//: RtpMapping
    );
    RtpParameters getConsumerRtpParameters(
      RtpParameters consumableParams,//: RtpParameters,
      RtpCapabilities &caps,//: RtpCapabilities,
      bool pipe//: boolean
    );
 
	} // namespace ortc

} // namespace mediasoupclient

#endif
