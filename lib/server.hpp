#ifndef WAMPP_SERVER_HPP_
#define WAMPP_SERVER_HPP_

#include <string>

#include "callbacks.hpp"

using std::string;

namespace WAMPP {

class Server {
public:
    static Server* create(const string& ident);
    virtual ~Server() {};
    virtual void run(uint16_t port) = 0;
    virtual void addRPC(string uri, RemoteProc *rpc) = 0;
    virtual void registerSubFilter(SubFilter *sub) = 0;
    virtual void publish(string topic, JSON::NodePtr event) = 0;
protected:
    Server() {};
    Server(const Server& rhs) {};
private:
    Server& operator=(const Server& rhs);
};

} // namespace WAMPP

#endif // WAMPP_SERVER_HPP_
