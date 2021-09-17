#ifndef MSC_SDP_OFFER_HPP
#define MSC_SDP_OFFER_HPP

#include <json.hpp>
#include "sdptransform/sdptransform.hpp"
//#include <api/media_stream_interface.h>
#include <string>
#include "../RtpParameters.hpp"
#include "../SctpParameters.hpp"
#include "../SrtpParameters.hpp"
#include "../scalabilityModes.hpp"
#include "../WebRtcTransport.hpp"
#include "../Consumer.hpp"
#include "../Producer.hpp"
using json = nlohmann::json;
using namespace mediasoupclient;
using namespace mediasoup;
namespace mediasoupclient
{
	namespace Sdp
	{
    //for raw offer and answer data convert to mediasoup format
		namespace Offer
		{
		  RtpCapabilities sdpToRecvRtpCapabilities(  json &sdpObject,json &localCaps);
      RtpParameters sdpToSendRtpParameters(json &sdpObject,json &localCaps,std::string &kind);
      DtlsParameters getMediasoupDtlsParameters(json &sdpOffer);
      std::vector<json> getMediasoupProducerMedias(json &sdpOffer);
      RtpParameters getMediasoupRtpParameters(json &sdpOffer, std::string &kind, json localCaps);
      RtpCapabilities getMediasoupRtpCapabilities(json &sdpOffer, json localCaps);
      json createWebrtcSdpOffer(std::shared_ptr<Transport> webRtcTransport,std::vector<std::shared_ptr<Consumer>> &consumers);
      json createWebrtcSdpAnswer( std::shared_ptr<Transport> webRtcTransport,std::vector<std::shared_ptr<Producer>> &producers,std::vector<json> producerMedias);
		} // namespace Offer
	}   // namespace Sdp
} // namespace mediasoupclient

#endif
