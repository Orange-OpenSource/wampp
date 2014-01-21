#include <rapidjson/document.h>

#include <string>
#include <cstring>

#include "logger.hpp"
#include "parser.hpp"

using namespace std;

namespace WAMPP {

Message* Parser::parse(string &message_buffer) {
    rapidjson::Document d;
    d.Parse<0>(message_buffer.c_str());
   
    if (d.HasParseError()) {
        LOGGER_WRITE(Logger::ERROR,"Invalid JSON at offset"
                     + d.GetErrorOffset() 
                     + ":" + d.GetParseError() );
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
    int nargs = d.Size();

    Message* message = NULL;

    switch (msg_type) {
        case WELCOME:
            if (4 == nargs) {
                if (d[1].IsString() && d[2].IsInt() && d[3].IsString()) {
                    message = new Welcome(
                        d[1].GetString(),
                        d[2].GetInt(),
                        d[3].GetString()
                    );
                } else {
                    LOGGER_WRITE(Logger::ERROR, "Invalid argument in WELCOME message");
                }
            } else {
                LOGGER_WRITE(Logger::ERROR, "Wrong number of arguments in WELCOME message");
            }
            break;
       case PREFIX:
            if (3 == nargs) {
                if (d[1].IsString() && d[2].IsString()) {
                    message = new Prefix(
                        d[1].GetString(),
                        d[2].GetString()
                    );
                } else {
                    LOGGER_WRITE(Logger::ERROR, "Invalid argument in PREFIX message");
                }
            } else {
                LOGGER_WRITE(Logger::ERROR, "Wrong number of arguments in PREFIX message");
            }
            break;
        case CALL:
            if (2 < nargs) {
                if (d[1].IsString() && d[2].IsString()) {
                    message = new Call(
                        d[1].GetString(),
                        d[2].GetString()
                    );
                    for (rapidjson::SizeType i = 3; i < d.Size(); i++)
                        static_cast<Call*>(message)->args.push_back(d[i]);
                } else {
                    LOGGER_WRITE(Logger::ERROR, "Invalid argument in CALL message");
                }
            } else {
                LOGGER_WRITE(Logger::ERROR, "Wrong number of arguments in CALL message");
            }
            break;
        case CALLRESULT:
            if (3 == nargs) {
                if (d[1].IsString()) {
                    message = new CallResult(d[1].GetString(),d[2]);
                } else {
                    LOGGER_WRITE(Logger::ERROR, "Invalid argument in CALLRESULT message");
                }
            } else {
                LOGGER_WRITE(Logger::ERROR, "Wrong number of arguments in CALLRESULT message");
            }
            break;
        case CALLERROR:
            if (3 < nargs) {
                if (d[1].IsString() && d[2].IsString() && d[3].IsString()) {
                    if (4 == nargs) {
                        message = new CallError(
                            d[1].GetString(),
                            d[2].GetString(),
                            d[3].GetString(),
                            d[4]);
                    } else {
                        // TODO
                    }
                } else {
                    LOGGER_WRITE(Logger::ERROR, "Invalid argument in CALLERROR message");
                }
            } else {
                LOGGER_WRITE(Logger::ERROR, "Wrong number of arguments in CALLERROR message");
            }
            break;
        case SUBSCRIBE:
            if (2 == nargs) {
                if (d[1].IsString()) {
                    message = new Subscribe(d[1].GetString());
                } else {
                    LOGGER_WRITE(Logger::ERROR, "Invalid argument in SUBSCRIBE message");
                }
            } else {
                LOGGER_WRITE(Logger::ERROR, "Wrong number of arguments in SUBSCRIBE message");
            }
            break;
        case UNSUBSCRIBE:
            if (2 == nargs) {
                if (d[1].IsString()) {
                    message = new UnSubscribe(d[1].GetString());
                } else {
                    LOGGER_WRITE(Logger::ERROR, "Invalid argument in UNSUBSCRIBE message");
                }
            } else {
                LOGGER_WRITE(Logger::ERROR, "Wrong number of arguments in UNSUBSCRIBE message");
            }
            break;
        case PUBLISH:
            if (3 == nargs) {
                if (d[1].IsString()) {
                    message = new Publish(d[1].GetString(),d[2]);
                } else {
                    LOGGER_WRITE(Logger::ERROR, "Invalid argument in PUBLISH message");
                }
            } else {
                LOGGER_WRITE(Logger::ERROR, "Wrong number of arguments in PUBLISH message");
            }
            break;
        case EVENT:
            if (3 == nargs) {
                if (d[1].IsString()) {
                    message = new Event(d[1].GetString(),d[2]);
                } else {
                    LOGGER_WRITE(Logger::ERROR, "Invalid argument in EVENT message");
                }
            } else {
                LOGGER_WRITE(Logger::ERROR, "Wrong number of arguments in EVENT message");
            }
            break;
        default:
            LOGGER_WRITE(Logger::ERROR, "Unknown WAMP message");
            break;
    }

    return message;
}

} // namespace WAMPP
