#include "server.hpp"
#include "logger.hpp"
#include "json.hpp"

#include <thread>

class MyRemoteProc: public WAMPP::RemoteProc
{

bool invoke(std::string callId,
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
};

class MySubFilter : public WAMPP::SubFilter {
    bool subscribe(string uri) {
        LOGGER_WRITE(Logger::DEBUG,"Subscription Received");
        return true;
    }
    void unsubscribe(string uri) {
        LOGGER_WRITE(Logger::DEBUG,"Unsubscription Received");
    }
};

std::string getFmtTime() {

    std::time_t rawtime;
    std::tm* timeinfo;
    char buffer [80];

    std::time(&rawtime);
    timeinfo = std::localtime(&rawtime);

    std::strftime(buffer,80,"%Y-%m-%d-%H-%M-%S",timeinfo);

    return std::string(buffer);

}


void eventLoop(WAMPP::Server* server) {
    while(1) {
        // send an event every 5 seconds
        sleep(5);
        WAMPP::JSON::NodePtr event = WAMPP::JSON::NodePtr(new WAMPP::JSON::Node(getFmtTime()));
        server->publish("foo",event);
    }
}

int main() {
    // Log everything to stdout
    LOGGER_START(Logger::DEBUG, "")
    try {

        WAMPP::Server* server = WAMPP::Server::create("WAMPP Server 1.0");

        // Register RPC cb
        MyRemoteProc myrpc;
        server->addRPC("test",&myrpc);

        // Register PubSub cb
        MySubFilter mysub;
        server->registerSubFilter(&mysub);

        // Start a thread to generate events
        std::thread eventThread(eventLoop,server);

        // Start the server
        server->run(9002);

        eventThread.join();
        delete server;

    } catch (std::exception & e) {
        LOGGER_WRITE(Logger::ERROR, e.what())
    }
    LOGGER_STOP()
}
