#define MSC_CLASS "SdpOffer"

#include "SdpUtils.hpp"
#include "../LoggerC.hpp"
#include "../MediaSoupClientErrors.hpp"
#include <algorithm> // ::transform
#include <cctype>    // ::tolower
#include <list>
#include <map>
#include <set>
#include <vector>
#include "../UtilsC.hpp"
#include "../RtpParameters.hpp"
#include "../SctpParameters.hpp"
#include "../SrtpParameters.hpp"
#include "../scalabilityModes.hpp"
#include "../WebRtcTransport.hpp"
#include "../Consumer.hpp"
#include "../Producer.hpp"
#include "RemoteSdp.hpp"
#include "sdptransform/sdptransform.hpp"
using json = nlohmann::json;
using namespace mediasoupclient;
using namespace mediasoup;
namespace mediasoupclient
{
	namespace Sdp
	{
    ////for raw offer and answer data convert to mediasoup format
		namespace Offer
		{
		   RtpCapabilities sdpToRecvRtpCapabilities(
          json &sdpObject,
          json &localCaps
        ) 
        {
          auto caps = Utils::extractRtpCapabilities(
            sdpObject
          );

          try {
            ortc::validateRtpCapabilities(caps);
          } catch (...) {
            MSC_THROW_TYPE_ERROR("FIXME BUG:");
            //process.exit(1);
          }

          auto extendedCaps = ortc::getExtendedRtpCapabilities(caps, localCaps);

          auto recvCaps = ortc::getRecvRtpCapabilities(extendedCaps);

          // DEBUG: Uncomment for details.
          // prettier-ignore
          {
            // MSC_DEBUG("[sdpToRecvRtpCapabilities] RtpCapabilities:\n%O", caps);
            // MSC_DEBUG("[sdpToRecvRtpCapabilities] Extended RtpCapabilities:\n%O", extendedCaps);
            // MSC_DEBUG("[sdpToRecvRtpCapabilities] RECV RtpCapabilities:\n%O", recvCaps);
          }

          return recvCaps;
        }



        RtpParameters sdpToSendRtpParameters(
          json &sdpObject,
          json &localCaps,
          std::string &kind
        ) {
          auto caps = Utils::extractRtpCapabilities(
            sdpObject
          );

          try {
            ortc::validateRtpCapabilities(caps);
          } catch (...) {
            MSC_THROW_TYPE_ERROR("FIXME BUG:");
            //process.exit(1);
          }

          auto extendedCaps = ortc::getExtendedRtpCapabilities(caps, localCaps);
          auto sendParams = ortc::getSendingRemoteRtpParameters(kind, extendedCaps);

          for (auto &codec : sendParams["codecs"]) {
            // FIXME: re codec payloadType
            // to include doing this, so we don"t need to branch an if() here.
            if(codec["mimeType"] == "video/H264" && sendParams["codecs"].size() == 2) {
              //sendParams.codecs.pop();
              sendParams["codecs"].erase(sendParams["codecs"].end());
            }

            break;
          }
          // Now we have to fill "mid", "encodings", and "rtcp" fields.

          //auto sdpMediaObj = (sdpObject.media || []).find((m) => m.type == kind) || {};
          auto sdpMediaObj = json::object();
          if (sdpObject.find("media") != sdpObject.end()) {
            // there is an entry with key "foo"
            for(auto &m : sdpObject["media"]) {
              if ( m["type"] == kind ) {
                 sdpMediaObj = m ;
              }
            }
          }

          if (sdpMediaObj.contains("mid" )) {
            sendParams["mid"] = sdpMediaObj["mid"];
          } else {
            sendParams["mid"] = (kind == "audio" ? "0" : "1");
          }

          if (sdpMediaObj.contains("rids" )) {
            for (auto &mediaRid : sdpMediaObj["rids"]) {
              // FIXME: Maybe MsRtpUtils.getRtpEncodings() should just be improved
              // to include doing this, so we don"t need to branch an if() here.

              // Push an RTCRtpEncodingParameters.
              sendParams["encodings"].push_back({
                { "rid", mediaRid["id"] }
              });
            }
          } else {
              sendParams["encodings"] = Utils::getRtpEncodings({
                  {"sdpObject",sdpObject},
                  {"kind",kind}
              });
          }

          sendParams["rtcp"] = {
            { "cname", Utils::getCname(sdpMediaObj)},

            // These are boolean fields.
            {"reducedSize", sdpMediaObj.contains("rtcpRsize") && sdpMediaObj["rtcpRsize"]},
            {"mux", sdpMediaObj.contains("rtcpMux") && sdpMediaObj["rtcpMux"]},
          };

          // DEBUG: Uncomment for details.
          // prettier-ignore
          {
            // MSC_DEBUG("[sdpToSendRtpParameters] ${kind} RtpCapabilities:\n%O", caps);
            // MSC_DEBUG("[sdpToSendRtpParameters] ${kind} Extended RtpCapabilities:\n%O", extendedCaps);
            // MSC_DEBUG("[sdpToSendRtpParameters] ${kind} SEND RtpParameters:\n%O", sendParams);
          }

          return sendParams;
        }

              
        DtlsParameters getMediasoupDtlsParameters(json &sdpOffer) 
        {
          auto remoteSdpObj = sdptransform::parse(sdpOffer);

          // DEBUG: Uncomment for details.
          // prettier-ignore
          // MSC_DEBUG("[getMediasouopDtlsParameters] Remote SDP object:\n%O", remoteSdpObj);

          auto dtlsParameters =  Utils::extractDtlsParameters({
            {"sdpObject", remoteSdpObj }
          });
          return dtlsParameters;
        }



        std::vector<json> getMediasoupProducerMedias(json &sdpOffer) 
        {
          auto remoteSdpObj = sdptransform::parse(sdpOffer);
          std::vector<json> producerMedias;
          // Get a list : media and make Producers for all : them.
          for (auto &media : remoteSdpObj["media"]) {
            if (!(media.contains("rtp"))) {
              // Skip media that is not RTP.
              continue;
            }
            if (!(media.contains("direction"))) {
              // Skip media for which the direction is unknown.
              continue;
            }
            if (media["direction"] != "sendonly") {
              // Skip media which is not "sendonly", because this is a receive-only
              // SDP endpoint for remote SDP Offers.
              // FIXME: A proper SDP endpoint should be able to handle all directions.
              continue;
            }
            producerMedias.push_back(media);
          }
          return producerMedias;
        }


        RtpParameters getMediasoupRtpParameters(json &sdpOffer, std::string &kind, json localCaps) 
        {

          auto remoteSdpObj = sdptransform::parse(sdpOffer);

          // Add a new Producer for the given media.
          auto rtpParameters = sdpToSendRtpParameters(
            remoteSdpObj,
            localCaps,//RtpCapabilities
            kind  //MediaKind audio|video
          );
          return rtpParameters;
        }


        RtpCapabilities getMediasoupRtpCapabilities(json &sdpOffer, json localCaps) 
        {

          auto remoteSdpObj = sdptransform::parse(sdpOffer);
          auto remoteCaps = sdpToRecvRtpCapabilities(remoteSdpObj,localCaps);
          return remoteCaps;

        }



        json createWebrtcSdpAnswer(std::shared_ptr<Transport> webRtcTransport,std::vector<std::shared_ptr<Producer>> &producers,std::vector<json> producerMedias) 
        {

              auto sdpBuilder = new RemoteSdp(
                webRtcTransport->iceParameters(),
                webRtcTransport->iceCandidates(),// as ClientIceCandidate[],
                webRtcTransport->dtlsParameters(),
                webRtcTransport->sctpParameters()
              );

              MSC_DEBUG("[createSdpAnswer] Make recvonly SDP Answer");

              for (int i = 0; i < producers.size(); i++) {
                auto sdpMediaObj = producerMedias[i];
                auto const recvParams = producers[i]->rtpParameters();
                json jrecvParams = recvParams;

                // Each call to RemoteSdp.send() creates a new AnswerMediaSection,
                // which always assumes an "a=recvonly" direction.
                const std::string reuseMid="";
                sdpBuilder->Send(
                  /*offerMediaObject:*/ sdpMediaObj,
                  /*reuseMid:*/ reuseMid,
                  /*offerRtpParameters:*/ jrecvParams,
                  /*answerRtpParameters:*/ jrecvParams,
                  /*codecOptions:*/ nullptr
                  /*extmapAllowMixed:false*/
                );
              }

              auto localSdp = sdpBuilder->GetSdp();
              delete sdpBuilder;
              sdpBuilder = nullptr;
              return localSdp;
        }



        json createWebrtcSdpOffer(std::shared_ptr<Transport> webRtcTransport,std::vector<std::shared_ptr<Consumer>> &consumers) 
        {

          auto sdpBuilder = new RemoteSdp({
            webRtcTransport->iceParameters(),
            webRtcTransport->iceCandidates(),// as ClientIceCandidate[],
            webRtcTransport->dtlsParameters(),
            webRtcTransport->sctpParameters()
          });

          // Make an MSID to be used for both "audio" and "video" kinds.
          auto sendMsid = uuidv4_prefix8();//  v4().substr(0, 8);

          MSC_DEBUG("[createSdpOffer] Make sendonly SDP Offer");

          for (int i = 0; i < consumers.size(); i++) {
            auto mid = consumers[i]->rtpParameters().mid.empty() ? "nomid" : consumers[i]->rtpParameters().mid;
            auto kind = consumers[i]->kind();
            auto sendParams = consumers[i]->rtpParameters();

            // Each call to RemoteSdp.receive() creates a new OfferMediaSection,
            // which always assumes an "a=sendonly" direction.
            sdpBuilder->Receive(
              mid,
              kind,
              /*offerRtpParameters:*/ sendParams,

              // Parameters used to build the "msid" attribute:
              // a=msid:<streamId> <trackId>
              /*streamId:*/ sendMsid,
              /*trackId:*/ sendMsid+"-"+kind //"${sendMsid}-${kind}",
            );
          }

          auto localSdp = sdpBuilder->GetSdp();
          delete sdpBuilder;
          sdpBuilder = nullptr;
          return localSdp;

        }


		} // namespace Offer
	}   // namespace Sdp
} // namespace mediasoupclient
