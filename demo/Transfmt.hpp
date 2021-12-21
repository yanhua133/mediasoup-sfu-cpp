#pragma once
#include <string>
#include "./rooms/Peer.hpp"
#include "Router.hpp"
#include "Producer.hpp"
#include "json.hpp"
#include "Config.hpp"

#define LIVE_MARK                         0x32505553 // SUP2(sohulive2)
#define LIVE_NAME_SIZE                    64
#define TRANSFMT_CREATE_STREAM_RTC_RTMP 0xb00
#define TRANSFMT_CLOSE_STREAM_RTC_RTMP 0xb01
#define TRANSFMT_CREATE_STREAM_RTMP_RTC 0xb02
#define TRANSFMT_CLOSE_STREAM_RTMP_RTC 0xb03


typedef struct {
    uint32_t pkg_len;
    uint32_t pkg_mark;
    uint16_t pkg_code;
} PkgHeader;

// heart
typedef struct {
    PkgHeader header;
} PkgHeart;

typedef struct TransfmtCreateRtc2Rtmp {
  PkgHeader header;
  char name[LIVE_NAME_SIZE];
  char roomid[LIVE_NAME_SIZE];
  uint8_t is_audio;
  uint8_t is_video;
  uint8_t trans_code;
  uint16_t width;
  uint16_t height;
  uint16_t bitrate;
  uint16_t frame_rate;
  uint16_t gop;
  uint16_t profile;
} TransfmtCreateRtc2Rtmp;

typedef struct TransfmtCloseRtc2Rtmp {
  PkgHeader header;
  char name[LIVE_NAME_SIZE];
} TransfmtCloseRtc2Rtmp;
class Transfmt
{
public:
    Transfmt();
    ~Transfmt();
    
public:
    void stopRecord(std::string peerId);
    void startRecord(std::shared_ptr<Router> router,std::shared_ptr<Peer> peer, std::shared_ptr<Producer> producer,std::string roomId);
    json getCodecInfoFromRtpParameters(std::string kind, json jrtpParameters);
    void getSdpData(json audio,json audioCodecInfo,json video ,json videoCodecInfo,std::string  sdpData);
    json createSdpText(json &rtpParameters);
    void writeSdpFile(std::string &userId,json &rtpParameters);

    void setConfig(Config &config1) {
        //this->config = config;
        this->config.initConfig();
    }
    void getPkgHeader(PkgHeader &header);
    void getTransfmtCreateRtc2RtmpInfo(std::string &name,std::string &roomid,TransfmtCreateRtc2Rtmp & data);
    void getTransfmtCloseRtc2RtmpInfo(std::string &name, TransfmtCloseRtc2Rtmp &data);
    json publishRtpStream(std::shared_ptr<Router> router,std::shared_ptr<Peer> peer, std::shared_ptr<Producer> producer);
    int getRandomPort ();
    int getPort();
    int getRandomNumber();

private:
    
    int minPort = 0;
    int maxPort = 0;
    int mTimeout=400;
    
    Config config;

};

