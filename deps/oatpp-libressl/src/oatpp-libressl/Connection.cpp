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

#include "Connection.hpp"
#include <openssl/err.h>

namespace oatpp { namespace libressl {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ConnectionContext

Connection::ConnectionContext::ConnectionContext(Connection* connection, data::stream::StreamType streamType, Properties&& properties)
  : Context(std::forward<Properties>(properties))
  , m_connection(connection)
  , m_streamType(streamType)
{}

void Connection::ConnectionContext::init() {

  if(!m_connection->m_initialized) {

    m_connection->m_initialized = true;

    auto tlsObject = m_connection->m_tlsObject;

    if (tlsObject->getType() == TLSObject::Type::SERVER) {

      auto res = tls_accept_cbs(tlsObject->getTLSHandle(), &m_connection->m_tlsHandle, readCallback, writeCallback, m_connection);

      if (res != 0) {
        OATPP_LOGD("[oatpp::libressl::Connection::ConnectionContext::init()]", "Error on call to 'tls_accept_cbs'. res=%d", res);
      }

    } else if (tlsObject->getType() == TLSObject::Type::CLIENT) {

      m_connection->m_tlsHandle = tlsObject->getTLSHandle();
      const char* host = nullptr;
      if(tlsObject->getServerName()) {
        host = (const char*) tlsObject->getServerName()->getData();
      }
      auto res = tls_connect_cbs(m_connection->m_tlsHandle,
                                 readCallback, writeCallback,
                                 m_connection, host);

      tlsObject->annul();

      if (res != 0) {
        OATPP_LOGD("[oatpp::libressl::Connection::ConnectionContext::init()]", "Error on call to 'tls_connect_cbs'. %s", tls_error(m_connection->m_tlsHandle));
      }

    } else {
      throw std::runtime_error("[oatpp::libressl::Connection::ConnectionContext::init()]: Error. Unknown TLSObject type.");
    }

  }

}

async::CoroutineStarter Connection::ConnectionContext::initAsync() {

  class HandshakeCoroutine : public oatpp::async::Coroutine<HandshakeCoroutine> {
  private:
    Connection* m_connection;
  public:

    HandshakeCoroutine(Connection* connection)
      : m_connection(connection)
    {}

    Action act() override {

      if(m_connection->m_initialized) {
        return finish();
      }

      auto tlsObject = m_connection->m_tlsObject;

      if (tlsObject->getType() == TLSObject::Type::SERVER) {
        return yieldTo(&HandshakeCoroutine::initServer);
      } else if (tlsObject->getType() == TLSObject::Type::CLIENT) {
        return yieldTo(&HandshakeCoroutine::initClient);
      }

      throw std::runtime_error("[oatpp::libressl::Connection::ConnectionContext::init()]: Error. Unknown TLSObject type.");

    }

    Action initServer() {

      auto tlsObject = m_connection->m_tlsObject;
      auto res = tls_accept_cbs(tlsObject->getTLSHandle(), &m_connection->m_tlsHandle, readCallback, writeCallback, m_connection);

      if (res != 0) {
        return error<Error>("[oatpp::libressl::Connection::ConnectionContext::initAsync(){initServer()}]: Error. Handshake failed.");
      }

      /* Handshake successful */
      m_connection->m_initialized = true;
      return finish();

    }

    Action initClient() {

      auto tlsObject = m_connection->m_tlsObject;
      m_connection->m_tlsHandle = tlsObject->getTLSHandle();
      const char* host = nullptr;
      if(tlsObject->getServerName()) {
        host = (const char*) tlsObject->getServerName()->getData();
      }
      auto res = tls_connect_cbs(m_connection->m_tlsHandle,
                                 readCallback, writeCallback,
                                 m_connection, host);

      tlsObject->annul();

      if (res != 0) {
        return error<Error>("[oatpp::libressl::Connection::ConnectionContext::initAsync(){initClient()}]: Error. Handshake failed.");
      }

      m_connection->m_initialized = true;
      return finish();

    }

  };

  if(m_connection->m_initialized) {
    return nullptr;
  }

  return HandshakeCoroutine::start(m_connection);

}

bool Connection::ConnectionContext::isInitialized() const {
  return m_connection->m_initialized;
}

data::stream::StreamType Connection::ConnectionContext::getStreamType() const {
  return m_streamType;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// IOLockGuard

Connection::IOLockGuard::IOLockGuard(Connection* connection, async::Action* checkAction)
  : m_connection(connection)
  , m_checkAction(checkAction)
{
  m_connection->packIOAction(m_checkAction);
  m_locked = true;
}

Connection::IOLockGuard::~IOLockGuard() {
  if(m_locked) {
    m_connection->m_ioLock.unlock();
  }
}

bool Connection::IOLockGuard::unpackAndCheck() {
  async::Action* check = m_connection->unpackIOAction();
  m_locked = false;
  return check == m_checkAction;
}

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// Connection

ssize_t Connection::writeCallback(struct tls *_ctx, const void *_buf, size_t _buflen, void *_cb_arg) {

  auto connection = static_cast<Connection*>(_cb_arg);
  async::Action* ioAction = connection->unpackIOAction();

  v_io_size res;
  if(ioAction && ioAction->isNone()) {
    res = connection->m_stream->write(_buf, _buflen, *ioAction);
    if(res == IOError::RETRY_READ || res == IOError::RETRY_WRITE) {
      res = TLS_WANT_POLLOUT;
    }
  } else {
    res = TLS_WANT_POLLOUT;
  }

  connection->packIOAction(ioAction);

  return (ssize_t)res;

}

ssize_t Connection::readCallback(struct tls *_ctx, void *_buf, size_t _buflen, void *_cb_arg) {

  auto connection = static_cast<Connection*>(_cb_arg);
  async::Action* ioAction = connection->unpackIOAction();

  v_io_size res;
  if(ioAction && ioAction->isNone()) {
    res = connection->m_stream->read(_buf, _buflen, *ioAction);
    if(res == IOError::RETRY_READ || res == IOError::RETRY_WRITE) {
      res = TLS_WANT_POLLOUT;
    }
  } else {
    res = TLS_WANT_POLLOUT;
  }

  connection->packIOAction(ioAction);

  return (ssize_t)res;

}

Connection::Connection(const std::shared_ptr<TLSObject>& tlsObject,
                       const std::shared_ptr<oatpp::data::stream::IOStream>& stream)
  : m_tlsHandle(nullptr)
  , m_tlsObject(tlsObject)
  , m_stream(stream)
  , m_initialized(false)
  , m_ioAction(nullptr)
{

  auto& streamInContext = stream->getInputStreamContext();
  data::stream::Context::Properties inProperties(streamInContext.getProperties());
  inProperties.put("tls", "libressl");
  inProperties.getAll();

  m_inContext = new ConnectionContext(this, streamInContext.getStreamType(), std::move(inProperties));

  auto& streamOutContext = stream->getOutputStreamContext();
  if(streamInContext == streamOutContext) {
    m_outContext = m_inContext;
  } else {

    data::stream::Context::Properties outProperties(streamOutContext.getProperties());
    outProperties.put("tls", "libressl");
    outProperties.getAll();

    m_outContext = new ConnectionContext(this, streamOutContext.getStreamType(), std::move(outProperties));

  }

}

Connection::~Connection(){
  if(m_inContext == m_outContext) {
    delete m_inContext;
  } else {
    delete m_inContext;
    delete m_outContext;
  }
  closeTLS();
  if(m_tlsHandle != nullptr) {
    tls_free(m_tlsHandle);
  }
}

void Connection::packIOAction(async::Action* action) {
  m_ioLock.lock();
  m_ioAction = action;
}

async::Action* Connection::unpackIOAction() {
  auto result = m_ioAction;
  m_ioAction = nullptr;
  m_ioLock.unlock();
  return result;
}

oatpp::v_io_size Connection::write(const void *buff, v_buff_size count, async::Action& action){

  IOLockGuard ioGuard(this, &action);

  auto result = tls_write(m_tlsHandle, buff, count);

  if(!ioGuard.unpackAndCheck()) {
    OATPP_LOGE("[oatpp::libressl::Connection::write(...)]", "Error. Packed action check failed!!!");
    return oatpp::IOError::BROKEN_PIPE;
  }

  if(result < 0) {
    switch (result) {
      case TLS_WANT_POLLIN: return oatpp::IOError::RETRY_WRITE;
      case TLS_WANT_POLLOUT: return oatpp::IOError::RETRY_WRITE;
      default:
        return oatpp::IOError::BROKEN_PIPE;
    }
  }

  return result;

}

oatpp::v_io_size Connection::read(void *buff, v_buff_size count, async::Action& action){

  IOLockGuard ioGuard(this, &action);

  auto result = tls_read(m_tlsHandle, buff, count);

  if(!ioGuard.unpackAndCheck()) {
    OATPP_LOGE("[oatpp::libressl::Connection::read(...)]", "Error. Packed action check failed!!!");
    return oatpp::IOError::BROKEN_PIPE;
  }

  if(result < 0) {
    switch (result) {
      case TLS_WANT_POLLIN: return oatpp::IOError::RETRY_WRITE;
      case TLS_WANT_POLLOUT: return oatpp::IOError::RETRY_WRITE;
      default:
        return oatpp::IOError::BROKEN_PIPE;
    }
  }

  return result;

}

void Connection::setOutputStreamIOMode(oatpp::data::stream::IOMode ioMode) {
  m_stream->setOutputStreamIOMode(ioMode);
}

oatpp::data::stream::IOMode Connection::getOutputStreamIOMode() {
  return m_stream->getOutputStreamIOMode();
}

oatpp::data::stream::Context& Connection::getOutputStreamContext() {
  return *m_outContext;
}

void Connection::setInputStreamIOMode(oatpp::data::stream::IOMode ioMode) {
  m_stream->setInputStreamIOMode(ioMode);
}

oatpp::data::stream::IOMode Connection::getInputStreamIOMode() {
  return m_stream->getInputStreamIOMode();
}

oatpp::data::stream::Context& Connection::getInputStreamContext() {
  return *m_inContext;
}

void Connection::closeTLS(){
  if(m_tlsHandle != nullptr) {
    tls_close(m_tlsHandle);
  }
}

std::shared_ptr<data::stream::IOStream> Connection::getTransportStream() {
  return m_stream;
}
  
}}
