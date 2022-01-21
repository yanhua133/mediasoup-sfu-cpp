#include <string>
#include <vector>
#include <json.hpp>
#include "RtpParameters.hpp"
#include "SctpParameters.hpp"
#include "SrtpParameters.hpp"
#include "Consumer.hpp"
#include "Producer.hpp"
#include "WebRtcTransport.hpp"
#include "PlainTransport.hpp"
#include "PipeTransport.hpp"
#include "PushTransport.hpp"

using json = nlohmann::json;
namespace mediasoup
{

void to_json(json& j, const RtcpFeedback& st) {
       j = json{
           {"type", st.type},
           {"parameter", st.parameter}
       };
}

void from_json(const json& j, RtcpFeedback& st) {
       if(j.contains("type"))
           j.at("type").get_to(st.type);
        if(j.contains("parameter"))
            j.at("parameter").get_to(st.parameter);
      
}

void to_json(json& j, const _rtx& st) {
       j = json{
           {"ssrc", st.ssrc}
       };
}

void from_json(const json& j, _rtx& st) {
    if(j.contains("ssrc"))
       j.at("ssrc").get_to(st.ssrc);
      
}

void to_json(json& j, const RtpEncodingParameters& st) {
       j = json{
           {"ssrc", st.ssrc},
           {"rid", st.rid},
           {"codecPayloadType", st.codecPayloadType},
           {"rtx", st.rtx},
           {"dtx", st.dtx},
           {"scalabilityMode", st.scalabilityMode},
           {"scaleResolutionDownBy", st.scaleResolutionDownBy},
           {"maxBitrate", st.maxBitrate},
           {"active", st.active}
       };
        if(st.ssrc == 0)
        {
            j.erase("ssrc");
        }
    if(st.rid.empty())
    {
        j.erase("rid");
    }
    if(st.codecPayloadType == 0)
    {
        j.erase("codecPayloadType");
    }
    if(st.rtx.ssrc == 0)
    {
        j.erase("rtx");
    }

    if(st.scalabilityMode.empty())
    {
        j.erase("scalabilityMode");
    }
    if(st.scaleResolutionDownBy == 0)
    {
        j.erase("scaleResolutionDownBy");
    }
    if(st.maxBitrate == 0 )
    {
        j.erase("maxBitrate");
    }

}

void from_json(const json& j, RtpEncodingParameters& st) {
    if(j.contains("ssrc"))
       j.at("ssrc").get_to(st.ssrc);
    if(j.contains("rid"))
       j.at("rid").get_to(st.rid);
    if(j.contains("codecPayloadType"))
       j.at("codecPayloadType").get_to(st.codecPayloadType);
    if(j.contains("rtx"))
       j.at("rtx").get_to(st.rtx);
    if(j.contains("dtx"))
       j.at("dtx").get_to(st.dtx);
    if(j.contains("scalabilityMode"))
       j.at("scalabilityMode").get_to(st.scalabilityMode);
    if(j.contains("scaleResolutionDownBy"))
       j.at("scaleResolutionDownBy").get_to(st.scaleResolutionDownBy);
    if(j.contains("maxBitrate"))
       j.at("maxBitrate").get_to(st.maxBitrate);
    if(j.contains("active"))
        j.at("active").get_to(st.active);
      
}
void to_json(json& j, const RtpHeaderExtensionParameters& st) {
       j = json{
           {"uri", st.uri},
           {"id", st.id},
           {"encrypt", st.encrypt},
           {"parameters", st.parameters},
       };
}

void from_json(const json& j, RtpHeaderExtensionParameters& st) {
    if(j.contains("uri"))
       j.at("uri").get_to(st.uri);
    if(j.contains("id"))
       j.at("id").get_to(st.id);
    if(j.contains("encrypt"))
       j.at("encrypt").get_to(st.encrypt);
    if(j.contains("parameters"))
       j.at("parameters").get_to(st.parameters);
      
}
void to_json(json& j, const RtpCodecParameters& st) {
  
        j = json{
            {"mimeType", st.mimeType},
            {"payloadType", st.payloadType},
            {"clockRate", st.clockRate},
            {"channels", st.channels},
            {"parameters", st.parameters},
            {"rtcpFeedback", st.rtcpFeedback},
        };
        if(st.mimeType.empty())
        {
            j.erase("mimeType");
        }
        if(st.payloadType ==0)
        {
            j.erase("payloadType");
        }
        if(st.clockRate == 0)
        {
            j.erase("clockRate");
        }
        if(st.channels == 0)
        {
            j.erase("channels");
        }
        if(st.rtcpFeedback.size() ==0)
        {
            j.erase("rtcpFeedback");
        }
        if(st.parameters.size() ==0)
        {
            j.erase("parameters");
        }

     
}

void from_json(const json& j, RtpCodecParameters& st) {
    if(j.contains("mimeType"))
       j.at("mimeType").get_to(st.mimeType);
    if(j.contains("payloadType"))
       j.at("payloadType").get_to(st.payloadType);
    if(j.contains("clockRate"))
       j.at("clockRate").get_to(st.clockRate);
    if(j.contains("channels"))
       j.at("channels").get_to(st.channels);
    if(j.contains("parameters"))
       j.at("parameters").get_to(st.parameters);
    if(j.contains("rtcpFeedback"))
       j.at("rtcpFeedback").get_to(st.rtcpFeedback);
      
}



void to_json(json& j, const RtpHeaderExtension& st) {
       j = json{
           {"kind", st.kind},
           {"uri", st.uri},
           {"preferredId", st.preferredId},
           {"preferredEncrypt", st.preferredEncrypt},
           {"direction", st.direction}
       };
}

void from_json(const json& j, RtpHeaderExtension& st) {
    if(j.contains("kind"))
       j.at("kind").get_to(st.kind);
    if(j.contains("uri"))
       j.at("uri").get_to(st.uri);
    if(j.contains("preferredId"))
       j.at("preferredId").get_to(st.preferredId);
    if(j.contains("preferredEncrypt"))
       j.at("preferredEncrypt").get_to(st.preferredEncrypt);
    if(j.contains("direction"))
       j.at("direction").get_to(st.direction);
      
      
}
void to_json(json& j, const RtpCodecCapability& st) {
       j = json{
           {"kind", st.kind},
           {"mimeType", st.mimeType},
           {"preferredPayloadType", st.preferredPayloadType},
           {"clockRate", st.clockRate},
           {"channels", st.channels},
           {"parameters", st.parameters},
           {"rtcpFeedback", st.rtcpFeedback}
       };
    if(st.kind.empty())
    {
        j.erase("kind");
    }
    if(st.mimeType.empty())
    {
        j.erase("mimeType");
    }
    if(st.preferredPayloadType ==0)
    {
        j.erase("preferredPayloadType");
    }
    if(st.clockRate ==0)
    {
        j.erase("clockRate");
    }
        if(st.channels == 0)
        {
            j.erase("channels");
        }
        if(st.rtcpFeedback.size() ==0)
        {
            j.erase("rtcpFeedback");
        }
        if(st.parameters.size() ==0)
        {
            j.erase("parameters");
        }

}

void from_json(const json& j, RtpCodecCapability& st) {
    if(j.contains("kind"))
       j.at("kind").get_to(st.kind);
    if(j.contains("mimeType"))
       j.at("mimeType").get_to(st.mimeType);
    if(j.contains("preferredPayloadType"))
       j.at("preferredPayloadType").get_to(st.preferredPayloadType);
    if(j.contains("clockRate"))
       j.at("clockRate").get_to(st.clockRate);
    if(j.contains("channels"))
       j.at("channels").get_to(st.channels);
    if(j.contains("parameters"))
       j.at("parameters").get_to(st.parameters);
    if(j.contains("rtcpFeedback"))
       j.at("rtcpFeedback").get_to(st.rtcpFeedback);
      
}
void to_json(json& j, const RtpCapabilities& st) {
       j = json{
           {"codecs", st.codecs},
           {"headerExtensions", st.headerExtensions},
           {"fecMechanisms", st.fecMechanisms}
       };
}

void from_json(const json& j, RtpCapabilities& st) {
    if(j.contains("codecs"))
       j.at("codecs").get_to(st.codecs);
    if(j.contains("headerExtensions"))
       j.at("headerExtensions").get_to(st.headerExtensions);
    if(j.contains("fecMechanisms"))
       j.at("fecMechanisms").get_to(st.fecMechanisms);
      
      
}

void to_json(json& j, const RtcpParameters& st) {
       j = json{
           {"cname", st.cname},
           {"reducedSize", st.reducedSize},
           {"mux", st.mux}
       };
}

void from_json(const json& j, RtcpParameters& st) {
    if(j.contains("cname"))
       j.at("cname").get_to(st.cname);
    if(j.contains("reducedSize"))
       j.at("reducedSize").get_to(st.reducedSize);
    if(j.contains("mux"))
       j.at("mux").get_to(st.mux);
      
}






void to_json(json& j, const RtpParameters& st) {
       j = json{
           {"mid", st.mid},
           {"codecs", st.codecs},
           {"headerExtensions", st.headerExtensions},
           {"encodings", st.encodings},
           {"rtcp", st.rtcp}
       };
}

void from_json(const json& j, RtpParameters& st) {
    if(j.contains("mid"))
       j.at("mid").get_to(st.mid);
    if(j.contains("codecs"))
       j.at("codecs").get_to(st.codecs);
    if(j.contains("headerExtensions"))
       j.at("headerExtensions").get_to(st.headerExtensions);
    if(j.contains("encodings"))
       j.at("encodings").get_to(st.encodings);
    if(j.contains("rtcp"))
       j.at("rtcp").get_to(st.rtcp);
      
}
void to_json(json& j, const SctpStreamParameters& st) {
       j = json{
           {"streamId", st.streamId},
           {"ordered", st.ordered},
           {"maxPacketLifeTime", st.maxPacketLifeTime}
           
       };
}

void from_json(const json& j, SctpStreamParameters& st) {
    if(j.contains("streamId"))
       j.at("streamId").get_to(st.streamId);
    if(j.contains("ordered"))
       j.at("ordered").get_to(st.ordered);
    if(j.contains("maxPacketLifeTime"))
       j.at("maxPacketLifeTime").get_to(st.maxPacketLifeTime);
}

void to_json(json& j, const SrtpParameters& st) {
       j = json{
           {"cryptoSuite", st.cryptoSuite},
           {"keyBase64", st.keyBase64}
          
           
       };
}

void from_json(const json& j, SrtpParameters& st) {
    if(j.contains("cryptoSuite"))
       j.at("cryptoSuite").get_to(st.cryptoSuite);
    if(j.contains("keyBase64"))
       j.at("keyBase64").get_to(st.keyBase64);
      
}
void to_json(json& j, const ConsumerLayers& st) {
       j = json{{"spatialLayer", st.spatialLayer}, {"temporalLayer", st.temporalLayer}};
}

void from_json(const json& j, ConsumerLayers& st) {
    if(j.contains("spatialLayer"))
       j.at("spatialLayer").get_to(st.spatialLayer);
    if(j.contains("temporalLayer"))
       j.at("temporalLayer").get_to(st.temporalLayer);
      
}

void from_json(const json& j, ProducerScore& st) {
    if(j.contains("ssrc"))
         j.at("ssrc").get_to(st.ssrc);
    if(j.contains("rid"))
       j.at("rid").get_to(st.rid);
    if(j.contains("score"))
        j.at("score").get_to(st.score);
  }
  void to_json(json& j, const ProducerScore& st) {
         j = json{
           {"ssrc", st.ssrc},
           {"rid", st.rid},
            {"score", st.score}
          
           
       };
  }
    
  void from_json(const json& j, ProducerVideoOrientation& st) {
      if(j.contains("camera"))
          j.at("camera").get_to(st.camera);
      if(j.contains("flip"))
       j.at("flip").get_to(st.flip);
      if(j.contains("rotation"))
        j.at("rotation").get_to(st.rotation);
  }
  void to_json(json& j, const ProducerVideoOrientation& st) {
         j = json{
           {"camera", st.camera},
           {"flip", st.flip},
            {"rotation", st.rotation}
          
           
       };
  }
    
  void from_json(const json& j, ProducerTraceEventData& st) {
      if(j.contains("type"))
          j.at("type").get_to(st.type);
      if(j.contains("timestamp"))
       j.at("timestamp").get_to(st.timestamp);
      if(j.contains("direction"))
        j.at("direction").get_to(st.direction);
      if(j.contains("info"))
         j.at("info").get_to(st.info);
  }
  void to_json(json& j, ProducerTraceEventData& st) {
         j = json{
           {"type", st.type},
           {"timestamp", st.timestamp},
            {"direction", st.direction},
             {"info", st.info}
          
           
       };
  }



void to_json(json& j, const NumSctpStreams& st){
    j = json{
      {"OS", st.OS},
      {"MIS", st.MIS}
  };
}

void from_json(const json& j, NumSctpStreams& st){
    if(j.contains("OS"))
        j.at("OS").get_to(st.OS);
    
    if(j.contains("MIS"))
        j.at("MIS").get_to(st.MIS);

}

void to_json(json& j, const SctpParameters& st){
    j = json{
      {"port", st.port},
      {"OS", st.OS},
       {"MIS", st.MIS},
        {"maxMessageSize", st.maxMessageSize}
     
      
  };
}

void from_json(const json& j, SctpParameters& st){
    if(j.contains("port"))
        j.at("port").get_to(st.port);
    if(j.contains("OS"))
        j.at("OS").get_to(st.OS);
    if(j.contains("MIS"))
        j.at("MIS").get_to(st.MIS);
    if(j.contains("maxMessageSize"))
        j.at("maxMessageSize").get_to(st.maxMessageSize);
}

void to_json(json& j, const SctpCapabilities& st){
    j = json{
      {"numStreams", st.numStreams},
  };
}

void from_json(const json& j, SctpCapabilities& st){
    if(j.contains("numStreams"))
    j.at("numStreams").get_to(st.numStreams);

}

void to_json(json& j, const WebRtcTransportOptions& st){
    j = json{
      {"listenIps", st.listenIps},
      {"enableUdp", st.enableUdp},
       {"enableTcp", st.enableTcp},
        {"preferUdp", st.preferUdp},
        {"preferTcp", st.preferTcp},
        {"initialAvailableOutgoingBitrate", st.initialAvailableOutgoingBitrate},
        {"enableSctp", st.enableSctp},
        {"numSctpStreams", st.numSctpStreams},
        {"maxSctpMessageSize", st.maxSctpMessageSize},
        {"sctpSendBufferSize", st.sctpSendBufferSize},
        {"appData", st.appData}
     
      
  };
}
void from_json(const json& j, WebRtcTransportOptions& st){
    j.at("listenIps").get_to(st.listenIps);
    if(j.contains("enableUdp"))
        j.at("enableUdp").get_to(st.enableUdp);
    if(j.contains("enableTcp"))
        j.at("enableTcp").get_to(st.enableTcp);
    
    if(j.contains("preferUdp"))
        j.at("preferUdp").get_to(st.preferUdp);
    
    if(j.contains("preferTcp"))
        j.at("preferTcp").get_to(st.preferTcp);
    
    if(j.contains("initialAvailableOutgoingBitrate"))
        j.at("initialAvailableOutgoingBitrate").get_to(st.initialAvailableOutgoingBitrate);
    
    if(j.contains("enableSctp"))
        j.at("enableSctp").get_to(st.enableSctp);
    
    if(j.contains("numSctpStreams"))
        j.at("numSctpStreams").get_to(st.numSctpStreams);
    
    if(j.contains("maxSctpMessageSize"))
        j.at("maxSctpMessageSize").get_to(st.maxSctpMessageSize);
    
    if(j.contains("sctpSendBufferSize"))
        j.at("sctpSendBufferSize").get_to(st.sctpSendBufferSize);
    
    if(j.contains("appData"))
        j.at("appData").get_to(st.appData);
}
void to_json(json& j, const IceParameters& st){
    j = json{
      {"usernameFragment", st.usernameFragment},
      {"password", st.password},
       {"iceLite", st.iceLite}
       
     
      
  };
}
void from_json(const json& j, IceParameters& st){
    if(j.contains("usernameFragment"))
        j.at("usernameFragment").get_to(st.usernameFragment);
    if(j.contains("password"))
        j.at("password").get_to(st.password);
    if(j.contains("iceLite"))
        j.at("iceLite").get_to(st.iceLite);
  
}
void to_json(json& j, const IceCandidate& st){
    j = json{
      {"foundation", st.foundation},
      {"priority", st.priority},
      {"ip", st.ip},
      {"protocol", st.protocol},
      {"port", st.port},
      {"type", st.type},
      {"tcpType", st.tcpType}
    };
    if(st.tcpType.empty())
    {
        j.erase("tcpType");
    }
      

}
void from_json(const json& j, IceCandidate& st){
    if(j.contains("foundation"))
        j.at("foundation").get_to(st.foundation);
    if(j.contains("priority"))
        j.at("priority").get_to(st.priority);
    if(j.contains("ip"))
        j.at("ip").get_to(st.ip);
    if(j.contains("protocol"))
        j.at("protocol").get_to(st.protocol);
    if(j.contains("port"))
        j.at("port").get_to(st.port);
    if(j.contains("type"))
        j.at("type").get_to(st.type);
    if(j.contains("tcpType"))
        j.at("tcpType").get_to(st.tcpType);
}
void to_json(json& j, const DtlsFingerprint& st){
    j = json{
      {"algorithm", st.algorithm},
      {"value", st.value},
   
      
  };
}
void from_json(const json& j, DtlsFingerprint& st){
    if(j.contains("algorithm"))
        j.at("algorithm").get_to(st.algorithm);
    if(j.contains("value"))
        j.at("value").get_to(st.value);
 
}
void to_json(json& j, const DtlsParameters& st){
    j = json{
      {"role", st.role},
      {"fingerprints", st.fingerprints}
    
      
  };
}
void from_json(const json& j, DtlsParameters& st){
    if(j.contains("role"))
        j.at("role").get_to(st.role);
    if(j.contains("fingerprints"))
        j.at("fingerprints").get_to(st.fingerprints);
 
}

void to_json(json& j, const TransportTuple& st){
    j = json{
      {"localIp", st.localIp},
      {"localPort", st.localPort},
       {"remoteIp", st.remoteIp},
        {"remotePort", st.remotePort},
        {"protocol", st.protocol}
    
     
      
  };
}
void from_json(const json& j, TransportTuple& st){
    if(j.contains("localIp"))
        j.at("localIp").get_to(st.localIp);
    if(j.contains("localPort"))
        j.at("localPort").get_to(st.localPort);
    if(j.contains("remoteIp"))
        j.at("remoteIp").get_to(st.remoteIp);
    if(j.contains("remotePort"))
        j.at("remotePort").get_to(st.remotePort);
    if(j.contains("protocol"))
        j.at("protocol").get_to(st.protocol);
  
}

void to_json(json& j, const PlainTransportOptions& st){
    j = json{
      {"listenIp", st.listenIp},
      {"rtcpMux", st.rtcpMux},
      {"comedia", st.comedia},
      {"enableSctp", st.enableSctp},
      {"numSctpStreams", st.numSctpStreams},
       {"maxSctpMessageSize", st.maxSctpMessageSize},
        {"sctpSendBufferSize", st.sctpSendBufferSize},
         {"enableSrtp", st.enableSrtp},
           {"_appData", st._appData}
    
     
      
  };
}
void from_json(const json& j, PlainTransportOptions& st){
    if(j.contains("listenIp"))
        j.at("listenIp").get_to(st.listenIp);
    if(j.contains("rtcpMux"))
        j.at("rtcpMux").get_to(st.rtcpMux);
    if(j.contains("comedia"))
        j.at("comedia").get_to(st.comedia);
    if(j.contains("enableSctp"))
        j.at("enableSctp").get_to(st.enableSctp);
    if(j.contains("numSctpStreams"))
        j.at("numSctpStreams").get_to(st.numSctpStreams);
    if(j.contains("maxSctpMessageSize"))
        j.at("maxSctpMessageSize").get_to(st.maxSctpMessageSize);
    if(j.contains("sctpSendBufferSize"))
        j.at("sctpSendBufferSize").get_to(st.sctpSendBufferSize);
    if(j.contains("enableSrtp"))
        j.at("enableSrtp").get_to(st.enableSrtp);
    if(j.contains("_appData"))
        j.at("_appData").get_to(st._appData);
  
}

void to_json(json& j, const PipeTransportOptions& st){
    j = json{
      {"listenIp", st.listenIp},
      {"enableSctp", st.enableSctp},
       {"numSctpStreams", st.numSctpStreams},
        {"maxSctpMessageSize", st.maxSctpMessageSize},
        {"sctpSendBufferSize", st.sctpSendBufferSize},
         {"enableRtx", st.enableRtx},
          {"enableSrtp", st.enableSrtp},
            {"_appData", st._appData},
    
     
      
  };
}
void from_json(const json& j, PipeTransportOptions& st){
    if(j.contains("listenIp"))
        j.at("listenIp").get_to(st.listenIp);
    if(j.contains("enableSctp"))
        j.at("enableSctp").get_to(st.enableSctp);
    if(j.contains("numSctpStreams"))
        j.at("numSctpStreams").get_to(st.numSctpStreams);
    if(j.contains("maxSctpMessageSize"))
        j.at("maxSctpMessageSize").get_to(st.maxSctpMessageSize);
    if(j.contains("sctpSendBufferSize"))
        j.at("sctpSendBufferSize").get_to(st.sctpSendBufferSize);
    if(j.contains("enableRtx"))
        j.at("enableRtx").get_to(st.enableRtx);
    if(j.contains("enableSrtp"))
        j.at("enableSrtp").get_to(st.enableSrtp);
    if(j.contains("_appData"))
        j.at("_appData").get_to(st._appData);
  
}
    void to_json(json& j, const ConsumerScore& st) {
        j = json{
                {"score", st.score},
                {"producerScore", st.producerScore},
                {"producerScores", st.producerScores}
        };
    }

    void from_json(const json& j, ConsumerScore& st) {
        if(j.contains("score"))
            j.at("score").get_to(st.score);
        if(j.contains("producerScore"))
            j.at("producerScore").get_to(st.producerScore);
        if(j.contains("producerScores"))
            j.at("producerScores").get_to(st.producerScores);


    }

    void to_json(json& j, const ConsumerTraceEventData& st) {
        j = json{
                {"type", st.type},
                {"timestamp", st.timestamp},
                {"direction", st.direction},
                {"info", st.info}
        };
    }

    void from_json(const json& j, ConsumerTraceEventData& st) {
        if(j.contains("type"))
            j.at("type").get_to(st.type);
        if(j.contains("timestamp"))
            j.at("timestamp").get_to(st.timestamp);
        if(j.contains("direction"))
            j.at("direction").get_to(st.direction);
        if(j.contains("info"))
            j.at("info").get_to(st.info);


    }

    void to_json(json& j, const PushTransportOptions& st) {
        j = json{
            {"appData", st.appData}
        };
    }
    void from_json(const json& j, PushTransportOptions& st) {
        if (j.contains("appData"))
            j.at("appData").get_to(st.appData);
    }
}

