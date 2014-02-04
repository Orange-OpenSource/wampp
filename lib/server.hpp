#ifndef WAMPP_SERVER_HPP_
#define WAMPP_SERVER_HPP_

#include <string>
#include <functional>

#include "json.hpp"

using std::string;

namespace WAMPP {

typedef std::function<bool(string,std::vector<JSON::NodePtr>,JSON::NodePtr&)> RemoteProc;

class Server {
public:
    static Server* create(const string& ident);
    virtual ~Server() {};
    virtual void run(uint16_t port) = 0;
    virtual void addRPC(string uri, RemoteProc rpc) = 0;
protected:
    Server() {};
    Server(const Server& rhs) {};
private:
    Server& operator=(const Server& rhs);
};

} // namespace WAMPP

#endif // WAMPP_SERVER_HPP_
