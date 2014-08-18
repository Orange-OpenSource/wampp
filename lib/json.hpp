#ifndef WAMPP_JSON_HPP_
#define WAMPP_JSON_HPP_

// Copyright (c) Orange 2014
// Use of this source code is governed by a BSD-style licence
// that can be found in the LICENSE file.

#include <map>
#include <sstream>

#include <boost/variant.hpp>
#include <boost/shared_ptr.hpp>

namespace WAMPP {

namespace JSON {

struct Null {};
struct Node;

typedef boost::shared_ptr<Node> NodePtr;

typedef std::map<std::string, NodePtr> Object;
typedef std::vector<NodePtr> Array;

typedef boost::variant<
    bool,
    int,
    unsigned int,
    int64_t,
    uint64_t,
    double,
    Null,
    std::string,
    Object,
    Array,
    boost::recursive_wrapper<Node>
> Value;

struct Node {
    Node() {}

    Node(bool b): data(b) {}
    Node(int i): data(i) {}
    Node(unsigned int ui): data(ui) {}
    Node(int64_t i): data(i) {}
    Node(uint64_t ui): data(ui) {}
    Node(double d): data(d) {}
    Node(Null n): data(n) {}
    Node(std::string s): data(s) {}

    Value data;
};

struct Serializer: public boost::static_visitor<> {

    Serializer(std::ostream& output) : m_output(output) {}

    void operator() (int i) const {
        m_output << i;
    }

    void operator() (unsigned int ui) const {
        m_output << ui;
    }

   void operator() (int64_t i) const {
        m_output << i;
    }

    void operator() (uint64_t ui) const {
        m_output << ui;
    }

    void operator() (double d) const {
        m_output << d;
    }

    void operator() (bool b) const {
        m_output << (b?"true":"false");
    }

    void operator() (const Null&) const {
        m_output << "null";
    }

    void operator() (const std::string& s) const {
        m_output << "\"" << s << "\"";
    }

    void operator() (const Object& o) const {
        m_output << "{";
        Object::const_iterator itr = o.begin();
        while (itr != o.end()) {
            m_output << "\"" << (*itr).first << "\":";
            boost::apply_visitor(Serializer(m_output),((*itr).second)->data);
            if (++itr != o.end()) {
                m_output << ",";
            }
        }
        m_output << "}";
    }

    void operator() (const Array& a) const {
        m_output << "[";
        Array::const_iterator itr = a.begin();
        while (itr != a.end()) {
            boost::apply_visitor(Serializer(m_output),(*itr)->data);
            if (++itr != a.end()) {
                m_output << ",";
            }
        }
        m_output << "]";
    }

    void operator() (const Node& n) const {
        return boost::apply_visitor(Serializer(m_output),n.data);
    }

private:
    std::ostream& m_output;

};

} // namespace JSON

} // namespace WAMPP

#endif // WAMP_JSON_HPP_
