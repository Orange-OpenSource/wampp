#include <string>
#include <vector>

using namespace std;

namespace WAMPP {

int PROTOCOL_VERSION = 1;

enum message_type {
    WELCOME = 0,
    PREFIX,
    CALL,
    CALLRESULT,
    CALLERROR,
    SUBSCRIBE,
    UNSUBSCRIBE,
    PUBLISH,
    EVENT
};

struct Message {
    Message(message_type t): type(t) {}

    message_type type;
};

struct Welcome: Message {
    Welcome(string id,
            int version,
            string ident):
        Message(WELCOME),
        sessionId(id),
        protocolVersion(version),
        serverIdent(ident) {}

    string sessionId;
    int protocolVersion;
    string serverIdent;
};

struct Prefix: Message {
    Prefix(string prefix,
           string uri):
        Message(PREFIX),
        prefix(prefix),
        URI(uri) {}

    string prefix;
    string URI;
};

struct Call: Message {
    Call(string id,
         string uri):
        Message(CALL),
        callId(id),
        procURI(uri) {}
    
    string callId;
    string procURI;
    vector<JSON::Value> args;
};

}
