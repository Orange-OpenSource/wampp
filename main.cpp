#include <iostream>
#include "wamp_server.hpp"

int main() {
	try {
	wamp_server server;

	// Start a thread to run the processing loop
	thread t(bind(&wamp_server::process_messages,&server));

	// Run the asio loop with the main thread
	server.run(9002);

	t.join();

	} catch (std::exception & e) {
	    std::cout << e.what() << std::endl;
	}
}
