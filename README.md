# WAMPP: A WAMP V1 C++ library
##Copyright Orange 2014, all Rights Reserved

WAMPP is a C++ library that implements the Websocket Application Messaging Protocol (http://wamp.ws/spec/) Version 1. It allows integrating WAMPP client and server functionality into C++ programs. WAMPP is based on Websocket++ and Rapidjson.

Note that the WAMP V1 protocol has been deprecated: this library is therefore merely archived here to provide a basis for a newest WAMP V2 implementation.

## Linux Build Instructions

Prerequisites:
- [autoconf](http://www.gnu.org/software/autoconf/)
- [libtool](https://www.gnu.org/software/libtool/)
- [boost](http://www.boost.org/)
- [websocketpp](https://github.com/zaphoyd/websocketpp)
- [rapidjson](https://code.google.com/p/rapidjson/)

1. Clone the WAMPP repository

    git clone scheme://path/to/wampp

2. Enter the WAMPP directory

    cd wampp

3. Build

    autoreconf -i && ./configure && make
