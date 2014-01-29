#include "json.hpp"

#include <rapidjson/document.h>

namespace WAMPP {

namespace JSON {

Node::Node(const rapidjson::Value &val) {
    switch(val.GetType()) {
        case rapidjson::kNullType:
        {
            data = Null();
            break;
        }
        case rapidjson::kFalseType:
        {
            data = false;
            break;
        }
        case rapidjson::kTrueType:
        {
            data = true;
            break;
        }
        case rapidjson::kObjectType:
        {
            data = Object();
            rapidjson::Value::ConstMemberIterator itr;
            for (itr = val.MemberBegin(); itr != val.MemberEnd(); ++itr) {
                NodePtr pChild(new Node(itr->value));
                boost::get<Object>(data).insert(
                    std::make_pair(itr->name.GetString(), pChild));
            }
            break;
        }
        case rapidjson::kArrayType:
        {
            data = Array();
            rapidjson::Value::ConstValueIterator itr;
            for (itr = val.Begin(); itr != val.End(); ++itr) {
                NodePtr pChild(new Node(*itr));
                boost::get<Array>(data).push_back(pChild);
            }
            break;
        }
        case rapidjson::kStringType:
        {
            data = std::string(val.GetString());
            break;
        }
        case rapidjson::kNumberType:
        {
            if (val.IsInt())
                data = val.GetInt();
            else if (val.IsUint())
                data = val.GetUint();
            else if (val.IsInt64())
                data = val.GetInt64();
            else if (val.IsUint64())
                data = val.GetUint64();
            else data = val.GetDouble();
            break;
        }
    }
}

} // namespace JSON

} //namespace WAMPP
