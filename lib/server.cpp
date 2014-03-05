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

struct SessionSet {
    mutex m_lock;
    ConnList m_cnxs;
};

class ServerImpl: public Server {
public:
    ServerImpl(const string& ident);
    ~ServerImpl();

    void run(uint16_t port);
    void addRPC(string uri, RemoteProc *rpc);

    bool on_validate(connection_hdl hdl);
    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, WSServer::message_ptr msg);

    void actions_loop();

private:
    WSServer m_server;

    shared_ptr<thread> m_actions_thread;
    
    SessionSet m_sessions;
    std::queue<action> m_actions;
    std::map<string,RemoteProc*> m_rpcs; 
    std::map<string,SessionSet*> m_topics;

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
    mutex m_rpc_lock;
    mutex m_topics_lock;
    condition_variable m_action_cond;

    void send(connection_hdl hdl, Message* msg);
    
    const string m_ident;    
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
            unique_lock<mutex> sesslock(m_sessions.m_lock);
            m_sessions.m_cnxs.insert(a.hdl);
            send(a.hdl,&welcome);
        } else if (a.type == CLOSE) {
            unique_lock<mutex> sesslock(m_sessions.m_lock);
            m_sessions.m_cnxs.erase(a.hdl);
        } else if (a.type == MESSAGE) {
            unique_lock<mutex> sesslock(m_sessions.m_lock);
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
                        unique_lock<mutex> topicslock(m_topics_lock);
                        std::map<string,SessionSet*>::iterator it = m_topics.find(topicURI);
                        SessionSet* subscribers;
                        if (it == m_topics.end()) {
                            subscribers = new SessionSet();
                            m_topics.insert(std::make_pair(topicURI,subscribers));
                        } else {
                            subscribers = it->second;
                        }
                        unique_lock<mutex> subslock(subscribers->m_lock);
                        subscribers->m_cnxs.insert(a.hdl);
                        break;
                    }
                    case UNSUBSCRIBE:
                    {
                        string topicURI = ((UnSubscribe *)wamp_msg)->topicURI();
                        unique_lock<mutex> lock(m_topics_lock);
                        std::map<string,SessionSet*>::iterator it = m_topics.find(topicURI);
                        if (it != m_topics.end()) {
                            SessionSet* subscribers = it->second;
                            unique_lock<mutex> subslock(subscribers->m_lock);
                            subscribers->m_cnxs.erase(a.hdl);
                            if (subscribers->m_cnxs.empty()) {
                                m_topics.erase(it);
                                delete(subscribers);
                            }
                        }
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

} // namespace WAMPP
