#ifndef WAMPP_PARSER_HPP_
#define WAMPP_PARSER_HPP_
#include <string>

#include "protocol.hpp"

using std::string;

namespace WAMPP {

Message* parseMessage(const string& buffer);

} // namespace WAMPP

#endif
