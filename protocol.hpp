#ifndef WAMPP_PROTOCOL_HPP_
#define WAMPP_PROTOCOL_HPP_

#include <string>
#include <vector>

#include "json.hpp"

#define WAMPP_PROTOCOL_VERSION 1

using std::string;

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
    void serialize(std::ostream & output) const;

protected:
    JSON::NodePtr m_pNode;
    void appendString(const string& s);
    void appendInt(int i);
    void appendNode(JSON::NodePtr pNode);
    JSON::NodePtr getNode(unsigned int index) const;
    const string& getString(unsigned int index) const;
    int getInt(unsigned int index) const;
private:
    message_type m_type;
};

class Welcome: public Message {
public:
    Welcome(const string& sessionId,
            int protocolVersion,
            const string& serverIdent);
    const string& sessionId() const;
    int protocolVersion() const;
    const string& serverIdent() const;
};

class Prefix: public Message {
public:
    Prefix(const string& prefix,
           const string& URI);
    const string& prefix() const;
    const string& URI() const;
};

class Call: public Message {
public:
    Call(const string& callID,
         const string& procURI,
         std::vector<JSON::NodePtr> args);
    const string& callID() const;
    const string& procURI() const;
    std::vector<JSON::NodePtr> args() const;
};

class CallResult: public Message {
public:
    CallResult(const string& callID, JSON::NodePtr result);
    const string& callID();
    JSON::NodePtr result();
};

class CallError: public Message {
public:
    CallError(const string& callID,
              const string& errorURI,
              const string& errorDesc,
              JSON::NodePtr errorDetails = NULL);
    const string& callID();
    const string& errorURI();
    const string& errorDesc();
    JSON::NodePtr errorDetails();
};

class Subscribe: public Message {
public:
    Subscribe(const string& topicURI);
    const string& topicURI();
};

class UnSubscribe: public Message {
public:
    UnSubscribe(const string& topicURI);
    const string& topicURI();
};

class Publish: public Message {
public:
    Publish(const string& topicURI,
            JSON::NodePtr event);
    const string& topicURI();
    JSON::NodePtr event();
};

class Event: public Message {
public:
    Event(const string& topicURI,
          JSON::NodePtr event);
    const string& topicURI();
    JSON::NodePtr event();
};

} // namespace WAMPP

#endif // WAMPP_PROTOCOL_HPP_
