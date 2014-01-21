#ifndef WAMPP_SERVER_HPP_
#define WAMPP_SERVER_HPP_

#include <string>

#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <websocketpp/common/thread.hpp>

using std::string;

typedef websocketpp::server<websocketpp::config::asio> WSServer;

using websocketpp::connection_hdl;

using websocketpp::lib::mutex;
using websocketpp::lib::condition_variable;

namespace WAMPP {

enum action_type {
    OPEN,
    CLOSE,
    MESSAGE
};

struct action {
    action(action_type t, connection_hdl h) : type(t), hdl(h) {}
    action(action_type t, WSServer::message_ptr m) : type(t), msg(m) {}

    action_type type;
    connection_hdl hdl;
    WSServer::message_ptr msg;
};

typedef std::pair<string,connection_hdl> Session;

class SessionComparator
{
public:
    bool operator()(const std::pair<std::string,connection_hdl>& a,
                    const std::pair<std::string,connection_hdl>& b) const {
        return std::owner_less<connection_hdl>()(a.second,b.second);
    }
};

class Message;

class Server {
public:
    Server(const string& ident);

    void run(uint16_t port);

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, WSServer::message_ptr msg);

    void actions_loop();

private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;
    typedef std::set<Session,SessionComparator> sess_list;

    WSServer m_server;
    con_list m_connections;
    sess_list m_sessions;
    std::queue<action> m_actions;

    mutex m_action_lock;
    mutex m_connection_lock;
    mutex m_session_lock;
    condition_variable m_action_cond;

    void send(connection_hdl hdl, Message* msg);
    
    const string m_ident;
};

} // namespace WAMPP

#endif // WAMPP_SERVER_HPP_
