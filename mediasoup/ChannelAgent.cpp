#include "ChannelAgent.hpp"
#include "Log.hpp"
#include "Promise.hpp"
#include "RtpParameters.hpp"
#include "SctpParameters.hpp"
#include "json.hpp"
using json = nlohmann::json;
#define PROTOO_LOG_ENABLE 0
extern "C"
{
#include "netstring.h"
}

namespace mediasoup
{

	// netstring length for a 4194304 bytes payload.
	const static size_t NS_MESSAGE_MAX_LEN = 4194313;
	const static size_t NS_PAYLOAD_MAX_LEN = 4194304;

	ChannelAgent::ChannelAgent(ChannelObserver* obs) {
		MS_lOGF();
		RegisterObserver(obs);
		m_producerPipe.reset(new UVPipeWrapper(this, NS_MESSAGE_MAX_LEN, UVPipeWrapper::Role::PRODUCER));
		m_consumerPipe.reset(new UVPipeWrapper(this, NS_MESSAGE_MAX_LEN, UVPipeWrapper::Role::CONSUMER));

	}

	ChannelAgent::~ChannelAgent() {
		MS_lOGF();
		this->_closed = true;

		// Close every pending sent.
		for (auto sent : this->m_sents)
		{
			sent.second->close();
		}
		m_producerPipe->Close();//, false, "m_producerPipe close failed");
		m_consumerPipe->Close();//, false, "m_consumerPipe close failed");

	}

	bool ChannelAgent::Init(int producerFd, int consumerFd) {
		MS_lOGF();
		MS_lOGE("ChannelAgent::Init producerFd=%d consumerFd=%d ", producerFd, consumerFd);
		MS_ASSERT_RV_LOGE(m_producerPipe->Init(producerFd), false, "m_producerPipe init failed");
		MS_ASSERT_RV_LOGE(m_consumerPipe->Init(consumerFd), false, "m_consumerPipe init failed");

		return true;
	}
	bool ChannelAgent::Start() {
		MS_lOGF();
		MS_ASSERT_RV_LOGE(m_consumerPipe->Start(), false, "m_consumerPipe start failed");
		return true;
	}
	void ChannelAgent::Write(const uint8_t* data, size_t len)
	{
		m_producerPipe->Write(data, len);
	}

	void ChannelAgent::SetPid(int pid) {
		m_pid = pid;
	}
	void ChannelAgent::removeAllListeners(std::string producerId)
	{

	}
	json ChannelAgent::request(const char* method, json& internal)
	{
		return request(method, internal, json::object());
	}
	json ChannelAgent::request(const char* method, json& internal, const json& data)
	{
		std::lock_guard<std::mutex> lck(m_mutex);

		auto id = this->_nextId < 4294967295 ? ++this->_nextId : (this->_nextId = 1);

		MS_lOGD("request() [method:%s, id:%ld]", method, id);

		if (this->_closed)
			MS_lOGE("Channel closed");

		json request = {
			{ "id" , id },
			{ "method" , std::string(method) },
			{ "internal" , internal },
			{ "data" , data }
		};
		std::string strreq = request.dump();
		size_t readLen = strreq.length();
		char* jsonStart = nullptr;
		size_t jsonLen = netstring_encode_new(&jsonStart, (char*)strreq.c_str(), (size_t)strreq.length());
		//const ns = netstring.nsWrite(JSON.stringify(request));

		if (jsonLen > NS_MESSAGE_MAX_LEN)
			MS_lOGE("Channel request too big");

		// This may throw if closed or remote side ended.
		//this._producerSocket.write(ns);


	//发送数据处理异步处理
		this->m_pPromise.reset(new std::promise<json>);
		//std::promise<json> promise;
	//	this->m_pTransport->send(request);//just like await
		//pplx::task_completion_event<json> tce;
		//m_request_task_queue.push(tce);
		//Timer timer = Timer();
		auto timeout = 1000 * (15 + (0.1 * this->m_sents.size()));

		std::shared_ptr<Sent> sent(new Sent);
		sent->id = request["id"].get<int>();
		sent->method = request["method"].get<std::string>();
		//capture all reference and capture value of request
		sent->resolve = [&, request](json data2) {
#if PROTOO_LOG_ENABLE
			std::cout << "[Peer] request resolved id=:" << request["id"].get<int>() << " data2=" << data2 << std::endl;
#endif

			auto sent_element = this->m_sents.find(request["id"].get<int>());
			if (sent_element == m_sents.end()) {
#if PROTOO_LOG_ENABLE
				std::cout << "[Peer] request id not found in map!\n" << std::endl;
#endif

				return;
			}
			this->m_sents.erase(sent_element);
			this->mTimer.stop();
			this->m_pPromise->set_value(data2);
		};

		sent->reject = [&, request](std::string errorInfo) {
#if PROTOO_LOG_ENABLE
			std::cout << "[Peer] request reject id=:" << request["id"].get<int>() << " errorInfo=" << errorInfo << std::endl;
#endif

			auto sent_element = this->m_sents.find(request["id"].get<int>());
			if (sent_element == m_sents.end()) {
				std::cout << "[Peer] reject request id not found in map!\n" << std::endl;
				return;
			}
			this->m_sents.erase(sent_element);
			this->mTimer.stop();
			this->m_pPromise->set_exception(std::make_exception_ptr(PeerError(errorInfo.c_str())));
		};

		sent->close = [&]() {
			this->mTimer.stop();
		};
		//超时处理
		mTimer.setTimeout([&, request]() {
#if PROTOO_LOG_ENABLE
			std::cout << "[Peer] request request timeout request id=" << request["id"].get<int>() << std::endl;
#endif

			auto sent_element = this->m_sents.find(request["id"].get<int>());
			if (sent_element == m_sents.end()) {
#if PROTOO_LOG_ENABLE
				std::cout << "[Peer] request id not found in map!\n" << std::endl;
#endif

				return;
			}
			this->m_sents.erase(sent_element);
			this->m_pPromise->set_exception(std::make_exception_ptr(PeerError("peer request Time out error")));
			}, timeout);

#if PROTOO_LOG_ENABLE
		std::cout << "[Peer] insert into m_sents request id=" << request["id"].get<int>() << std::endl;
#endif // PROTOO_LOG_ENABLE

		this->m_sents[request["id"].get<int>()] = sent;
#if PROTOO_LOG_ENABLE
		std::cout << "[Peer] after insert m_sents.size =" << m_sents.size() << std::endl;
#endif
		//需要放到最后发送，不然会出现收到返回信息后还没有加入到队列的问题
		MS_lOGD("request raw data = %s", strreq.c_str());
		Write((const uint8_t*)jsonStart, jsonLen);
		return this->m_pPromise->get_future().get();
	}

	UVPipeWrapper* ChannelAgent::GetProducer() {
		return m_producerPipe.get();
	}

	UVPipeWrapper* ChannelAgent::GetConsumer() {
		return m_consumerPipe.get();
	}


	void ChannelAgent::OnRead(UVPipeWrapper* pipe, uint8_t* data, size_t len) {
		if (data != 0) {
			data[len] = 0;
		}
		MS_lOGD("ChannelAgent::OnRead: rawdata = %s", (char*)data);
		MS_lOGD("ChannelAgent::OnRead:m_readBuf left =%s", m_readBuf.c_str());
		m_readBuf.append((char*)data, len);
		MS_lOGD("ChannelAgent::OnRead:m_readBuf append =%s", m_readBuf.c_str());

		// once
		if (!m_once && m_readBuf.find("running") != std::string::npos) {
			m_once = true;
			NotifyObserver(std::bind(&ChannelObserver::OnRunning, std::placeholders::_1));
			m_readBuf.clear();
			return;
		}

		if (m_readBuf.length() > NS_PAYLOAD_MAX_LEN) {
			MS_lOGE("receiving buffer is full, discarding all data into it");
			// Reset the buffer and exit.
			m_readBuf.clear();
			return;
		}

		// Be ready to parse more than a single message in a single TCP chunk.
		while (true) {
			size_t readLen = m_readBuf.length();
			char* jsonStart = nullptr;
			size_t jsonLen;
			int nsRet = netstring_read(const_cast<char*>(m_readBuf.c_str()), readLen, &jsonStart, &jsonLen);

			if (nsRet != 0) {
				switch (nsRet) {
				case NETSTRING_ERROR_TOO_SHORT:
				{
					// message incomplete is not full, just wait.
					MS_lOGI("NETSTRING_ERROR_TOO_SHORT");
					MS_lOGE("netstring_read error raw = %s", m_readBuf.c_str());
					return;
				}
				case NETSTRING_ERROR_TOO_LONG:
				{
					MS_lOGE("NETSTRING_ERROR_TOO_LONG");
					break;
				}
				case NETSTRING_ERROR_NO_COLON:
				{
					MS_lOGE("NETSTRING_ERROR_NO_COLON");
					break;
				}
				case NETSTRING_ERROR_NO_COMMA:
				{
					MS_lOGE("NETSTRING_ERROR_NO_COMMA");
					break;
				}
				case NETSTRING_ERROR_LEADING_ZERO:
				{
					MS_lOGE("NETSTRING_ERROR_LEADING_ZERO");
					break;
				}
				case NETSTRING_ERROR_NO_LENGTH:
				{
					MS_lOGE("NETSTRING_ERROR_NO_LENGTH");
					break;
				}

				}
				MS_lOGE("netstring_read error raw = %s", m_readBuf.c_str());

				// Error, so reset and exit the parsing loop.
				m_readBuf.clear();
				return;
			}

			try {
				nlohmann::json jsonMessage = nlohmann::json::parse(jsonStart, jsonStart + jsonLen);
				processMessage(jsonMessage);

				if (jsonLen == m_readBuf.length()) {
					m_readBuf.clear();
					return;
				}
				else {
					int nslen = netstring_buffer_size(jsonLen);
					m_readBuf = m_readBuf.substr(nslen, m_readBuf.length() - nslen);
					if (m_readBuf.length() == 0) {
						return;
					}
					else {
						continue;
					}

				}

			}
			catch (const nlohmann::json::parse_error& error) {
				MS_lOGE("JSON parsing error: %s", error.what());
				m_readBuf.clear();
				return;
			}

			return;
		}
	}

	void ChannelAgent::processMessage(const nlohmann::json& msg) {
        if(msg.contains("event") && msg["event"].get<std::string>() != "score")
            MS_lOGD("ChannelAgent::processMessage: msg=%s", msg.dump().c_str());
		if (msg.contains("id"))
		{
			auto sent_element = this->m_sents.find(msg["id"].get<int>());
			if (sent_element == m_sents.end()) {

				MS_lOGE(
					"received response does not match any sent request [id:%s] m_sents.size=%lu", msg["id"].dump().c_str(), m_sents.size());

				return;
			}
			auto sent = sent_element->second;

			if (msg.contains("accepted"))
			{
				MS_lOGD(
					"request succeeded [method:%s, id:%d]", sent->method.c_str(), sent->id);
				if (msg.contains("data")) {
					sent->resolve(msg["data"]);
				}
				else {
					json data = json::object();
					sent->resolve(data);
				}

			}
			else if (msg.contains("error"))
			{
				MS_lOGW(
					"request failed [method:%s, id:%d]: %s",
					sent->method.c_str(), sent->id, msg["reason"].dump().c_str());

				if (true)
				{
					if (msg["error"] == "TypeError")
					{
						sent->reject(msg["reason"]);//new TypeError(msg.reason));
						//break;
					}
					else {
						sent->reject(msg["reason"]);//new Error(msg.reason));
					}

				}
			}
			else
			{
				MS_lOGE(
					"received response is not accepted nor rejected [method:%s, id:%d]",
					sent->method.c_str(), sent->id);
			}

		}
		// If a notification emit it to the corresponding entity.
		else if (msg.contains("targetId") && msg.contains("event"))
		{
			if (msg.contains("data"))
				//this->emit(msg["targetId"].dump().c_str(), msg["event"].dump().c_str(), msg["data"]);
                this->emit(msg["targetId"].get<std::string>(), msg["event"].get<std::string>(), msg["data"]);
            else{
				//this->emit(msg["targetId"].dump().c_str(), msg["event"].dump().c_str(), json({}));
                this->emit(msg["targetId"].get<std::string>(), msg["event"].get<std::string>(), json({}));
            }
		}
		// Otherwise unexpected message.
		else
		{
			MS_lOGD(
				"received message is not a response nor a notification");
		}
	}

	void ChannelAgent::OnClose(UVPipeWrapper* pipe) {
		if (pipe == m_consumerPipe.get()) {
			MS_lOGI("m_consumerPipe ChannelAgent OnClose");
		}

		if (pipe == m_producerPipe.get()) {
			MS_lOGI("m_producerPipe ChannelAgent OnClose");
		}

	}

}
