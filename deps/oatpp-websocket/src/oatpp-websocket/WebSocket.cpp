/***************************************************************************
 *
 * Project         _____    __   ____   _      _
 *                (  _  )  /__\ (_  _)_| |_  _| |_
 *                 )(_)(  /(__)\  )( (_   _)(_   _)
 *                (_____)(__)(__)(__)  |_|    |_|
 *
 *
 * Copyright 2018-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
 *
 * Licensed under the Apache License, Version 2.0 (the "License");
 * you may not use this file except in compliance with the License.
 * You may obtain a copy of the License at
 *
 *     http://www.apache.org/licenses/LICENSE-2.0
 *
 * Unless required by applicable law or agreed to in writing, software
 * distributed under the License is distributed on an "AS IS" BASIS,
 * WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 * See the License for the specific language governing permissions and
 * limitations under the License.
 *
 ***************************************************************************/

#include "WebSocket.hpp"

#include "./Utils.hpp"

#if defined(WIN32) || defined(_WIN32)
  #include <WinSock2.h>
#else
  #include <arpa/inet.h>
#endif

namespace oatpp { namespace websocket {

WebSocket::WebSocket(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, const Config& config)
  : m_config(config)
  , m_connection(connection)
  , m_listener(nullptr)
  , m_lastOpcode(-1)
  , m_listening(false)
{}

WebSocket::WebSocket(const std::shared_ptr<oatpp::data::stream::IOStream>& connection, bool maskOutgoingMessages)
  : m_connection(connection)
  , m_listener(nullptr)
  , m_lastOpcode(-1)
  , m_listening(false)
{
  m_config.maskOutgoingMessages = maskOutgoingMessages;
}

void WebSocket::setConfig(const Config& config) {
  m_config = config;
}

bool WebSocket::checkForContinuation(const Frame::Header& frameHeader) {
  if(m_lastOpcode == Frame::OPCODE_TEXT || m_lastOpcode == Frame::OPCODE_BINARY) {
    return false;
  }
  if(frameHeader.fin) {
    m_lastOpcode = -1;
  } else {
    m_lastOpcode = frameHeader.opcode;
  }
  return true;
}
  
void WebSocket::readFrameHeader(Frame::Header& frameHeader) const {
  
  v_uint16 bb;
  auto res = m_connection->readExactSizeDataSimple(&bb, 2);
  if(res != 2) {
    throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::readFrameHeader()]: Error reading frame header");
  }
  
  v_uint8 messageLen1;
  Frame::unpackHeaderBits(ntohs(bb), frameHeader, messageLen1);
  
  if(messageLen1 < 126) {
    frameHeader.payloadLength = messageLen1;
  } else if(messageLen1 == 126) {
    v_uint16 messageLen2;
    res = m_connection->readExactSizeDataSimple(&messageLen2, 2);
    if(res != 2) {
      throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::readFrameHeader()]: Error reading frame header. Reading payload length scenario 2.");
    }
    frameHeader.payloadLength = ntohs(messageLen2);
  } else if(messageLen1 == 127) {
    v_uint32 messageLen3[2];
    res = m_connection->readExactSizeDataSimple(&messageLen3, 8);
    if(res != 8) {
      throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::readFrameHeader()]: Error reading frame header. Reading payload length scenario 3.");
    }
    frameHeader.payloadLength = (((v_uint64) ntohl(messageLen3[0])) << 32) | ntohl(messageLen3[1]);
  }
  
  if(frameHeader.hasMask) {
    res = m_connection->readExactSizeDataSimple(frameHeader.mask, 4);
    if(res != 4) {
      throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::readFrameHeader()]: Error reading frame header. Reading mask.");
    }
  }
  
}
  
void WebSocket::writeFrameHeader(const Frame::Header& frameHeader) const {

  v_uint16 bb;
  v_uint8 messageLengthScenario;
  Frame::packHeaderBits(bb, frameHeader, messageLengthScenario);
  
  bb = htons(bb);
  
  auto res = m_connection->writeExactSizeDataSimple(&bb, 2);
  if(res != 2) {
    throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::writeFrameHeader()]: Error writing frame header");
  }
  
  if(messageLengthScenario == 2) {
    v_uint16 messageLen2 = htons(frameHeader.payloadLength);
    res = m_connection->writeExactSizeDataSimple(&messageLen2, 2);
    if(res != 2) {
      throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::writeFrameHeader()]: Error writing frame header. Writing payload length scenario 2.");
    }
  } else if(messageLengthScenario == 3) {
    v_uint32 messageLen3[2];
    messageLen3[0] = htonl(frameHeader.payloadLength >> 32);
    messageLen3[1] = htonl(frameHeader.payloadLength & 0xFFFFFFFF);
    res = m_connection->writeExactSizeDataSimple(&messageLen3, 8);
    if(res != 8) {
      throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::writeFrameHeader()]: Error writing frame header. Writing payload length scenario 3.");
    }
  }
  
  if(frameHeader.hasMask) {
    res = m_connection->writeExactSizeDataSimple(frameHeader.mask, 4);
    if(res != 4) {
      throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::writeFrameHeader()]: Error writing frame header. Writing mask.");
    }
  }
  
}
  
void WebSocket::readPayload(const Frame::Header& frameHeader, oatpp::data::stream::ChunkedBuffer* shortMessageStream) const {
  
  if(shortMessageStream && frameHeader.payloadLength > 125) {
    throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::readPayload()]: Invalid payloadLength. See RFC-6455, section-5.5.");
  }
  
  std::unique_ptr<v_char8[]> buffer(new v_char8[m_config.readBufferSize]);
  oatpp::v_io_size progress = 0;
  
  while (progress < frameHeader.payloadLength) {
    
    oatpp::v_io_size desiredSize = m_config.readBufferSize;
    if(desiredSize > frameHeader.payloadLength - progress) {
      desiredSize = frameHeader.payloadLength - progress;
    }
    
    auto res = m_connection->readSimple(buffer.get(), desiredSize);
    
    if(res > 0) {
      
      if(frameHeader.hasMask) {
		  std::unique_ptr<v_char8[]> decoded(new v_char8[res]);
        for(v_int32 i = 0; i < res; i ++) {
          decoded.get()[i] = buffer.get()[i] ^ frameHeader.mask[(i + progress) % 4];
        }
        if(shortMessageStream) {
          shortMessageStream->writeSimple(decoded.get(), res);
        } else if(m_listener) {
          m_listener->readMessage(*this, frameHeader.opcode, decoded.get(), res);
        }
      } else {
        if(shortMessageStream) {
          shortMessageStream->writeSimple(buffer.get(), res);
        } else if(m_listener) {
          m_listener->readMessage(*this, frameHeader.opcode, buffer.get(), res);
        }
      }
      progress += res;
      
    } else { // if res == 0 then probably stream handles read() error incorrectly. trow.
      
      if(res == oatpp::IOError::RETRY_READ || res == oatpp::IOError::RETRY_WRITE) {
        continue;
      }
      throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::readPayload()]: Invalid connection state.");
      
    }
  }
  
  /* call listener to inform abount messge end */
  if(shortMessageStream == nullptr && frameHeader.fin && m_listener) {
    m_listener->readMessage(*this, frameHeader.opcode, nullptr, 0);
  }
  
}
  
void WebSocket::handleFrame(const Frame::Header& frameHeader) {
  
  switch (frameHeader.opcode) {
    case Frame::OPCODE_CONTINUATION:
      if(m_lastOpcode < 0) {
        throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::handleFrame()]: Invalid communication state.");
      }
      readPayload(frameHeader, nullptr);
      break;
      
    case Frame::OPCODE_TEXT:
      if(checkForContinuation(frameHeader)) {
        readPayload(frameHeader, nullptr);
      } else {
        throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::handleFrame()]: Invalid communication state. OPCODE_CONTINUATION expected");
      }
      break;
      
    case Frame::OPCODE_BINARY:
      if(checkForContinuation(frameHeader)) {
        readPayload(frameHeader, nullptr);
      } else {
        throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::handleFrame()]: Invalid communication state. OPCODE_CONTINUATION expected");
      }
      break;
      
    case Frame::OPCODE_CLOSE:
      {
        oatpp::data::stream::ChunkedBuffer messageStream;
        readPayload(frameHeader, &messageStream);
        if(m_listener) {
          v_uint16 code = 0;
          oatpp::String message;
          if(messageStream.getSize() >= 2) {
            messageStream.readSubstring(&code, 0, 2);
            code = ntohs(code);
            message = messageStream.getSubstring(2, messageStream.getSize() - 2);
          }
          if(!message) {
            message = "";
          }
          m_listener->onClose(*this, code, message);
        }
      }
      break;
      
    case Frame::OPCODE_PING:
      {
        oatpp::data::stream::ChunkedBuffer messageStream;
        readPayload(frameHeader, &messageStream);
        if(m_listener) {
          m_listener->onPing(*this, messageStream.toString());
        }
      }
      break;
      
    case Frame::OPCODE_PONG:
      {
        oatpp::data::stream::ChunkedBuffer messageStream;
        readPayload(frameHeader, &messageStream);
        if(m_listener) {
          m_listener->onPong(*this, messageStream.toString());
        }
      }
      break;
      
    default:
      throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::handleFrame()]: Unknown frame");
      break;
  }
  
}
  
void WebSocket::iterateFrame(Frame::Header& frameHeader) {
  readFrameHeader(frameHeader);
  handleFrame(frameHeader);
}
  
void WebSocket::listen() {
  
  m_listening = true;
  
  try {
    Frame::Header frameHeader;
    do {
      readFrameHeader(frameHeader);
      handleFrame(frameHeader);
    } while(frameHeader.opcode != Frame::OPCODE_CLOSE && m_listening);
  } catch(const std::runtime_error& error) {
    OATPP_LOGD("[oatpp::web::protocol::websocket::WebSocket::listen()]", "Unhandled error occurred. Message='%s'", error.what());
  } catch(...) {
    OATPP_LOGD("[oatpp::web::protocol::websocket::WebSocket::listen()]", "Unhandled error occurred");
  }
  
}
  
void WebSocket::stopListening() const {
  m_listening = false;
}
  
void WebSocket::sendFrameHeader(Frame::Header& frameHeader, bool fin, v_uint8 opcode, v_int64 messageSize) const {
  frameHeader.fin = fin;
  frameHeader.rsv1 = false;
  frameHeader.rsv2 = false;
  frameHeader.rsv3 = false;
  frameHeader.opcode = opcode;
  frameHeader.hasMask = m_config.maskOutgoingMessages;
  frameHeader.payloadLength = messageSize;
  
  if(frameHeader.hasMask) {
    Utils::generateMaskForFrame(frameHeader);
  }
  
  writeFrameHeader(frameHeader);
}
  
bool WebSocket::sendOneFrame(bool fin, v_uint8 opcode, const oatpp::String& message) const {
  Frame::Header frameHeader;
  if(message && message->getSize() > 0) {
    sendFrameHeader(frameHeader, fin, opcode, message->getSize());
    oatpp::v_io_size res;
    if(frameHeader.hasMask) {
		std::unique_ptr<v_char8[]> encoded(new v_char8[message->getSize()]);
      for(v_int32 i = 0; i < message->getSize(); i ++) {
        encoded.get()[i] = message->getData()[i] ^ frameHeader.mask[i % 4];
      }
      res = m_connection->writeExactSizeDataSimple(encoded.get(), message->getSize());
    } else {
      res = m_connection->writeExactSizeDataSimple(message->getData(), message->getSize());
    }
    if(res != message->getSize()) {
      return false;
    }
  } else {
    sendFrameHeader(frameHeader, fin, opcode, 0);
  }
  return true;
}
  
void WebSocket::sendClose(v_uint16 code, const oatpp::String& message) const {
  
  code = htons(code);
  
  oatpp::data::stream::ChunkedBuffer buffer;
  buffer.writeSimple(&code, 2);
  if(message) {
    buffer.writeSimple(message->getData(), message->getSize());
  }
  
  if(!sendOneFrame(true, Frame::OPCODE_CLOSE, buffer.toString())) {
    stopListening();
    throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::sendClose(...)]: Unknown error while writing to socket.");
  }
  
}

void WebSocket::sendClose() const {
  if(!sendOneFrame(true, Frame::OPCODE_CLOSE, nullptr)) {
    stopListening();
    throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::sendClose()]: Unknown error while writing to socket.");
  }
}
  
void WebSocket::sendPing(const oatpp::String& message) const {
  if(!sendOneFrame(true, Frame::OPCODE_PING, message)) {
    stopListening();
    throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::sendPing()]: Unknown error while writing to socket.");
  }
}

void WebSocket::sendPong(const oatpp::String& message) const {
  if(!sendOneFrame(true, Frame::OPCODE_PONG, message)) {
    stopListening();
    throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::sendPong()]: Unknown error while writing to socket.");
  }
}

void WebSocket::sendOneFrameText(const oatpp::String& message) const {
  if(!sendOneFrame(true, Frame::OPCODE_TEXT, message)) {
    stopListening();
    throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::sendOneFrameText()]: Unknown error while writing to socket.");
  }
}

void WebSocket::sendOneFrameBinary(const oatpp::String& message) const {
  if(!sendOneFrame(true, Frame::OPCODE_BINARY, message)) {
    stopListening();
    throw std::runtime_error("[oatpp::web::protocol::websocket::WebSocket::sendOneFrameBinary()]: Unknown error while writing to socket.");
  }
}
  
}}
