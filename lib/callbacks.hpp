#ifndef WAMPP_RPC_HPP_
#define WAMPP_RPC_HPP_

#include <string>

#include "json.hpp"

namespace WAMPP {

class RemoteProc {
public:
    virtual bool invoke(std::string uri,
                        std::vector<JSON::NodePtr> args,
                        JSON::NodePtr& result) = 0; 
};

} // namespace WAMPP

#endif // WAMPP_RPC_HPP_


