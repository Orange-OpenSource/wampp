#include <string>

#include "protocol.hpp"

using namespace std;

namespace WAMPP {

class Parser {

    Parser() {}

    Message* parse(string &message_buffer);

};

}
