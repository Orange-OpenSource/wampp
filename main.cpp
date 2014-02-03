#include "server.hpp"
#include "logger.hpp"
#include "json.hpp"

bool myrpc(connection_hdl hdl,
           std::string callId,
           std::vector<WAMPP::JSON::NodePtr> args,
           WAMPP::JSON::NodePtr& result) {
    
    bool ret;

    LOGGER_WRITE(Logger::DEBUG,"RPC Called");

    if (args.size()>0) {
        LOGGER_WRITE(Logger::DEBUG,"Params:");
        for(std::vector<int>::size_type i = 0; i != args.size(); i++) {
            std::ostringstream oss;
            oss << "#" << i << ":";
            boost::apply_visitor(WAMPP::JSON::Serializer(oss),args[i]->data);
            LOGGER_WRITE(Logger::DEBUG,oss.str());
        }

        result = WAMPP::JSON::NodePtr(new WAMPP::JSON::Node(string("OK")));
        ret = true;
    } else {
        result = WAMPP::JSON::NodePtr(new WAMPP::JSON::Node(string("No parameters provided")));
        ret = false;
    }
    std::ostringstream oss;
    boost::apply_visitor(WAMPP::JSON::Serializer(oss),result->data);
    LOGGER_WRITE(Logger::DEBUG,"Answering:");
    LOGGER_WRITE(Logger::DEBUG,oss.str());

    return ret;
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
