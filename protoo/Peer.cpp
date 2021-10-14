#include "Peer.h"
#include "Message.h"

using namespace std;
using namespace nlohmann;

namespace protoo {

    Peer::Peer(string peerId, WebSocketTransport* pTransport) {
        m_closed = false;
        //自己new一个transport
        m_peerId = peerId;
        m_pTransport = pTransport;
        this->_handleTransport();

    }
    Peer::~Peer() {

    }

    string Peer::id()
    {
        return m_peerId;
    }
    bool Peer::closed()
    {
        return m_closed;
    }

    void Peer::close()
    {
        if(this->m_closed){
            return;
        }
        m_pTransport->close();
        for(auto &sent : this->m_sents)
        {
            sent.second->close();
        }
        this->emit("close");
    }

    json Peer::data() {
        return m_data;
    }

    void Peer::setData(json datax) {
        //TODO
    }

    //void Peer::requestTest(string method, json data) {
    //    auto request = Message::createRequest(method, data);
    //    std::cout << "[Peer] send a new request id=:" << request["id"].get<int>() << " method=" << method << endl;
    //    //会进入到uWS的线程发送
    //    std::promise<json> promise;
    //    this->m_pTransport->send(request);//just like await
    //}

    std::future<json> Peer::request(string method, json data) {
        auto request = Message::createRequest(method, data);
        //会进入到uWS的线程发送
        
        std::promise<json>* pPromiseJson = new std::promise<json>();
        this->m_pPromise.reset(pPromiseJson);
        //std::promise<json> promise;
        this->m_pTransport->send(request);//just like await
        //pplx::task_completion_event<json> tce;
        //m_request_task_queue.push(tce);
        //Timer timer = Timer();
        auto timeout = 1500 * (15 + (0.1 * this->m_sents.size()));

        std::shared_ptr<SENT_MSG> sent(new SENT_MSG);
        sent->id = request["id"].get<int>();
        sent->method = request["method"].get<std::string>();
        //capture all reference and capture value of request
        sent->resolve = [&, request](json data2) {

            auto sent_element = this->m_sents.find(request["id"].get<int>());
            if (sent_element == m_sents.end()) {
                return;
            }
            this->m_sents.erase(sent_element);
            this->mTimer.stop();
            this->m_pPromise->set_value(data2);
        };

        sent->reject = [&, request](string errorInfo) {
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
            auto sent_element = this->m_sents.find(request["id"].get<int>());
            if (sent_element == m_sents.end()) {
                return;
            }
            this->m_sents.erase(sent_element);
            this->m_pPromise->set_exception(std::make_exception_ptr(PeerError("peer request Time out error")));
        }, timeout);

        this->m_sents[request["id"].get<int>()] = sent;

        if (this->m_pPromise == nullptr) { //add by jacky 20211012
            std::cout << "[Peer] request promise is null ! "  << endl;
        }
        return this->m_pPromise->get_future();
    }

    void Peer::notify(string method, json data) {
        auto notify = Message::createNotification(method, data);
        this->m_pTransport->send(notify);
    }

    void Peer::_handleTransport()
    {
        if (this->m_pTransport->closed())
        {
            this->m_closed = true;
            this->emit("close");
            //setImmediate(() => this.safeEmit('close'));
            return;
        }

        this->m_pTransport->on("close", [&](){
            if (this->m_closed)
                return;
            this->m_closed = true;
           
            this->emit("close",this->id());
        });

        this->m_pTransport->on("message", [&](json message){
            if (message["request"].is_boolean())
                this->handleRequest(message);
            else if (message["response"].is_boolean())
                this->handleResponse(message);
            else if (message["notification"].is_boolean())
                this->handleNotification(message);
        });
    }


    void Peer::handleRequest(json request) {
        //std::shared_ptr<PROTOO_MSG> pmsg(new PROTOO_MSG);
        //TODO better way to emit
        std::function<void(json data)> accept([&, request](json data) {
            std::cout << "[Peer] handleRequest accept" << request.dump(4) << endl;
            auto response = Message::createSuccessResponse(request, data);
            this->m_pTransport->send(response);
        });
        std::function<void(int errorCode, std::string errorReason)> reject([&, request](int errorCode, std::string errorReason) {
            std::cout << "[Peer] handleRequest accept" << request.dump(4) << endl;
            auto response = Message::createErrorResponse(request, errorCode, errorReason);
            this->m_pTransport->send(response);
        });
//        auto accept =  [&, request](json data) {
//            std::cout << "[Peer] handleRequest accept" << request.dump(4) << endl;
//            auto response = Message::createSuccessResponse(request, data);
//            this->m_pTransport->send(response);
//        };
//        auto reject = [&, request](int errorCode, std::string errorReason) {
//            auto response = Message::createErrorResponse(request, errorCode, errorReason);
//            this->m_pTransport->send(response);
//
//        };
        this->emit("request", request,
                //accept() function
                   accept,
                //reject() function
                   reject
        );
        //message
//        pmsg->message = request;
//        //accept
//        pmsg->accept = [&, request](json data) {
//            std::cout << "[Peer] handleRequest accept" << request.dump(4) << endl;
//            auto response = Message::createSuccessResponse(request, data);
//            this->m_pTransport->send(response);
//        };
//        //reject
//        pmsg->reject = [&, request](int errorCode, std::string errorReason) {
//            auto response = Message::createErrorResponse(request, errorCode, errorReason);
//            this->m_pTransport->send(response);
//
//        };
//        this->emit("request", pmsg);
    }

    void Peer::handleResponse(json response) {
        std::shared_ptr<PROTOO_MSG> pmsg(new PROTOO_MSG);
        pmsg->message = response;

        auto sent_element = this->m_sents.find(response["id"].get<int>());
        if (sent_element == m_sents.end()) {
            return;
        }
        auto sent = sent_element->second;

        if (response.contains("ok"))
        {
            if (response["ok"].get<bool>())
                sent->resolve(response["data"]);
        }
        else
        {
            auto error = "error response!";
            sent->reject(error);
        }
    }

    void Peer::handleNotification(json notification) {
        //std::shared_ptr<PROTOO_MSG> pmsg(new PROTOO_MSG);
        //pmsg->message = notification;
        this->emit("notification", notification);
    }
}
