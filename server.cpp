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

Server::Server(const string& ident):
    m_ident(ident) {
    // Initialize Asio Transport
    m_server.init_asio();

    // Register handler callbacks
    m_server.set_open_handler(bind(&Server::on_open,this,::_1));
    m_server.set_close_handler(bind(&Server::on_close,this,::_1));
    m_server.set_message_handler(bind(&Server::on_message,this,::_1,::_2));

    // Start a thread to run the processing loop
    m_actions_thread.reset(new thread(bind(&Server::actions_loop,this)));
}

Server::~Server() {
    // Terminate the actions thread
    m_actions_thread->join();
}

// Run the asio loop (called from main thread)
void Server::run(uint16_t port) {
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

void Server::on_open(connection_hdl hdl) {
    unique_lock<mutex> lock(m_action_lock);
    LOGGER_WRITE(Logger::DEBUG,"on_open");
    m_actions.push(action(OPEN,hdl));
    lock.unlock();
    m_action_cond.notify_one(); 
}

void Server::on_close(connection_hdl hdl) {
    unique_lock<mutex> lock(m_action_lock);
    LOGGER_WRITE(Logger::DEBUG,"on_close");
    m_actions.push(action(CLOSE,hdl));
    lock.unlock();
    m_action_cond.notify_one();
}

void Server::on_message(connection_hdl hdl, WSServer::message_ptr msg) {
    // queue message up for sending by processing thread
    unique_lock<mutex> lock(m_action_lock);
    LOGGER_WRITE(Logger::DEBUG,"on_message");
    m_actions.push(action(MESSAGE,hdl,msg));
    lock.unlock();
    m_action_cond.notify_one();
}

void Server::actions_loop() {
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
            unique_lock<mutex> lock(m_session_lock);
            m_sessions.insert(std::make_pair(sessionId,a.hdl));
            send(a.hdl,&welcome);
        } else if (a.type == CLOSE) {
            unique_lock<mutex> lock(m_session_lock);
            m_sessions.erase(std::make_pair("",a.hdl));
        } else if (a.type == MESSAGE) {
            unique_lock<mutex> lock(m_session_lock);
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
                        unique_lock<mutex> lock(m_rpc_lock);
                        std::map<string,RemoteProc>::iterator it = m_rpcs.find(procURI);
                        if(it != m_rpcs.end()) {
                            // Call RPC
                            JSON::NodePtr result;
                            if (it->second(a.hdl,
                                           callID,
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

void Server::send(connection_hdl hdl, Message* msg) {
    std::ostringstream oss;
    msg->serialize(oss);
    m_server.send(hdl,oss.str(),websocketpp::frame::opcode::text);
}

void Server::addRPC(string uri, RemoteProc rpc) {
    unique_lock<mutex> lock(m_rpc_lock);
    m_rpcs.insert(std::make_pair(uri,rpc));
}

} // namespace WAMPP
