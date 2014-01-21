#include "protocol.hpp"

#include <string>

using namespace std;

namespace WAMPP {

class Parser {

    Parser() {}

    Message* parse(string &message_buffer);

private:
    char m_errbuf[1024];

};

}
