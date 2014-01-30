bin_PROGRAMS = wampp			

ACLOCAL_AMFLAGS = -I m4

wampp_SOURCES = main.cpp     \
				server.cpp   \
				protocol.cpp \
				json.cpp     \
				parser.cpp   \
				util.cpp     \
				logger.cpp

wampp_CPPFLAGS = $(WEBSOCKETPP_CPPFLAGS) \
				 $(BOOST_CPPFLAGS) \
				 $(RAPIDJSON_CPPFLAGS)\
				 -DENABLE_LOGGER

wampp_LDADD = $(WEBSOCKETPP_LIBS)

#					   -D_WEBSOCKETPP_NO_CPP11_REGEX_ \
#					   -D_WEBSOCKETPP_NOEXCEPT_TOKEN_='' \
#					   -DBOOST_NO_DEFAULTED_FUNCTIONS \
#					   -DBOOST_ASIO_DISABLE_STD_ADDRESSOF

WEBSOCKETPP_LIBS = $(BOOST_REGEX_LIB) $(BOOST_SYSTEM_LIB) -lpthread

RAPIDJSON_CPPFLAGS = -I$(RAPIDJSON_PATH)
