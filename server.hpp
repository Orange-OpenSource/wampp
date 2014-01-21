#include <websocketpp/config/asio_no_tls.hpp>

#include <websocketpp/server.hpp>

#include <websocketpp/common/thread.hpp>

typedef websocketpp::server<websocketpp::config::asio> WSServer;

using websocketpp::connection_hdl;

using websocketpp::lib::mutex;
using websocketpp::lib::condition_variable;

enum action_type {
    SUBSCRIBE,
    UNSUBSCRIBE,
    MESSAGE
};

struct action {
    action(action_type t, connection_hdl h) : type(t), hdl(h) {}
    action(action_type t, WSServer::message_ptr m) : type(t), msg(m) {}

    action_type type;
    websocketpp::connection_hdl hdl;
    WSServer::message_ptr msg;
};

namespace WAMPP {

class Server {
public:
    Server();

    void run(uint16_t port);

    void on_open(connection_hdl hdl);
    void on_close(connection_hdl hdl);
    void on_message(connection_hdl hdl, WSServer::message_ptr msg);

    void process_messages();

private:
    typedef std::set<connection_hdl,std::owner_less<connection_hdl>> con_list;

    WSServer m_server;
    con_list m_connections;
    std::queue<action> m_actions;

    mutex m_action_lock;
    mutex m_connection_lock;
    condition_variable m_action_cond;
};

} // namespace WAMPP
