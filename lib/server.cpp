// Copyright (c) Orange 2014
// Use of this source code is governed by a BSD-style licence
// that can be found in the LICENSE file.

#include <boost/bimap.hpp>
#include <boost/bimap/multiset_of.hpp>

#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <websocketpp/common/thread.hpp>

#include "logger.hpp"
#include "protocol.hpp"
#include "parser.hpp"
#include "util.hpp"
#include "server.hpp"

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

using websocketpp::lib::thread;
using websocketpp::lib::mutex;
using websocketpp::lib::unique_lock;
using websocketpp::lib::condition_variable;

using boost::bimap;

namespace WAMPP {

typedef websocketpp::server<websocketpp::config::asio> WSServer;

enum action_type {
    OPEN,
    CLOSE,
    MESSAGE
};

struct action {
    action(action_type t,
           connection_hdl h) : type(t), hdl(h) {}
    action(action_type t,
           connection_hdl h,
           WSServer::message_ptr m) : type(t), hdl(h), msg(m) {}

    action_type type;
    connection_hdl hdl;
    WSServer::message_ptr msg;
};

typedef std::set<connection_hdl,std::owner_less<connection_hdl>> ConnList;

/**
 * Subscriptions are stored in a complex container representing
 * a many to many relationship between connection handles
 * and subscription topics.
 * The container relies on the boost::bimap to create a bidirectional
 * link between two "left" and "right" std::multimap representing
 * the one-to-many associations between handles and strings repsectively.
 * A specific ordering function is provided that allows comparison between
 * connection/topic pairs.
 */

template< class Rel >
struct SubOrder
{
    bool operator()(Rel ra, Rel rb) const
    {
        bool result = std::owner_less<connection_hdl>()(ra.left,rb.left);
        if (!result && !std::owner_less<connection_hdl>()(rb.left,ra.left)) {
            // Connection handles are equal, compare topics
            result = (ra.right < rb.right);
        }
        return result;
    }
};

typedef boost::bimap< boost::bimaps::multiset_of<connection_hdl,std::owner_less<connection_hdl>>,
                      boost::bimaps::multiset_of<string>,
                      boost::bimaps::set_of_relation<SubOrder<boost::bimaps::_relation>>
                    > SubscriptionList;

class ServerImpl: public Server {
public:
    ServerImpl(const string& ident);
    ~ServerImpl();

    void run(uint16_t port);
    void addRPC(string uri, RemoteProc *rpc);
    void registerSubFilter(SubFilter *sub);
    void publish(string topic, JSON::NodePtr event); 

    bool on_validate(connection_hdl hdl);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, WSServer::message_ptr msg);

    void actions_loop();

private:
    WSServer m_server;

    shared_ptr<thread> m_actions_thread;
    
    ConnList m_sessions;
    std::queue<action> m_actions;
    std::map<string,RemoteProc*> m_rpcs; 
    SubscriptionList m_subscriptions;

    // Concurrent read/write access to the Server internal lists
    // is protected by a set of mutexes defined below.
    // The same pattern is used whenever a member requests access
    // to one of them.
    // 1. A lock is acquired at the block level by instantiating a
    // std::unique_lock:
    // {
    //     unique_lock<mutex> lock(m_action_lock);
    // (This call blocks until the mutex is available)
    // 2. The required operations are performed
    //     ...
    // 3. The mutex is unlocked when the lock goes out of scope
    // }
    mutex m_action_lock;
    mutex m_sessions_lock;
    mutex m_rpc_lock;
    mutex m_subs_lock;
    condition_variable m_action_cond;

    void send(connection_hdl hdl, Message* msg);
    
    const string m_ident;    
    
    SubFilter* m_subFilter;
};

Server* Server::create(const std::string& ident) {
    return new ServerImpl(ident);
}

ServerImpl::ServerImpl(const string& ident):
    m_ident(ident) {
    // Initialize Asio Transport
    m_server.init_asio();

    // Register handler callbacks
    m_server.set_validate_handler(bind(&ServerImpl::on_validate,this,::_1));
    m_server.set_open_handler(bind(&ServerImpl::on_open,this,::_1));
    m_server.set_close_handler(bind(&ServerImpl::on_close,this,::_1));
    m_server.set_message_handler(bind(&ServerImpl::on_message,this,::_1,::_2));

    // Start a thread to run the processing loop
    m_actions_thread.reset(new thread(bind(&ServerImpl::actions_loop,this)));
}

ServerImpl::~ServerImpl() {
    // Terminate the actions thread
    m_actions_thread->join();
}

// Run the asio loop (called from main thread)
void ServerImpl::run(uint16_t port) {
    // listen on specified port
    m_server.listen(port);

    // Start the server accept loop
	m_server.start_accept();

	// Start the ASIO io_service run loop
    try {
        m_server.run();
    } catch (const std::exception & e) {
        LOGGER_WRITE(Logger::DEBUG,e.what());
    } catch (websocketpp::lib::error_code e) {
        LOGGER_WRITE(Logger::DEBUG,e.message());
    } catch (...) {
        LOGGER_WRITE(Logger::DEBUG,"other exception");
    }
}

bool ServerImpl::on_validate(connection_hdl hdl) {

    bool result = false;

    WSServer::connection_ptr con = m_server.get_con_from_hdl(hdl);

    const std::vector<std::string> & subp_requests = con->get_requested_subprotocols();
    std::vector<std::string>::const_iterator it;

    LOGGER_WRITE(Logger::DEBUG,"Requested protocols:");
    for (it = subp_requests.begin(); it != subp_requests.end(); ++it) {
        std::string subprotocol = std::string(*it);
        LOGGER_WRITE(Logger::DEBUG,subprotocol);
        if (subprotocol == "wamp") {
            con->select_subprotocol("wamp");
            LOGGER_WRITE(Logger::DEBUG,"Selecting WAMP subprotocol");
            result = true;
        }
    }

    return result;
}

void ServerImpl::on_open(connection_hdl hdl) {
    unique_lock<mutex> lock(m_action_lock);
    LOGGER_WRITE(Logger::DEBUG,"on_open");
    m_actions.push(action(OPEN,hdl));
    lock.unlock();
    m_action_cond.notify_one(); 
}

void ServerImpl::on_close(connection_hdl hdl) {
    unique_lock<mutex> lock(m_action_lock);
    LOGGER_WRITE(Logger::DEBUG,"on_close");
    m_actions.push(action(CLOSE,hdl));
    lock.unlock();
    m_action_cond.notify_one();
}

void ServerImpl::on_message(connection_hdl hdl, WSServer::message_ptr msg) {
    // queue message up for sending by processing thread
    unique_lock<mutex> lock(m_action_lock);
    LOGGER_WRITE(Logger::DEBUG,"on_message");
    m_actions.push(action(MESSAGE,hdl,msg));
    lock.unlock();
    m_action_cond.notify_one();
}

void ServerImpl::actions_loop() {
    while(1) {
        unique_lock<mutex> lock(m_action_lock);

        while(m_actions.empty()) {
            m_action_cond.wait(lock);
        }

        action a = m_actions.front();
        m_actions.pop();

        lock.unlock();
        if (a.type == OPEN) {
            string sessionId = genRandomId(16);
            Welcome welcome(sessionId,WAMPP_PROTOCOL_VERSION,m_ident);
            unique_lock<mutex> sesslock(m_sessions_lock);
            m_sessions.insert(a.hdl);
            send(a.hdl,&welcome);
        } else if (a.type == CLOSE) {
            // Cancel pending subscriptions
            unique_lock<mutex> lock(m_subs_lock);
            std::pair<SubscriptionList::left_iterator,SubscriptionList::left_iterator> range = 
                m_subscriptions.left.equal_range(a.hdl);
            for (SubscriptionList::left_iterator it = range.first;it != range.second; it++) {
                if (m_subFilter) {
                    m_subFilter->unsubscribe(it->second);
                }
                m_subscriptions.left.erase(it);
            }
            unique_lock<mutex> sesslock(m_sessions_lock);
            m_sessions.erase(a.hdl);
        } else if (a.type == MESSAGE) {
            unique_lock<mutex> sesslock(m_sessions_lock);
            Message* wamp_msg = parseMessage(a.msg->get_payload());
            if (wamp_msg) {
                switch (wamp_msg->getType()) {
                    case WELCOME:
                    case CALLRESULT:
                    case CALLERROR:
                    case EVENT:
                    {
                        LOGGER_WRITE(Logger::ERROR,"Ignoring Server to Client message");
                        delete wamp_msg;
                        break;
                    }
                    case CALL:
                    {
                        string callID = ((Call *)wamp_msg)->callID();
                        string procURI = ((Call *)wamp_msg)->procURI();
                        unique_lock<mutex> rpclock(m_rpc_lock);
                        std::map<string,RemoteProc*>::iterator it = m_rpcs.find(procURI);
                        if(it != m_rpcs.end()) {
                            // Call RPC
                            JSON::NodePtr result;
                            if (it->second->invoke(callID,
                                                   ((Call *)wamp_msg)->args(),
                                                   result)) {
                                CallResult response(callID,result);
                                send(a.hdl,&response);
                            } else {
                                CallError error(callID,
                                                "wampp:call-error",
                                                "The remote procedure call failed (see error details)",
                                                result);
                                send(a.hdl,&error);
                            }
                        } else {
                            JSON::NodePtr errorDetails(new JSON::Node(procURI));
                            CallError error(callID,
                                            "wampp:no-such-method",
                                            "No such method",
                                            errorDetails);
                            send(a.hdl,&error);
                        }
                        break;
                    }
                    case SUBSCRIBE:
                    {
                        string topicURI = ((Subscribe *)wamp_msg)->topicURI();
                        if (!m_subFilter || m_subFilter->subscribe(topicURI)) {
                            unique_lock<mutex> subslock(m_subs_lock);
                            SubscriptionList::value_type subscription(a.hdl,topicURI);
                            if (m_subscriptions.find(subscription) == m_subscriptions.end()) {
                                // We do not allow multiple subscriptions from the same session
                                m_subscriptions.insert(SubscriptionList::value_type(a.hdl,topicURI));
                            }
                        }
                        break;
                    }
                    case UNSUBSCRIBE:
                    {
                        string topicURI = ((UnSubscribe *)wamp_msg)->topicURI();
                        if (m_subFilter) {
                            m_subFilter->unsubscribe(topicURI);
                        }
                        unique_lock<mutex> subslock(m_subs_lock);
                        m_subscriptions.erase(SubscriptionList::value_type(a.hdl,topicURI));
                    }
                    default:
                        break;
                }
                delete wamp_msg;
            }
        } else {
            // undefined.
        }
    }
}

void ServerImpl::send(connection_hdl hdl, Message* msg) {
    std::ostringstream oss;
    msg->serialize(oss);
    m_server.send(hdl,oss.str(),websocketpp::frame::opcode::text);
}

void ServerImpl::addRPC(string uri, RemoteProc* rpc) {
    unique_lock<mutex> lock(m_rpc_lock);
    m_rpcs.insert(std::make_pair(uri,rpc));
}

void ServerImpl::registerSubFilter(SubFilter *sub) {
    m_subFilter = sub;
}

void ServerImpl::publish(string topicURI, JSON::NodePtr evt) {
    unique_lock<mutex> lock(m_subs_lock);
    std::pair<SubscriptionList::right_iterator,SubscriptionList::right_iterator> range = m_subscriptions.right.equal_range(topicURI);
    for (SubscriptionList::right_iterator it = range.first;it != range.second; it++) {
        Event msg(topicURI,evt);
        /* This seems counter intuitive, but since it is an iterator from the right view,
         * connection is the second term here (topic, acting as key, is the first) 
         */
        send(it->second,&msg);
    }
}

} // namespace WAMPP
