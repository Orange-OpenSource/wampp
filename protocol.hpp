#ifndef WAMPP_PROTOCOL_HPP_
#define WAMPP_PROTOCOL_HPP_

#include <string>
#include <vector>

#include <rapidjson/document.h>
#include <rapidjson/stringbuffer.h>

#define WAMPP_PROTOCOL_VERSION 1

using namespace std;

namespace WAMPP {

enum message_type {
    WELCOME = 0,
    PREFIX,
    CALL,
    CALLRESULT,
    CALLERROR,
    SUBSCRIBE,
    UNSUBSCRIBE,
    PUBLISH,
    EVENT,
    UNDEFINED
};

class Message {
public:
    Message(message_type t);
    Message(const string& buffer);

    const message_type& getType() {
        return m_type;
    }
    rapidjson::Type getParamType(int index);
    const rapidjson::Value& getParam(unsigned int index);
    rapidjson::Document::AllocatorType getAllocator();

    void serialize(rapidjson::StringBuffer& s);

protected:
    rapidjson::Document m_d;

private:
    message_type m_type;
};

class Welcome: public Message {
public:
    Welcome(const string& sessionId, const string& serverIdent);
};

} // namespace WAMPP

#endif // WAMPP_PROTOCOL_HPP_
