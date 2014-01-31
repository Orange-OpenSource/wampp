# ===========================================================================
#     ax_cxx_addressof.m4
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CXX_ADDRESSOF
#
# DESCRIPTION
#
#   If the C++ compiler supports std::addressof, define HAVE_CXX_ADDRESSOF.
#
# LICENSE
#
#   Copyright (c) 2014 David Corvoysier
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AU_ALIAS([AC_CXX_ADDRESSOF], [AX_CXX_ADDRESSOF])
AC_DEFUN([AX_CXX_ADDRESSOF],
[AC_CACHE_CHECK(whether the compiler supports std::addressof,
ax_cv_cxx_addressof,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 SAVE_CXXFLAGS=$CXXFLAGS
 AC_TRY_COMPILE([#include <memory>],[using std::addressof;],
 ax_cv_cxx_addressof=yes, ax_cv_cxx_addressof=no)
 CXXFLAGS=$SAVE_CXXFLAGS
 AC_LANG_RESTORE
])
if test "$ax_cv_cxx_addressof" = yes; then
  AC_DEFINE(HAVE_CXX_ADDRESSOF,,[define if the compiler supports std::addressof])
fi
])
