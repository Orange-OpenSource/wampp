#include <iostream>
#include "server.hpp"
#include "logger.hpp"

using websocketpp::lib::bind;
using websocketpp::lib::thread;

int main() {
    // Log everything to stdout
    LOGGER_START(Logger::DEBUG, "")
    try {
        WAMPP::Server server("WAMPP Server 1.0");

        // Start a thread to run the processing loop
        thread t(bind(&WAMPP::Server::actions_loop,&server));

        // Run the asio loop with the main thread
        server.run(9002);

        t.join();

    } catch (std::exception & e) {
        LOGGER_WRITE(Logger::ERROR, e.what())
    }
    LOGGER_STOP()
}
