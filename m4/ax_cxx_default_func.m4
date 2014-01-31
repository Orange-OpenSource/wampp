# ===========================================================================
#     ax_cxx_default_func.m4
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CXX_DEFAULT_FUNC
#
# DESCRIPTION
#
#   If the C++ compiler supports the default keyword for functions, define 
#   HAVE_CXX_DEFAULT_FUNC.
#
# LICENSE
#
#   Copyright (c) 2014 David Corvoysier
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AU_ALIAS([AC_CXX_DEFAULT_FUNC], [AX_CXX_DEFAULT_FUNC])
AC_DEFUN([AX_CXX_DEFAULT_FUNC],
[AC_CACHE_CHECK(whether the compiler supports the default keyword for functions,
ax_cv_cxx_default_func,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 SAVE_CXXFLAGS=$CXXFLAGS
 AC_TRY_COMPILE(,[include <memory>;using std::addressof;],
 ax_cv_cxx_default_func=yes, ax_cv_cxx_default_func=no)
 CXXFLAGS=$SAVE_CXXFLAGS
 AC_LANG_RESTORE
])
if test "$ax_cv_cxx_default_func" = yes; then
  AC_DEFINE(HAVE_CXX_DEFAULT_FUNC,,[define if the compiler supports the default keyword for functions])
fi
])
