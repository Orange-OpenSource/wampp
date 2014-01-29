bin_PROGRAMS = wampp			

wampp_SOURCES = main.cpp     \
				server.cpp   \
				protocol.cpp \
				json.cpp     \
				parser.cpp   \
				util.cpp     \
				logger.cpp

wampp_CPPFLAGS = $(WEBSOCKETPP_CPPFLAGS) \
				 $(RAPIDJSON_CPPFLAGS)\
				 -DENABLE_LOGGER

wampp_LDADD = $(WEBSOCKETPP_LIBS)

WEBSOCKETPP_CPPFLAGS = -I$(BOOST_PATH) \
					   -I$(WEBSOCKETPP_PATH) \
					   -std=c++0x \
					   -D_WEBSOCKETPP_CPP11_STL_ \
					   -D_WEBSOCKETPP_NO_CPP11_REGEX_

WEBSOCKETPP_LIBS = -lboost_regex -lboost_system -lpthread

RAPIDJSON_CPPFLAGS = -I$(RAPIDJSON_PATH)
