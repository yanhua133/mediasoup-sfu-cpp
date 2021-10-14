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

//#include <openssl/err.h>

namespace oatpp { namespace openssl {

////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
// ConnectionContext

Connection::ConnectionContext::ConnectionContext(Connection* connection, data::stream::StreamType streamType, Properties&& properties)
  : Context(std::forward<Properties>(properties))
  , m_connection(connection)
  , m_streamType(streamType)
{}

void Connection::ConnectionContext::init() {

  if(m_connection->m_initialized) {
    return;
  }

  m_connection->m_initialized = true;

  if (!SSL_is_init_finished(m_connection->m_ssl)) {

    int res;

    do {

      //ERR_clear_error();

      m_connection->m_readAction = async::Action::createActionByType(async::Action::TYPE_NONE);
      m_connection->m_writeAction = async::Action::createActionByType(async::Action::TYPE_NONE);

      res = SSL_do_handshake(m_connection->m_ssl);

      if(!m_connection->m_readAction.isNone() || !m_connection->m_writeAction.isNone()) {
        throw std::runtime_error("[oatpp::openssl::Connection::ConnectionContext::init()]: "
                                 "Error. The async::Action is unexpectedly returned in a Simple-API method.");
      }

      int err = SSL_get_error(m_connection->m_ssl, res);
      switch (err) {
        case SSL_ERROR_NONE:
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
          break;
        default:
          //std::string errStr = std::string(ERR_error_string(ERR_get_error(), nullptr));
          //OATPP_LOGE("[oatpp::openssl::Connection::ConnectionContext::init()]", "Error. Handshake failed. code=%d, msg='%s'", err, errStr.c_str());
          //throw std::runtime_error("[oatpp::openssl::Connection::ConnectionContext::init()]: Error. Handshake failed. " + errStr);
          return;
      }

    } while(res != 1);

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
      m_connection->m_initialized = true;
      if (SSL_is_init_finished(m_connection->m_ssl)) {
        return finish();
      }
      return yieldTo(&HandshakeCoroutine::doHandshake);
    }

    Action doHandshake() {

      if(!m_connection->m_readAction.isNone()) return std::move(m_connection->m_readAction);
      if(!m_connection->m_writeAction.isNone()) return std::move(m_connection->m_writeAction);

      auto res = SSL_do_handshake(m_connection->m_ssl);

      if(!m_connection->m_readAction.isNone()) return std::move(m_connection->m_readAction);
      if(!m_connection->m_writeAction.isNone()) return std::move(m_connection->m_writeAction);

      int err = SSL_get_error(m_connection->m_ssl, res);
      switch (err) {
        case SSL_ERROR_NONE:
        case SSL_ERROR_WANT_READ:
        case SSL_ERROR_WANT_WRITE:
          break;
        default:
          OATPP_LOGE("[oatpp::openssl::Connection::ConnectionContext::initAsync()]", "Error. Handshake failed. err=%d", err);
          throw std::runtime_error("[oatpp::openssl::Connection::ConnectionContext::initAsync()]: Error. Handshake failed.");
      }

      if(res == 1) {
        return finish();
      }

      return repeat();

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
// Connection

int Connection::createBio(BIO* bio) {
  BIO_set_init(bio, 1);
  return 1;
}

int Connection::destroyBio(BIO* bio) {
  return 1;
}

long Connection::BioCtrl(BIO* bio, int cmd, long larg, void *parg) {
  switch(cmd) {
    case BIO_CTRL_FLUSH: return 1;
  }
  return 0;
}

BIO_METHOD* Connection::createBioMethod() {
  BIO_METHOD* m = BIO_meth_new(BIO_get_new_index(), "oatpp-openssl::BIO_METHOD");
  BIO_meth_set_write(m, &Connection::BioWrite);
  BIO_meth_set_read(m, &Connection::BioRead);
  BIO_meth_set_create(m, &Connection::createBio);
  BIO_meth_set_destroy(m, &Connection::destroyBio);
  BIO_meth_set_ctrl(m, &Connection::BioCtrl);
  return m;
}

BIO_METHOD* Connection::getBioMethod() {
  static BIO_METHOD* m = createBioMethod();
  return m;
}

int Connection::BioWrite(BIO* bio, const char* data, int size) {

  auto _this = static_cast<Connection*>(BIO_get_data(bio));
  if(!_this->m_writeAction.isNone()) {
    BIO_set_retry_write(bio);
    return -1;
  }
  auto res = _this->m_stream->write(data, size, _this->m_writeAction);

  if(res > 0) {
    return res;
  }

  switch(res) {

    case IOError::RETRY_READ: {
      BIO_set_retry_read(bio);
      break;
    }
    case IOError::RETRY_WRITE: {
      BIO_set_retry_write(bio);
      break;
    }

    default:
      BIO_clear_retry_flags(bio);

  }

  return -1;
}

int Connection::BioRead(BIO* bio, char* data, int size) {

  auto _this = static_cast<Connection*>(BIO_get_data(bio));
  if(!_this->m_readAction.isNone()) {
    BIO_set_retry_read(bio);
    return -1;
  }
  auto res = _this->m_stream->read(data, size, _this->m_readAction);

  if(res > 0) {
    return res;
  }

  switch(res) {

    case IOError::RETRY_READ: {
      BIO_set_retry_read(bio);
      break;
    }
    case IOError::RETRY_WRITE: {
      BIO_set_retry_write(bio);
      break;
    }

    default:
      BIO_clear_retry_flags(bio);

  }

  return -1;

}

Connection::Connection(SSL* ssl, const std::shared_ptr<oatpp::data::stream::IOStream>& stream)
  : m_ssl(ssl)
  , m_rbio(BIO_new(getBioMethod()))
  , m_wbio(BIO_new(getBioMethod()))
  , m_stream(stream)
  , m_initialized(false)
{

  BIO_set_data(m_rbio, this);
  BIO_set_data(m_wbio, this);

  SSL_set_bio(m_ssl, m_rbio, m_wbio);

  auto& streamInContext = stream->getInputStreamContext();
  data::stream::Context::Properties inProperties(streamInContext.getProperties());
  inProperties.put("tls", "openssl");
  inProperties.getAll();

  m_inContext = new ConnectionContext(this, streamInContext.getStreamType(), std::move(inProperties));

  auto& streamOutContext = stream->getOutputStreamContext();
  if(streamInContext == streamOutContext) {
    m_outContext = m_inContext;
  } else {

    data::stream::Context::Properties outProperties(streamOutContext.getProperties());
    outProperties.put("tls", "openssl");
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
  SSL_free(m_ssl);
}

oatpp::v_io_size Connection::write(const void* buff, v_buff_size count, async::Action& action) {

  m_writeAction = async::Action::createActionByType(async::Action::TYPE_NONE);
  auto res = SSL_write(m_ssl, buff, count);
  if(!m_writeAction.isNone()) {
    action = std::move(m_writeAction);
  }
  if(res > 0) {
    return res;
  }

  int err = SSL_get_error(m_ssl, res);
  switch (err) {
    case SSL_ERROR_NONE:
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE: return IOError::RETRY_WRITE;
  }

  return IOError::BROKEN_PIPE;

}

oatpp::v_io_size Connection::read(void *buff, v_buff_size count, async::Action& action){

  m_readAction = async::Action::createActionByType(async::Action::TYPE_NONE);
  auto res = SSL_read(m_ssl, buff, count);
  if(!m_readAction.isNone()) {
    action = std::move(m_readAction);
  }

  if(res > 0) {
    return res;
  }

  int err = SSL_get_error(m_ssl, res);
  switch (err) {
    case SSL_ERROR_NONE:
    case SSL_ERROR_WANT_READ:
    case SSL_ERROR_WANT_WRITE: return IOError::RETRY_READ;
  }

  return IOError::BROKEN_PIPE;

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

std::shared_ptr<data::stream::IOStream> Connection::getTransportStream() {
  return m_stream;
}
  
}}
