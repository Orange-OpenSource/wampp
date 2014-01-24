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
using websocketpp::lib::thread;
using websocketpp::lib::shared_ptr;
using websocketpp::lib::function;

namespace WAMPP {

class Message;

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

typedef ::function<void(connection_hdl,string,Message*)> RemoteProc;

class Server {
public:
    Server(const string& ident);
    ~Server();

    void run(uint16_t port);

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, WSServer::message_ptr msg);

    void actions_loop();

    void addRPC(string uri, RemoteProc rpc);

private:
    WSServer m_server;

    shared_ptr<thread> m_actions_thread;
    
    std::set<Session,SessionComparator> m_sessions;
    std::queue<action> m_actions;
    std::map<string,RemoteProc> m_rpcs; 

    mutex m_action_lock;
    mutex m_session_lock;
    mutex m_rpc_lock;
    condition_variable m_action_cond;

    void send(connection_hdl hdl, Message* msg);
    
    const string m_ident;
};

} // namespace WAMPP

#endif // WAMPP_SERVER_HPP_
