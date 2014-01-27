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

    message_type getType() const {
        return m_type;
    }
    const rapidjson::Value& getParam(unsigned int index) const;
    unsigned int getParamSize() const;

    void serialize(rapidjson::StringBuffer& s) const;

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
