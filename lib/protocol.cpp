#include <string>
#include <cstring>

#include "rapidjson/writer.h"

#include "logger.hpp"
#include "protocol.hpp"

using std::string;

namespace WAMPP {

Message::Message(message_type t): m_type(t) {
    if (t != UNDEFINED) {
        // Initialize an empty JSON Array
        m_pNode = JSON::NodePtr(new JSON::Node());
        m_pNode->data = JSON::Array();
        // Set first parameter as Message type
        JSON::NodePtr pType = JSON::NodePtr(new JSON::Node(int(m_type)));
        boost::get<JSON::Array>(m_pNode->data).push_back(pType);
    }
}

void Message::serialize(std::ostream& output) const {
    if (m_pNode) {
        boost::apply_visitor(JSON::Serializer(output),m_pNode->data);
    }
}

void Message::appendString(const string& s) {
    if (m_pNode) {
        JSON::NodePtr pTmp = JSON::NodePtr(new JSON::Node(s));
        boost::get<JSON::Array>(m_pNode->data).push_back(pTmp);
    }
}

void Message::appendInt(int i) {
    if (m_pNode) {
        JSON::NodePtr pTmp = JSON::NodePtr(new JSON::Node(i));
        boost::get<JSON::Array>(m_pNode->data).push_back(pTmp);
    }
}

void Message::appendNode(JSON::NodePtr pNode) {
    if (m_pNode) {
        boost::get<JSON::Array>(m_pNode->data).push_back(pNode);
    }
}

JSON::NodePtr Message::getNode(unsigned int index) const {
    JSON::NodePtr result;
    if (m_pNode) {
        if (index < boost::get<JSON::Array>(m_pNode->data).size()){
            result = boost::get<JSON::Array>(m_pNode->data)[index];
        }
    }
    return result;
}

const string& Message::getString(unsigned int index) const {
    JSON::NodePtr pNode = getNode(index);
    if (pNode) {
        return boost::get<string>(pNode->data);
    } else {
        return string("");
    }
}

int Message::getInt(unsigned int index) const {
    JSON::NodePtr pNode = getNode(index);
    if (pNode) {
        return boost::get<int>(pNode->data);
    } else {
        return 0;
    }
}

Welcome::Welcome(const string& sessionId,
                 int protocolVersion,
                const string& serverIdent): Message(WELCOME) {
    appendString(sessionId);
    appendInt(protocolVersion);
    appendString(serverIdent);
}

Prefix::Prefix(const string& prefix,
               const string& URI): Message(PREFIX) {
    appendString(prefix);
    appendString(URI);
}

Call::Call(const string& callID,
           const string& procURI,
           std::vector<JSON::NodePtr> args): Message(CALL) {
    appendString(callID);
    appendString(procURI);
    std::vector<JSON::NodePtr>::const_iterator itr;
    for(itr = args.begin(); itr != args.end(); itr++) {
        appendNode(*itr);
    }
}

const string& Call::callID() const{
    return getString(1);
}

const string& Call::procURI() const{
    return getString(2);
}

std::vector<JSON::NodePtr> Call::args() const {
    std::vector<JSON::NodePtr> result;
    int index = 3;
    JSON::NodePtr pNode = getNode(index);
    while (pNode) {
        result.push_back(pNode);
        pNode = getNode(++index);
    }
    return result;
}

CallResult::CallResult(const string& callID,
                       JSON::NodePtr result): Message(CALLRESULT) {
    appendString(callID);
    appendNode(result);
}

CallError::CallError(const string& callID,
                     const string& errorURI,
                     const string& errorDesc,
                     JSON::NodePtr errorDetails): Message(CALLERROR) {
    appendString(callID);
    appendString(errorURI);
    appendString(errorDesc);
    if (errorDetails) {
        appendNode(errorDetails);
    }
}

Subscribe::Subscribe(const string& topicURI): Message(SUBSCRIBE) {
    appendString(topicURI);
}

UnSubscribe::UnSubscribe(const string& topicURI): Message(UNSUBSCRIBE) {
    appendString(topicURI);
}

Publish::Publish(const string& topicURI,
                 JSON::NodePtr event): Message(PUBLISH) {
    appendString(topicURI);
    appendNode(event);
}

Event::Event(const string& topicURI,
             JSON::NodePtr event): Message(EVENT) {
    appendString(topicURI);
    appendNode(event);
}

} // namespace WAMPP
