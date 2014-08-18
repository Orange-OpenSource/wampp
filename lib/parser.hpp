#ifndef WAMPP_PARSER_HPP_
#define WAMPP_PARSER_HPP_

// Copyright (c) Orange 2014
// Use of this source code is governed by a BSD-style licence
// that can be found in the LICENSE file.

#include <string>

#include "protocol.hpp"

using std::string;

namespace WAMPP {

Message* parseMessage(const string& buffer);

} // namespace WAMPP

#endif
