#include "server.hpp"
#include "logger.hpp"
#include "json.hpp"

void myrpc(connection_hdl hdl, std::string callId, std::vector<WAMPP::JSON::NodePtr> args) {
    LOGGER_WRITE(Logger::DEBUG,"RPC CALLED with params:");

    for(std::vector<int>::size_type i = 0; i != args.size(); i++) {
        std::ostringstream oss;
        boost::apply_visitor(WAMPP::JSON::Serializer(oss),args[i]->data);
        LOGGER_WRITE(Logger::DEBUG,oss.str());
    }
}

int main() {
    // Log everything to stdout
    LOGGER_START(Logger::DEBUG, "")
    {
        WAMPP::JSON::Node root;
        root.data = WAMPP::JSON::Object();
        WAMPP::JSON::NodePtr pobj(new WAMPP::JSON::Node());
        pobj->data = WAMPP::JSON::Object();
        WAMPP::JSON::NodePtr ps(new WAMPP::JSON::Node());
        ps->data = "bar";
        boost::get<WAMPP::JSON::Object>(pobj->data).insert(
            std::make_pair("foo",ps));
        boost::get<WAMPP::JSON::Object>(root.data).insert(
            std::make_pair("inner",pobj));
    }
    {
        const char json[] = " { \"hello\" : \"world\", \"t\" : true , \"f\" : false, \"n\": null, \"i\":123, \"pi\": 3.1416, \"a\":[1, 2, 3, 4] } ";
        printf("Original JSON:\n %s\n", json);

        rapidjson::Document document;
        document.Parse<0>(json);
        
        WAMPP::JSON::Node clone(document);

        std::ostringstream oss;
        boost::apply_visitor(WAMPP::JSON::Serializer(oss),clone.data);
        LOGGER_WRITE(Logger::DEBUG,oss.str());

    }
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
