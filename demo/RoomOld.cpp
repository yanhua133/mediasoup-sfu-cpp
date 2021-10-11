#include "Room.hpp"
#include <iostream>
#include <string>
#include <cstring>


void to_json(json& j, const _device& st){
    j = json{
      {"flag", st.flag},
      {"name", st.name},
       {"version", st.version}
  };
}
void from_json(const json& j, _device& st){
    j.at("flag").get_to(st.flag);
  j.at("name").get_to(st.name);
   j.at("version").get_to(st.version);
  
}

void to_json(json& j, const pairInfo& st){
    j = json{
      {"id", st.id},
      {"kind", st.kind}
  };
}
void from_json(const json& j, pairInfo& st){
    j.at("id").get_to(st.id);
    j.at("kind").get_to(st.kind);
 
  
}
 
void to_json(json& j, const PeerInfo& st){
    j = json{
      {"id", st.id},
      {"displayName", st.displayName},
      {"device", st.device},
      {"producerInfo", st.producerInfo},
      {"producers", st.producers}
  };
}
void from_json(const json& j, PeerInfo& st){
    j.at("id").get_to(st.id);
    j.at("displayName").get_to(st.displayName);
    j.at("device").get_to(st.device);
    j.at("producerInfo").get_to(st.producerInfo);
    j.at("producers").get_to(st.producers);
 
  
}
std::string Base64::Encode(const unsigned char * str,int bytes) {
    int num = 0,bin = 0,i;
    std::string _encode_result;
    const unsigned char * current;
    current = str;
    while(bytes > 2) {
        _encode_result += _base64_table[current[0] >> 2];
        _encode_result += _base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
        _encode_result += _base64_table[((current[1] & 0x0f) << 2) + (current[2] >> 6)];
        _encode_result += _base64_table[current[2] & 0x3f];

        current += 3;
        bytes -= 3;
    }
    if(bytes > 0)
    {
        _encode_result += _base64_table[current[0] >> 2];
        if(bytes%3 == 1) {
            _encode_result += _base64_table[(current[0] & 0x03) << 4];
            _encode_result += "==";
        } else if(bytes%3 == 2) {
            _encode_result += _base64_table[((current[0] & 0x03) << 4) + (current[1] >> 4)];
            _encode_result += _base64_table[(current[1] & 0x0f) << 2];
            _encode_result += "=";
        }
    }
    return _encode_result;
}
std::string Base64::Decode(const char *str,int length) {
       //解码表
    const int DecodeTable[] =
    {
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -1, -1, -2, -2, -1, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -1, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, 62, -2, -2, -2, 63,
        52, 53, 54, 55, 56, 57, 58, 59, 60, 61, -2, -2, -2, -2, -2, -2,
        -2,  0,  1,  2,  3,  4,  5,  6,  7,  8,  9, 10, 11, 12, 13, 14,
        15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, -2, -2, -2, -2, -2,
        -2, 26, 27, 28, 29, 30, 31, 32, 33, 34, 35, 36, 37, 38, 39, 40,
        41, 42, 43, 44, 45, 46, 47, 48, 49, 50, 51, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2,
        -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2, -2
    };
    int bin = 0,i=0,pos=0;
    std::string _decode_result;
    const char *current = str;
    char ch;
    while( (ch = *current++) != '\0' && length-- > 0 )
    {
        if (ch == base64_pad) { // 当前一个字符是“=”号
            /*
            先说明一个概念：在解码时，4个字符为一组进行一轮字符匹配。
            两个条件：
                1、如果某一轮匹配的第二个是“=”且第三个字符不是“=”，说明这个带解析字符串不合法，直接返回空
                2、如果当前“=”不是第二个字符，且后面的字符只包含空白符，则说明这个这个条件合法，可以继续。
            */
            if (*current != '=' && (i % 4) == 1) {
                return NULL;
            }
            continue;
        }
        ch = (char)DecodeTable[ch];
        //这个很重要，用来过滤所有不合法的字符
        if (ch < 0 ) { /* a space or some other separator character, we simply skip over */
            continue;
        }
        switch(i % 4)
        {
            case 0:
                bin = ch << 2;
                break;
            case 1:
                bin |= ch >> 4;
                _decode_result += bin;
                bin = ( ch & 0x0f ) << 4;
                break;
            case 2:
                bin |= ch >> 2;
                _decode_result += bin;
                bin = ( ch & 0x03 ) << 6;
                break;
            case 3:
                bin |= ch;
                _decode_result += bin;
                break;
        }
        i++;
    }
    return _decode_result;
}
/**
 https://www.cnblogs.com/lrxing/p/5535601.html
 int main()
 {
     unsigned char str[] = "这是一条测试数据：http://img.v.cmcm.com/7dd6e6d7-0c2d-5a58-a072-71f828b94cbc_crop_216x150.jpg";
     string normal,encoded;
     int i,len = sizeof(str)-1;
     Base64 *base = new Base64();
     encoded = base->Encode(str,len);
     cout << "base64 encode : " << encoded << endl;
     len = encoded.length();
     const char * str2 = encoded.c_str();
     normal = base->Decode(str2,len);
     cout << "base64 decode : " << normal <<endl;
     return 0;
 }
 
 */
