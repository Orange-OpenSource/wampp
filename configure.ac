AC_INIT([LIBWAMPP], 1.0)
AM_INIT_AUTOMAKE
AC_PROG_CXX
LT_INIT

AC_SUBST([LIBWAMPP_SO_VERSION], [1:0:0])
AC_SUBST([LIBWAMPP_API_VERSION], [1.0])

AC_CONFIG_MACRO_DIR([m4])

## 
## Check LIBWAMPP Prerequisites
##

dnl Add environment variables to specify dependency paths
AC_ARG_VAR([BOOST_PATH],[Path to Boost libraries])
AC_ARG_VAR([WEBSOCKETPP_PATH],[Path to WebSocketPP library])
AC_ARG_VAR([RAPIDJSON_PATH],[Path to rapidJSON library])

dnl Check for C++0x support, and set CXXFLAGS accordingly
AX_CXX_COMPILE_STDCXX_0X()
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

dnl Check for Boost libraries

dnl Boost base (this sets BOOST_CPPFLAGS)
AX_BOOST_BASE([1.49.0])

dnl Boost system (set BOOST_SYSTEM_CPPFLAGS and BOOST_SYSTEM_LIB)

dnl Workaround wrong detection of the default keyword
AX_CXX_DEFAULT_FUNC()
if test "$ax_cv_cxx_default_func" != yes; then
    BOOST_SYSTEM_CPPFLAGS="-DBOOST_NO_DEFAULTED_FUNCTIONS"
fi

AC_LANG_PUSH([C++])
ac_save_cxx_flags="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS $BOOST_SYSTEM_CPPFLAGS"
AC_CHECK_HEADERS([boost/system/error_code.hpp], [],
    [AC_MSG_ERROR(You need the Boost system library.)])
CXXFLAGS="$ac_save_cxx_flags"
AC_LANG_POP([C++])

BOOST_SYSTEM_LIB="-lboost_system"

AC_SUBST(BOOST_SYSTEM_CPPFLAGS)
AC_SUBST(BOOST_SYSTEM_LIB)

dnl Boost Asio (Set BOOST_ASIO_CPPFLAGS)

dnl Workaround wrong detection of std::addressof
AX_CXX_ADDRESSOF()
if test "$ax_cv_cxx_addressof" != yes; then
    BOOST_ASIO_CPPFLAGS="-DBOOST_ASIO_DISABLE_STD_ADDRESSOF"
fi

AC_LANG_PUSH([C++])
ac_save_cxx_flags="$CXXFLAGS"
CXXFLAGS="$CXXFLAGS \
          $BOOST_SYSTEM_CPPFLAGS
          $BOOST_ASIO_CPPFLAGS"
AC_CHECK_HEADERS([boost/asio.hpp], [],
    [AC_MSG_ERROR(You need the Boost asio library.)])
CXXFLAGS="$ac_save_cxx_flags"
AC_LANG_POP([C++])

AC_SUBST(BOOST_ASIO_CPPFLAGS)

dnl Boost regex
AC_LANG_PUSH([C++])
AC_CHECK_HEADERS([boost/regex.hpp], [],
    [AC_MSG_ERROR(You need the Boost Regex library.)])
AC_LANG_POP([C++])

dnl Check for WebSocketPP library (Set WEBSOCKETPP_CPPFLAGS)

WEBSOCKETPP_CPPFLAGS="-D_WEBSOCKETPP_CPP11_STL_"
WEBSOCKETPP_CPPFLAGS="$WEBSOCKETPP_CPPFLAGS -D_WEBSOCKETPP_NO_CPP11_REGEX_"
if test x"$WEBSOCKETPP_PATH" != x; then
    WEBSOCKETPP_CPPFLAGS="$WEBSOCKETPP_CPPFLAGS -I$WEBSOCKETPP_PATH"
fi

dnl Check for noexcept token
AX_CXX_NOEXCEPT()
if test "$ax_cv_cxx_noexcept" != yes; then
    WEBSOCKETPP_CPPFLAGS="$WEBSOCKETPP_CPPFLAGS -D_WEBSOCKETPP_NOEXCEPT_TOKEN_="
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

##
## Generate Output Files
##

AC_CONFIG_FILES(Makefile
                lib/Makefile
                lib/libwampp-${LIBWAMPP_API_VERSION}.pc:lib/libwampp.pc.in
                examples/Makefile)
AC_OUTPUT
