#ifndef PEER_H
#define PEER_H

#include "utils/json.hpp"
#include "Transport/WebSocketTransport.h"
#include <stdexcept>
#include "utils/PeerTimer.h"
#include "utils/EventEmitter.hpp"

using namespace std;
using namespace nlohmann;

struct PROTOO_MSG {
    json message;
    function<void(json)> accept;
    function<void(int, string)> reject;//error code and error desc
    function<void(std::string)> Event;//event with json message as param
};

struct SENT_MSG {
    int id;
    string method;
    function<void(json)> resolve;
    function<void(string)> reject;
    function<void()> close;
};

namespace protoo {
    class PeerError : public std::runtime_error
    {
    public:
        explicit PeerError(const char* description);
    };

    /* Inline methods. */

    inline PeerError::PeerError(const char* description)
            : std::runtime_error(description)
    {
    }

    class Peer : public EventEmitter {
    public:
        Peer(string peerId, WebSocketTransport* pTransport);
        ~Peer();

    public:
        string id();
        bool closed();
        json data();
        void setData(json data);
        void close();
        //void requestTest(string method, json data);
        std::future<json> request(string method, json data = json({}));
        void notify(string method, json data = json({}));

    private:
        void _handleTransport();
        void handleRequest(json request);
        void handleResponse(json response);
        void handleNotification(json notification);

    private:
        bool m_closed = false;
        string m_peerId;
        WebSocketTransport* m_pTransport;
        json m_data = json({});
        map<int, shared_ptr<SENT_MSG>> m_sents;
        PeerTimer mTimer;
        unique_ptr<std::promise<nlohmann::json>> m_pPromise;
        // Queue to maintain the receive tasks when there are no messages(yet).

        //function<void(json)> accept;
        //function<void(int errorCode, std::string errorReason)> reject;
    };

}
#endif

