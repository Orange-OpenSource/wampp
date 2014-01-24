#include "server.hpp"
#include "logger.hpp"

void myrpc(connection_hdl hdl, std::string callId, WAMPP::Message* msg) {
    LOGGER_WRITE(Logger::DEBUG,"RPC CALLED");
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
