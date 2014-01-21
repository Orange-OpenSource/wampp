#include "logger.hpp"
#include "protocol.hpp"
#include "util.hpp"
#include "server.hpp"

using websocketpp::connection_hdl;
using websocketpp::lib::placeholders::_1;
using websocketpp::lib::placeholders::_2;
using websocketpp::lib::bind;

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
}

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
    m_actions.push(action(MESSAGE,msg));
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
            Welcome welcome(sessionId,m_ident);
            unique_lock<mutex> lock(m_session_lock);
            m_sessions.insert(std::make_pair(sessionId,a.hdl));
            send(a.hdl,&welcome);
        } else if (a.type == CLOSE) {
            unique_lock<mutex> lock(m_session_lock);
            m_sessions.erase(std::make_pair("",a.hdl));
        } else if (a.type == MESSAGE) {
            unique_lock<mutex> lock(m_session_lock);
            Message* wamp_msg = new Message(a.msg->get_payload());
            switch (wamp_msg->getType()) {
                case WELCOME:
                case CALLRESULT:
                case CALLERROR:
                case EVENT:
                    LOGGER_WRITE(Logger::ERROR,"Ignoring Server to Client message");
                    delete wamp_msg;
                    break;
                default:
                    break;
            }
        } else {
            // undefined.
        }
    }
}

void Server::send(connection_hdl hdl, Message* msg) {
    rapidjson::StringBuffer s(0, 1024*1024);
    msg->serialize(s);
    m_server.send(hdl,s.GetString(),
                         //s.Size(),
                          websocketpp::frame::opcode::text);
}

} // namespace WAMPP
