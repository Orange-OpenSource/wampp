AC_INIT([WAMPP], 1.0)
AM_INIT_AUTOMAKE
AC_PROG_CXX

AC_CONFIG_MACRO_DIR([m4])

dnl Add environment variables to specify dependency paths
AC_ARG_VAR([BOOST_PATH],[Path to Boost libraries])
AC_ARG_VAR([WEBSOCKETPP_PATH],[Path to WebSocketPP library])
AC_ARG_VAR([RAPIDJSON_PATH],[Path to rapidJSON library])

dnl Check for C++0x support, and set CXXFLAGS accordingly
AX_CXX_COMPILE_STDCXX_0X
if test "$ax_cv_cxx_compile_cxx0x_native" != yes &&
   test "$ax_cv_cxx_compile_cxx0x_cxx" != yes &&
   test "$ax_cv_cxx_compile_cxx0x_gxx" != yes; then
   AC_MSG_ERROR([A compiler supporting C++0X is required.])
fi
if test "$ax_cv_cxx_compile_cxx0x_cxx" == yes; then
    CXXFLAGS="$CXXFLAGS -std=c++0x"
else
    if test "$ax_cv_cxx_compile_cxx0x_gxx" == yes; then
        CXXFLAGS="$CXXFLAGS -std=gnu++0x"
    fi
fi

dnl Check for Boost libraries and:
dnl - Set BOOST_CPPFLAGS
dnl - Set BOOST_REGEX_LIB
dnl - Set BOOST_SYSTEM_LIB

AX_BOOST_BASE([1.49.0])
AX_BOOST_ASIO()
AX_BOOST_REGEX()
AX_BOOST_SYSTEM()

dnl Check for WebSocketPP library and:
dnl - Set WEBSOCKETPP_CPPFLAGS

WEBSOCKETPP_CPPFLAGS="-D_WEBSOCKETPP_CPP11_STL_"
WEBSOCKETPP_CPPFLAGS="$WEBSOCKETPP_CPPFLAGS -D_WEBSOCKETPP_NO_CPP11_REGEX_"
if test x"$WEBSOCKETPP_PATH" != x; then
    WEBSOCKETPP_CPPFLAGS="$WEBSOCKETPP_CPPFLAGS -I$WEBSOCKETPP_PATH"
fi

dnl Check for noexcept token
AX_CXX_NOEXCEPT()
if test "$ax_cv_cxx_noexcept" != yes; then
    WEBSOCKETPP_CPPFLAGS="WEBSOCKETPP_CPPFLAGS -D_WEBSOCKETPP_NOEXCEPT_TOKEN_=''"
fi

AC_LANG_PUSH([C++])
ac_save_cxx_flags="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS $WEBSOCKETPP_CPPFLAGS"
AC_CHECK_HEADERS([websocketpp/config/core.hpp], [],
    [AC_MSG_ERROR(You need the WebSocketPP library.)])
CXXFLAGS="$ac_save_cxx_flags"
AC_LANG_POP([C++])

AC_SUBST(WEBSOCKETPP_CPPFLAGS)

dnl Check for rapidJSON library
AC_LANG_PUSH([C++])
if test x"$RAPIDJSON_PATH" != x; then
    CPPFLAGS="$CPPFLAGS -I$RAPIDJSON_PATH"
fi
AC_CHECK_HEADERS([rapidjson/document.h], [],
    [AC_MSG_ERROR(You need the rapidJSON library.)])
AC_LANG_POP([C++])

AC_CONFIG_FILES(Makefile)
AC_OUTPUT
