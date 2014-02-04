#include <string>

#include "rapidjson/document.h"

#include "json.hpp"
#include "parser.hpp"
#include "logger.hpp"

using std::string;

namespace WAMPP {

using JSON::Node;
using JSON::NodePtr;

NodePtr createNode(const rapidjson::Value &val) {
    NodePtr result;
    switch(val.GetType()) {
        case rapidjson::kNullType:
        {
            result = NodePtr(new Node(JSON::Null()));
            break;
        }
        case rapidjson::kFalseType:
        {
            result = NodePtr(new Node(false));
            break;
        }
        case rapidjson::kTrueType:
        {
            result = NodePtr(new Node(true));
            break;
        }
        case rapidjson::kObjectType:
        {
            result = NodePtr(new Node());
            result->data = JSON::Object();
            rapidjson::Value::ConstMemberIterator itr;
            for (itr = val.MemberBegin(); itr != val.MemberEnd(); ++itr) {
                NodePtr pChild = createNode(itr->value);
                boost::get<JSON::Object>(result->data).insert(
                    std::make_pair(itr->name.GetString(), pChild));
            }
            break;
        }
        case rapidjson::kArrayType:
        {
            result = NodePtr(new Node());
            result->data = JSON::Array();
            rapidjson::Value::ConstValueIterator itr;
            for (itr = val.Begin(); itr != val.End(); ++itr) {
                NodePtr pChild = createNode(*itr);
                boost::get<JSON::Array>(result->data).push_back(pChild);
            }
            break;
        }
        case rapidjson::kStringType:
        {
            result = NodePtr(new Node(std::string(val.GetString())));
            break;
        }
        case rapidjson::kNumberType:
        {
            if (val.IsInt())
                result = NodePtr(new Node(val.GetInt()));
            else if (val.IsUint())
                result = NodePtr(new Node(val.GetUint()));
            else if (val.IsInt64())
                result = NodePtr(new Node(val.GetInt64()));
            else if (val.IsUint64())
                result = NodePtr(new Node(val.GetUint64()));
            else result = NodePtr(new Node(val.GetDouble()));
            break;
        }
        default:
        {
            break;
        }
    }
    return result;
}

Message* parseMessage(const string& buffer) {

    // Expect the worse 
    Message* result = NULL;

    //Try to parse the JSON payload
    rapidjson::Document d;
    d.Parse<0>(buffer.c_str());

    if (d.HasParseError()) {
        LOGGER_WRITE(Logger::ERROR,d.GetParseError());
        return NULL;
    }

    if (!d.IsArray()
    || (d.Size() == 0)
    || (!d[0u].IsNumber())
    || (!d[0u].IsInt())) {
        LOGGER_WRITE(Logger::ERROR, "Not a WAMP message");
        return NULL;
    }

    int msg_type = d[0u].GetInt();
    if (msg_type < 0 || msg_type > 8) {
        LOGGER_WRITE(Logger::ERROR, "Invalid WAMP message type");
        return NULL;
    }

    int nargs = d.Size();

    switch (msg_type) {
        case WELCOME:
            if (4 == nargs) {
                if (d[1].IsString() && d[2].IsInt() && d[3].IsString()) {
                    std::string sessionId(d[1].GetString());
                    int protocolVersion(d[2].GetInt());
                    std::string serverIdent(d[3].GetString());
                    result = new Welcome(sessionId,protocolVersion,serverIdent);
                }
            }
            break;
       case PREFIX:
            if (3 == nargs) {
                if (d[1].IsString() && d[2].IsString()) {
                    string prefix(d[1].GetString());
                    string URI(d[2].GetString());
                    result = new Prefix(prefix,URI);
                }
            }
            break;
        case CALL:
            if (2 < nargs) {
                if (d[1].IsString() && d[2].IsString()) {
                    string callID(d[1].GetString());
                    string procURI(d[2].GetString());
                    std::vector<JSON::NodePtr> args;
                    for (unsigned int i=3; i<d.Size();i++) {
                        JSON::NodePtr arg = createNode(d[i]);
                        args.push_back(arg);
                    }
                    result = new Call(callID,procURI,args);
                }
            }
            break;
        case CALLRESULT:
            if (3 == nargs) {
                if (d[1].IsString()) {
                    string callID(d[1].GetString());
                    JSON::NodePtr event = createNode(d[2]);
                    result = new CallResult(callID,event);
                }
            }
            break;
        case CALLERROR:
            if (3 < nargs) {
                if (d[1].IsString() && d[2].IsString() && d[3].IsString()) {
                    string callID(d[1].GetString());
                    string errorURI(d[2].GetString());
                    string errorDesc(d[3].GetString());
                    if (5 == nargs) {
                        JSON::NodePtr errorDetails = createNode(d[4]);
                        result = new CallError(callID,
                                               errorURI,
                                               errorDesc,
                                               errorDetails);
                    } else {
                        result = new CallError(callID,
                                               errorURI,
                                               errorDesc);
                    }
                }
            }
            break;
        case SUBSCRIBE:
            if (2 == nargs) {
                if (d[1].IsString()) {
                    string topicURI(d[1].GetString());
                    result = new Subscribe(topicURI);
                }
            }
            break;
        case UNSUBSCRIBE:
            if (2 == nargs) {
                if (d[1].IsString()) {
                    string topicURI(d[1].GetString());
                    result = new UnSubscribe(topicURI);
                }
            }
            break;
        case PUBLISH:
            if (3 == nargs) {
                if (d[1].IsString()) {
                    string topicURI(d[1].GetString());
                    JSON::NodePtr event = createNode(d[2]);
                    result = new Publish(topicURI,event);
                }
            }
            break;
        case EVENT:
            if (3 == nargs) {
                if (d[1].IsString()) {
                    string topicURI(d[1].GetString());
                    JSON::NodePtr event = createNode(d[2]);
                    result = new Event(topicURI,event);
                }
            }
            break;
        default:
            LOGGER_WRITE(Logger::ERROR, "Unknown WAMP message");
            return NULL;
    }

    if (result == NULL) {
        LOGGER_WRITE(Logger::ERROR, "Malformed WAMP message");
    }

    return result;
}

} // namespace WAMPP
