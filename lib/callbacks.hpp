#ifndef WAMPP_RPC_HPP_
#define WAMPP_RPC_HPP_

// Copyright (c) Orange 2014
// Use of this source code is governed by a BSD-style licence
// that can be found in the LICENSE file.

#include <string>

#include "json.hpp"

namespace WAMPP {

class RemoteProc {
public:
    virtual bool invoke(std::string uri,
                        std::vector<JSON::NodePtr> args,
                        JSON::NodePtr& result) = 0; 
};

class SubFilter {
public:
    virtual bool subscribe(std::string uri) = 0;
    virtual void unsubscribe(std::string uri) = 0;
};

} // namespace WAMPP

#endif // WAMPP_RPC_HPP_


