bin_PROGRAMS = wampp			

ACLOCAL_AMFLAGS = -I ../m4

wampp_SOURCES = main.cpp

wampp_CPPFLAGS = -I../lib \
				 $(LIBWAMPP_CPPFLAGS) \
				 -DENABLE_LOGGER

wampp_LDADD = ../lib/libwampp-@LIBWAMPP_API_VERSION@.la $(LIBWAMPP_LIBS)

RAPIDJSON_CPPFLAGS = -I$(RAPIDJSON_PATH)
