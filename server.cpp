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

Server::Server() {
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
        std::cout << e.what() << std::endl;
    } catch (websocketpp::lib::error_code e) {
        std::cout << e.message() << std::endl;
    } catch (...) {
        std::cout << "other exception" << std::endl;
    }
}

void Server::on_open(connection_hdl hdl) {
    unique_lock<mutex> lock(m_action_lock);
    std::cout << "on_open" << std::endl;
    m_actions.push(action(SUBSCRIBE,hdl));
    lock.unlock();
    m_action_cond.notify_one(); 
}

void Server::on_close(connection_hdl hdl) {
    unique_lock<mutex> lock(m_action_lock);
    std::cout << "on_close" << std::endl;
    m_actions.push(action(UNSUBSCRIBE,hdl));
    lock.unlock();
    m_action_cond.notify_one();
}

void Server::on_message(connection_hdl hdl, server::message_ptr msg) {
    // queue message up for sending by processing thread
    unique_lock<mutex> lock(m_action_lock);
    std::cout << "on_message" << std::endl;
    m_actions.push(action(MESSAGE,msg));
    lock.unlock();
    m_action_cond.notify_one();
}

void Server::process_messages() {
    while(1) {
        unique_lock<mutex> lock(m_action_lock);

        while(m_actions.empty()) {
            m_action_cond.wait(lock);
        }

        action a = m_actions.front();
        m_actions.pop();

        lock.unlock();
        if (a.type == SUBSCRIBE) {
            unique_lock<mutex> lock(m_connection_lock);
            m_connections.insert(a.hdl);
        } else if (a.type == UNSUBSCRIBE) {
            unique_lock<mutex> lock(m_connection_lock);
            m_connections.erase(a.hdl);
        } else if (a.type == MESSAGE) {
            unique_lock<mutex> lock(m_connection_lock);

            con_list::iterator it;
            for (it = m_connections.begin(); it != m_connections.end(); ++it) {
                m_server.send(*it,a.msg);
            }
        } else {
            // undefined.
        }
    }
}

} // namespace WAMPP
