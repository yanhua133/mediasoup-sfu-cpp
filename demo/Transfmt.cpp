#include "Transfmt.hpp"
#include <string>
#include <sstream>
#include <iostream> 
#include "Log.hpp"
#include "Router.hpp"
#include "Producer.hpp"
#include "./rooms/Peer.hpp"
#include "json.hpp"

std::string& replace_str(std::string& str, const std::string& to_replaced, const std::string& newchars)
{
    for(std::string::size_type pos(0); pos != std::string::npos; pos += newchars.length())
    {
        pos = str.find(to_replaced,pos);
        if(pos!=std::string::npos)
           str.replace(pos,to_replaced.length(),newchars);
        else
            break;
    }
    return   str;
}
Transfmt::Transfmt()
{

}
Transfmt::~Transfmt()
{
  
}
// Gets codec information from rtpParameters
json Transfmt::getCodecInfoFromRtpParameters(std::string kind, json jrtpParameters)
{
    RtpParameters rtpParameters = jrtpParameters;
    std::string to_replaced = kind+"/";
    std::string codecName = replace_str(rtpParameters.codecs[0].mimeType,to_replaced,"");
  return {
    {"payloadType", rtpParameters.codecs[0].payloadType},
    {"codecName", codecName},
    {"clockRate", rtpParameters.codecs[0].clockRate},
    {"channels", kind == "audio" ? rtpParameters.codecs[0].channels : 0}
  };
};

void Transfmt::getSdpData(json audio,json audioCodecInfo,json video ,json videoCodecInfo,std::string  sdpData)
{
    std::stringstream stream;
    stream << "v=0o=- 0 0 IN IP4 127.0.0.1\n"; 
    stream << "s=FFmpeg\n"; 
    stream << "c=IN IP4 127.0.0.1\n"; 
    stream << "t=0 0\n"; 
    stream << "m=video ";
    stream << video["remoteRtpPort"];
    stream << " RTP/AVP ";
    stream << videoCodecInfo["payloadType"];
    stream << " \n"; 
    stream << "a=rtpmap:";
    stream << videoCodecInfo["payloadType"];
    stream <<" ";
    stream <<videoCodecInfo["codecName"];
    stream <<"/";
    stream <<videoCodecInfo["clockRate"];
    stream <<"\n"; 
    stream << "a=sendonly\n"; 
    stream << "m=audio " << audio["remoteRtpPort"] <<" RTP/AVP " <<audioCodecInfo["payloadType"] << "\n";  
    stream << "a=rtpmap:" << audioCodecInfo["payloadType"] << " "<< audioCodecInfo["codecName"]<<"/"<<audioCodecInfo["clockRate"]<<"/"<<audioCodecInfo["channels"]<<"\n"; 
    stream << "a=sendonly\n";
    stream >> sdpData; 
    
}
// File to create SDP text from mediasoup RTP Parameters
json Transfmt::createSdpText(json &rtpParameters)
{
  MS_lOGD("rtpParameters=%s",rtpParameters.dump().c_str());
  json &video = rtpParameters["video"];
  json &audio = rtpParameters["audio"];
  
  // Video codec info
  auto videoCodecInfo = getCodecInfoFromRtpParameters("video", video["rtpParameters"]);

  // Audio codec info
  auto audioCodecInfo = getCodecInfoFromRtpParameters("audio", audio["rtpParameters"]);

  json sdpData;
  getSdpData(audio,audioCodecInfo,video,videoCodecInfo,sdpData);
  return sdpData;
};

void Transfmt::writeSdpFile(std::string &userId,json &rtpParameters) {
  auto sdp = createSdpText(rtpParameters);
  MS_lOGD("createSdpText %s",sdp.dump().c_str());
  /*
  fs.writeFile("/tmp/"+userId+".sdp", sdp, function (error) {
    if (error) {
      MS_lOGD("sdp 写入失败")
    } else {
      MS_lOGD("sdp 写入成功了")
    }
  })*/
}


void Transfmt::getPkgHeader(PkgHeader &header) {

	header.pkg_mark = LIVE_MARK;
	header.pkg_code = TRANSFMT_CREATE_STREAM_RTC_RTMP;

}
void Transfmt::getTransfmtCreateRtc2RtmpInfo(std::string &name,std::string &roomid,TransfmtCreateRtc2Rtmp & data)
{
    getPkgHeader(data.header);
    data.header.pkg_len = sizeof(TransfmtCreateRtc2Rtmp);
    strcpy(data.name,name.c_str());
    strcpy(data.roomid,roomid.c_str());
    data.is_audio = 1;
    data.is_video = 1;
    data.trans_code = 0;
    data.width = 1280;
    data.height = 720 ;
    data.bitrate = 800;
    data.frame_rate = 25;
    data.gop = 5;
    data.profile = 0 ;

}
void Transfmt::getTransfmtCloseRtc2RtmpInfo(std::string &name, TransfmtCloseRtc2Rtmp &data) {

  PkgHeader header;
  getPkgHeader(data.header);
  header.pkg_len = sizeof(TransfmtCloseRtc2Rtmp);
  strcpy(data.name,name.c_str());
}


#include <random>

using std::default_random_engine;
using std::uniform_real_distribution;

int Transfmt::getRandomNumber() {
    default_random_engine e;
    uniform_real_distribution<double> u(0, 1);
    return int(u(e) * 1000);
}

int Transfmt::getRandomPort ()
{
    int random = getRandomNumber();
    int port = (random * (config.rtp["maxPort"].get<int>() - config.rtp["minPort"].get<int>() + 1) + config.rtp["minPort"].get<int>());
    return port;
}
int Transfmt::getPort()
{

	int port = getRandomPort();
	return port;

};

json Transfmt::publishRtpStream(std::shared_ptr<Router> router,std::shared_ptr<Peer> peer, std::shared_ptr<Producer> producer)
{
	MS_lOGD("publishRtpStream()");

	// Create the mediasoup RTP Transport used to send media to the GStreamer process
	json rtpTransportConfig = {
		{"listenIp" , config.mediasoup.plainTransportOptions["listenIp"]["announcedIp"]},
		{"rtcpMux"  , true},
		{"comedia"  , false}
	};
  
    MS_lOGD("rtpTransportConfig=%s",rtpTransportConfig.dump().c_str());
    PlainTransportOptions options;
    options.listenIp = config.mediasoup.plainTransportOptions["listenIp"]["announcedIp"];
    options.rtcpMux = true;
    options.comedia = false;
	auto rtpTransport =  router->createPlainTransport(options);

	// Set the receiver RTP ports
	auto remoteRtpPort =  getPort();

	//MS_lOGD("remoteRtpPort = ", remoteRtpPort);

	// Connect the mediasoup RTP transport to the ports used by GStreamer
      json sendRtpConfig = {
          {"ip"   , config.rtp["ip"]},
          {"port" , remoteRtpPort}
      };
        
      rtpTransport->connect(sendRtpConfig);
      MS_lOGD("sendRtpConfig=%s",(sendRtpConfig.dump().c_str()));

	peer->data.transports[rtpTransport->id()] =  rtpTransport;

	json codecs = json::array();
 
	// Codec passed to the RTP Consumer must match the codec in the Mediasoup router rtpCapabilities
	// auto routerCodec = router->rtpCapabilities.codecs.find(
	// 	(codec) => codec.kind == producer->kind
	// );
  json routerCodec;
  for(auto & codec : router->rtpCapabilities().codecs )
  {
      if(codec.kind == producer->kind())
      {
          routerCodec = codec;
          break;
      }
  }

	codecs.push_back(routerCodec);

	json rtpCapabilities = {
		{"codecs",codecs},
		{"rtcpFeedback",json::array()}
	};

	// MS_lOGD(rtpCapabilities);
	// MS_lOGD(codecs[0].parameters);
	// MS_lOGD(rtpTransport->tuple);

	// Start the consumer paused
	// Once the gstreamer process is ready to consume resume and send a keyframe
  ConsumerOptions consumerOptions;
    consumerOptions.producerId = producer->id();
    consumerOptions.rtpCapabilities = rtpCapabilities;
    consumerOptions.paused = true;
	auto rtpConsumer =  rtpTransport->consume(consumerOptions);

	peer->data.consumers[rtpConsumer->id()] =  rtpConsumer;

	return {
		{"remoteRtpPort",remoteRtpPort},
		{"localRtcpPort" , rtpTransport->rtcpTuple().localPort},
		{"rtpCapabilities",rtpCapabilities},
		{"rtpParameters" , rtpConsumer->rtpParameters()}
	};
};

void Transfmt::startRecord(std::shared_ptr<Router> router,std::shared_ptr<Peer> peer, std::shared_ptr<Producer> producer,std::string roomId)
{
	json recordInfo = json::object();

	for (auto &kv : peer->data.producers)
	{
        auto producer = kv.second;
		recordInfo[producer->kind()] =  publishRtpStream(router, peer, producer);
		MS_lOGD("publishRtpStream kind=%s  info=%s",producer->kind().c_str(),recordInfo[producer->kind()].dump().c_str());
	}

  MS_lOGD("startRecord userId=%s  roomId=%s",peer->getPeerId().c_str(),roomId.c_str());
    auto peerId = peer->getPeerId();
  writeSdpFile(peerId, recordInfo);
    TransfmtCreateRtc2Rtmp data;
    getTransfmtCreateRtc2RtmpInfo(peerId,roomId,data);
 // MS_lOGD(buf);
  //auto ret = tcp_client.write(buf);
  //MS_lOGD("write ret="+ret);

	for (auto &kv : peer->data.consumers)
	{
        auto consumer = kv.second;
		 consumer->resume();
	}
	// setTimeout(async () => 
	// {
	// 	for (auto * consumer : peer.data.consumers) 
	// 	{
	// 		 consumer->resume();
	// 	}
	// }, 1000);
};
void Transfmt::stopRecord(std::string peerId)
{
  MS_lOGD("stopRecord userId=%s",peerId.c_str());
  TransfmtCloseRtc2Rtmp data;
  getTransfmtCloseRtc2RtmpInfo(peerId,data);
  //MS_lOGD(buf);
  //auto ret = tcp_client.write(buf);
  //MS_lOGD("write ret="+ret);

	
};
