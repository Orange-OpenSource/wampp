AC_INIT([WAMPP], 1.0)
AM_INIT_AUTOMAKE
AC_PROG_CXX

dnl Add environment variables to specify dependency paths
AC_ARG_VAR([BOOST_PATH],[Path to Boost libraries])
AC_ARG_VAR([WEBSOCKETPP_PATH],[Path to WebSocketPP library])
AC_ARG_VAR([RAPIDJSON_PATH],[Path to rapidJSON library])

dnl Store CPPFLAGS while doing our checks
oldCPPFLAGS=$CPPFLAGS

dnl Check for Boost library
AC_LANG_PUSH([C++])
if test x"$BOOST_PATH" != x; then
    CPPFLAGS="$CPPFLAGS -I$BOOST_PATH"
fi
AC_CHECK_HEADERS([boost/foreach.hpp], [],
    [AC_MSG_ERROR(You need the Boost libraries.)])
AC_LANG_POP([C++])

dnl Check for WebSocketPP library
AC_LANG_PUSH([C++])
if test x"$WEBSOCKETPP_PATH" != x; then
    CPPFLAGS="$CPPFLAGS -I$WEBSOCKETPP_PATH"
fi
AC_CHECK_HEADERS([websocketpp/config/core.hpp], [],
    [AC_MSG_ERROR(You need the WebSocketPP library.)])
AC_LANG_POP([C++])

dnl Check for rapidJSON library
AC_LANG_PUSH([C++])
if test x"$RAPIDJSON_PATH" != x; then
    CPPFLAGS="$CPPFLAGS -I$RAPIDJSON_PATH"
fi
AC_CHECK_HEADERS([rapidjson/document.h], [],
    [AC_MSG_ERROR(You need the rapidJSON library.)])
AC_LANG_POP([C++])

dnl Restore CPP Flags
CPPFLAGS=$oldCPPFLAGS

AC_CONFIG_FILES(Makefile)
AC_OUTPUT
