#include <string>
#include <vector>

#include <rapidjson/document.h>

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
        callID(id),
        procURI(uri) {}
    
    string callID;
    string procURI;
    vector<rapidjson::Value> args;
};

struct CallResult: Message {
    CallResult(string id,
               rapidjson::Value& res):
        Message(CALLRESULT),
        callID(id),
        result(res) {}
    
    string callID;
    rapidjson::Value& result;
};

struct CallError: Message {
    CallError(string id,
              string uri,
              string desc,
              rapidjson::Value& details):
        Message(CALLERROR),
        callID(id),
        errorURI(uri),
        errorDesc(desc),
        errorDetails(details) {}

/*    CallError(string id,
              string uri,
              string desc):
        Message(CALLERROR),
        callID(id),
        errorURI(uri),
        errorDesc(desc) {
        errorDetails = rapidjson::Value();
        } */

    string callID;
    string errorURI;
    string errorDesc;
    rapidjson::Value& errorDetails;
};

struct Subscribe: Message {
    Subscribe(string uri):
        Message(SUBSCRIBE),
        topicURI(uri) {}

    string topicURI;
};

struct UnSubscribe: Message {
    UnSubscribe(string uri):
        Message(UNSUBSCRIBE),
        topicURI(uri) {}

    string topicURI;
};

struct Publish: Message {
    Publish(string uri,
          rapidjson::Value& evt):
        Message(EVENT),
        topicURI(uri),
        event(evt) {}

    string topicURI;
    rapidjson::Value& event;
};

// Note: exclusion/eligibility not supported

struct Event: Message {
    Event(string uri,
          rapidjson::Value& evt):
        Message(EVENT),
        topicURI(uri),
        event(evt) {}

    string topicURI;
    rapidjson::Value& event;
};

} // namespace WAMPP
