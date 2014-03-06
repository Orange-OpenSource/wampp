lib_LTLIBRARIES = libwampp-@LIBWAMPP_API_VERSION@.la			

ACLOCAL_AMFLAGS = -I ../m4

libwampp_@LIBWAMPP_API_VERSION@_la_SOURCES = server.cpp   \
					  						 protocol.hpp \
					  						 protocol.cpp \
					  						 parser.hpp   \
					  						 parser.cpp   \
					  						 util.hpp     \
					  						 util.cpp     \
					  						 logger.hpp   \
					  						 logger.cpp

libwampp_@LIBWAMPP_API_VERSION@_la_includedir = \
	$(includedir)/wampp-@LIBWAMPP_API_VERSION@
libwampp_@LIBWAMPP_API_VERSION@_la_include_HEADERS = server.hpp \
							  					     json.hpp   \
							  					     callbacks.hpp

libwampp_@LIBWAMPP_API_VERSION@_la_CPPFLAGS = $(LIBWAMPP_CPPFLAGS) \
				 						      -DENABLE_LOGGER

pkgconfigdir = $(libdir)/pkgconfig
pkgconfig_DATA = libwampp-$(LIBWAMPP_API_VERSION).pc
