#include "server.hpp"
#include "logger.hpp"
#include "json.hpp"

WAMPP::JSON::NodePtr myrpc(connection_hdl hdl,
                           std::string callId,
                           std::vector<WAMPP::JSON::NodePtr> args) {

    LOGGER_WRITE(Logger::DEBUG,"RPC Called with params:");
    for(std::vector<int>::size_type i = 0; i != args.size(); i++) {
        std::ostringstream oss;
        oss << "#" << i << ":";
        boost::apply_visitor(WAMPP::JSON::Serializer(oss),args[i]->data);
        LOGGER_WRITE(Logger::DEBUG,oss.str());
    }

    WAMPP::JSON::NodePtr result(new WAMPP::JSON::Node(string("OK")));
    std::ostringstream oss;
    boost::apply_visitor(WAMPP::JSON::Serializer(oss),result->data);
    LOGGER_WRITE(Logger::DEBUG,"Answering:");
    LOGGER_WRITE(Logger::DEBUG,oss.str());

    return result;
}

int main() {
    // Log everything to stdout
    LOGGER_START(Logger::DEBUG, "")
    try {

        WAMPP::Server server("WAMPP Server 1.0");

        // Register RPC cp
        server.addRPC("test",&myrpc);

        // Start the server
        server.run(9002);

    } catch (std::exception & e) {
        LOGGER_WRITE(Logger::ERROR, e.what())
    }
    LOGGER_STOP()
}
