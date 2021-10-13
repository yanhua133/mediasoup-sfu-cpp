/***************************************************************************
 *
 * Project:   ______                ______ _
 *           / _____)              / _____) |          _
 *          | /      ____ ____ ___| /     | | _   ____| |_
 *          | |     / _  |  _ (___) |     | || \ / _  |  _)
 *          | \____( ( | | | | |  | \_____| | | ( ( | | |__
 *           \______)_||_|_| |_|   \______)_| |_|\_||_|\___)
 *
 *
 * Copyright 2020-present, Leonid Stryzhevskyi <lganzzzo@gmail.com>
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

#include "Room.hpp"

oatpp::String Room::getId() {
  return m_id;
}

void Room::addPeer(const std::shared_ptr<Peer>& peer) {
  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  m_peerById[peer->getPeerId()] = peer;
}

void Room::welcomePeer(const std::shared_ptr<Peer>& peer) {

  /* Inform all that peer have joined the room */

  auto joinedMessage = MessageDto::createShared();
  joinedMessage->code = MessageCodes::CODE_PEER_JOINED;
  joinedMessage->peerId = oatpp::String(peer->getPeerId().c_str());
  joinedMessage->peerName = peer->getNickname();
  joinedMessage->message = peer->getNickname() + " - joined room";

  addHistoryMessage(joinedMessage);
  sendMessageAsync(joinedMessage);

}

void Room::onboardPeer(const std::shared_ptr<Peer>& peer) {

//  auto infoMessage = MessageDto::createShared();
//  infoMessage->code = MessageCodes::CODE_INFO;
//  //infoMessage->peerId = peer->getPeerId();
//  infoMessage->peerName = peer->getNickname();
//
//  infoMessage->peers = {};
//
//  {
//    std::lock_guard<std::mutex> guard(m_peerByIdLock);
//    for (auto &it : m_peerById) {
//      auto p = PeerDto::createShared();
////      p->peerId = it.second->getPeerId();
////      p->peerName = it.second->getNickname();
////      infoMessage->peers->push_back(p);
//    }
//  }
//
//  infoMessage->history = getHistory();
//  peer->sendMessageAsync(infoMessage);

}

void Room::goodbyePeer(const std::shared_ptr<Peer>& peer) {

  auto message = MessageDto::createShared();
  message->code = MessageCodes::CODE_PEER_LEFT;
  //message->peerId = peer->getPeerId();
  message->message = peer->getNickname() + " - left room";

  addHistoryMessage(message);
  sendMessageAsync(message);

}

std::shared_ptr<Peer> Room::getPeerById(std::string peerId) {
  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  auto it = m_peerById.find(peerId);
  if(it != m_peerById.end()) {
    return it->second;
  }
  return nullptr;
}

void Room::removePeerById(std::string peerId) {

  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  auto peer = m_peerById.find(peerId);

  if(peer != m_peerById.end()) {
    m_peerById.erase(peerId);

  }

}

void Room::addHistoryMessage(const oatpp::Object<MessageDto>& message) {

  if(!m_appConfig->maxRoomHistoryMessages || *m_appConfig->maxRoomHistoryMessages == 0) {
    return;
  }

  std::lock_guard<std::mutex> guard(m_historyLock);

  m_history.push_back(message);

  while(m_history.size() > m_appConfig->maxRoomHistoryMessages) {
    m_history.pop_front();
  }

}

void Room::handleRequest(json request, std::function<void(json data)> const & accept, std::function<void(int errorCode, std::string errorReason)> const & reject){

}

void Room::handleNotification(json notification){

}


oatpp::List<oatpp::Object<MessageDto>> Room::getHistory() {

  if(!m_appConfig->maxRoomHistoryMessages || *m_appConfig->maxRoomHistoryMessages == 0) {
    return nullptr;
  }

  auto result = oatpp::List<oatpp::Object<MessageDto>>::createShared();

  std::lock_guard<std::mutex> guard(m_historyLock);

  for(auto& message : m_history) {
    result->push_back(message);
  }

  return result;

}

void Room::sendMessageAsync(const oatpp::Object<MessageDto>& message) {
//  std::lock_guard<std::mutex> guard(m_peerByIdLock);
//  for(auto& pair : m_peerById) {
//    pair.second->sendMessageAsync(message);
//  }
}

void Room::pingAllPeers() {
  std::lock_guard<std::mutex> guard(m_peerByIdLock);
  for(auto& pair : m_peerById) {
    auto& peer = pair.second;
    if(!peer->sendPingAsync()) {
      peer->invalidateSocket();
      ++ m_statistics->EVENT_PEER_ZOMBIE_DROPPED;
    }
  }
}

bool Room::isEmpty() {
  return m_peerById.size() == 0;
}
