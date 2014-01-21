#include <yajl/yajl_tree.h>

#include <string>
#include <cstring>

#include "logger.hpp"
#include "parser.hpp"

using namespace std;

namespace WAMPP {

Message* Parser::parse(string &message_buffer) {
    yajl_val node = yajl_tree_parse(message_buffer.c_str(),
                                    m_errbuf, sizeof(m_errbuf));
    if (node == NULL) {
        string error = "Parsing error:";
        if (strlen(m_errbuf))
            error += m_errbuf;
        else
            error += "unknown error";
        LOGGER_WRITE(Logger::ERROR, error);
        return NULL;
    }

    if (!YAJL_IS_ARRAY(node)
    || (YAJL_GET_ARRAY(node)->len == 0)
    || (!YAJL_IS_NUMBER(YAJL_GET_ARRAY(node)->values[0]))
    || (!((YAJL_GET_ARRAY(node)->values[0]->u.number.flags 
        & YAJL_NUMBER_INT_VALID)))){
        LOGGER_WRITE(Logger::ERROR, "Not a WAMP message");
        return NULL;
    }

    int msg_type = YAJL_GET_INTEGER(YAJL_GET_ARRAY(node)->values[0]);
    int nargs = YAJL_GET_ARRAY(node)->len;

    Message* message = NULL;

    switch (msg_type) {
        case WELCOME:
            if (4 == nargs) {
                if (YAJL_IS_STRING(YAJL_GET_ARRAY(node)->values[1])
                && YAJL_IS_NUMBER(YAJL_GET_ARRAY(node)->values[2])
                && YAJL_IS_STRING(YAJL_GET_ARRAY(node)->values[3])) {
                    message = new Welcome(
                        YAJL_GET_STRING(YAJL_GET_ARRAY(node)->values[1]),
                        YAJL_GET_INTEGER(YAJL_GET_ARRAY(node)->values[2]),
                        YAJL_GET_STRING(YAJL_GET_ARRAY(node)->values[3])
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
                if (YAJL_IS_STRING(YAJL_GET_ARRAY(node)->values[1])
                && YAJL_IS_STRING(YAJL_GET_ARRAY(node)->values[2])) {
                    message = new Prefix(
                        YAJL_GET_STRING(YAJL_GET_ARRAY(node)->values[1]),
                        YAJL_GET_STRING(YAJL_GET_ARRAY(node)->values[2])
                    );
                } else {
                    LOGGER_WRITE(Logger::ERROR, "Invalid argument in PREFIX message");
                }
            } else {
                LOGGER_WRITE(Logger::ERROR, "Wrong number of arguments in PREFIX message");
            }
            break;
        case CALL:
            if (3 < nargs) {
                if (YAJL_IS_STRING(YAJL_GET_ARRAY(node)->values[1])
                && YAJL_IS_STRING(YAJL_GET_ARRAY(node)->values[2])) {

                } else {
                    LOGGER_WRITE(Logger::ERROR, "Invalid argument in CALL message");
                }
            } else {
                LOGGER_WRITE(Logger::ERROR, "Wrong number of arguments in CALL message");
            }
            break;
        default:
            LOGGER_WRITE(Logger::ERROR, "Unknown WAMP message");
            break;
    }

    yajl_tree_free(node);

    return message;
}

}
