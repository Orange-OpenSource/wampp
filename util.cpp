#include <sstream>
#include <random>

#include "util.hpp"

using std::string;
using std::stringstream;
using std::random_device;
using std::uniform_int_distribution;

namespace WAMPP {

string genRandomId(int size) {
    stringstream ss;
    string chars(
                    "abcdefghijklmnopqrstuvwxyz"
                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                    "1234567890");
    random_device rng;
    uniform_int_distribution<> index_dist(0, chars.size() - 1);
    for(int i = 0; i < size; ++i) {
        ss << chars[index_dist(rng)];
    }
    return ss.str();
}

}
