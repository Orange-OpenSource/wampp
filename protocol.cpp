#include <string>
#include <cstring>

#include "rapidjson/writer.h"

#include "logger.hpp"
#include "protocol.hpp"

using namespace std;

namespace WAMPP {

Message::Message(message_type t): m_type(t) {
    if (t != UNDEFINED) {
        // Initialize an empty JSON Array
        m_d.SetArray();
        // Set first parameter as Message type
        rapidjson::Document::AllocatorType& allocator = m_d.GetAllocator();
        m_d.PushBack(t,allocator);
    }
}

Message::Message(const string& message_buffer) {

    // Expect the worse 
    m_type = UNDEFINED;

    //Try to parse the JSON payload
    m_d.Parse<0>(message_buffer.c_str());

    if (m_d.HasParseError()) {
        LOGGER_WRITE(Logger::ERROR,m_d.GetParseError());
        return;
    }

    if (!m_d.IsArray()
    || (m_d.Size() == 0)
    || (!m_d[0u].IsNumber())
    || (!m_d[0u].IsInt())) {
        LOGGER_WRITE(Logger::ERROR, "Not a WAMP message");
        return;
    }

    int msg_type = m_d[0u].GetInt();
    if (msg_type < 0 || msg_type > 8) {
        LOGGER_WRITE(Logger::ERROR, "Invalid WAMP message type");
        return;
    }

    int nargs = m_d.Size();

    switch (msg_type) {
        case WELCOME:
            if (4 == nargs) {
                if (m_d[1].IsString() && m_d[2].IsInt() && m_d[3].IsString()) {
                    m_type= WELCOME;
                }
            }
            break;
       case PREFIX:
            if (3 == nargs) {
                if (m_d[1].IsString() && m_d[2].IsString()) {
                    m_type = PREFIX;
                }
            }
            break;
        case CALL:
            if (2 < nargs) {
                if (m_d[1].IsString() && m_d[2].IsString()) {
                    m_type = CALL;
                }
            }
            break;
        case CALLRESULT:
            if (3 == nargs) {
                if (m_d[1].IsString()) {
                    m_type = CALLRESULT;
                }
            }
            break;
        case CALLERROR:
            if (3 < nargs) {
                if (m_d[1].IsString() && m_d[2].IsString() && m_d[3].IsString()) {
                    m_type = CALLERROR;
                }
            }
            break;
        case SUBSCRIBE:
            if (2 == nargs) {
                if (m_d[1].IsString()) {
                    m_type = SUBSCRIBE;
                }
            }
            break;
        case UNSUBSCRIBE:
            if (2 == nargs) {
                if (m_d[1].IsString()) {
                    m_type = UNSUBSCRIBE;
                }
            }
            break;
        case PUBLISH:
            if (3 == nargs) {
                if (m_d[1].IsString()) {
                    m_type = PUBLISH;
                }
            }
            break;
        case EVENT:
            if (3 == nargs) {
                if (m_d[1].IsString()) {
                    m_type = EVENT;
                }
            }
            break;
        default:
            LOGGER_WRITE(Logger::ERROR, "Unknown WAMP message");
            return;
    }

    if (m_type == UNDEFINED) {
        LOGGER_WRITE(Logger::ERROR, "Malformed WAMP message");
    }
}

void Message::serialize(rapidjson::StringBuffer& s) {
    rapidjson::Writer<rapidjson::StringBuffer> writer(s);
    m_d.Accept(writer);
}

const rapidjson::Value& Message::getParam(unsigned int index) {
    if (index < m_d.Size()) {
        return m_d[index];
    } else {
        return NULL;
    }
}

Welcome::Welcome(const string& sessionId, const string& serverIdent):
    Message(WELCOME) {
    rapidjson::Document::AllocatorType& allocator = m_d.GetAllocator();
    rapidjson::Value v;
    v.SetString(sessionId.c_str(),sessionId.size(),allocator);
    m_d.PushBack(v,allocator);
    m_d.PushBack(WAMPP_PROTOCOL_VERSION,allocator);
    v.SetString(serverIdent.c_str(),serverIdent.size(),allocator);
    m_d.PushBack(v,allocator);
}

} // namespace WAMPP
