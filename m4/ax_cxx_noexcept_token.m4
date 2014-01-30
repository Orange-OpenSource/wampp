# ===========================================================================
#     ax_cxx_noexcept_token.m4
# ===========================================================================
#
# SYNOPSIS
#
#   AX_CXX_NOEXCEPT
#
# DESCRIPTION
#
#   If the C++ compiler supports the noexcept keyword, define HAVE_NOEXCEPT.
#
# LICENSE
#
#   Copyright (c) 2014 David Corvoysier
#
#   Copying and distribution of this file, with or without modification, are
#   permitted in any medium without royalty provided the copyright notice
#   and this notice are preserved. This file is offered as-is, without any
#   warranty.

AU_ALIAS([AC_CXX_NOEXCEPT], [AX_CXX_NOEXCEPT])
AC_DEFUN([AX_CXX_NOEXCEPT],
[AC_CACHE_CHECK(whether the compiler supports the noexcept token,
ax_cv_cxx_noexcept,
[AC_LANG_SAVE
 AC_LANG_CPLUSPLUS
 SAVE_CXXFLAGS=$CXXFLAGS
 AC_TRY_COMPILE(,[void foo() noexcept;],
 ax_cv_cxx_noexcept=yes, ax_cv_cxx_noexcept=no)
 CXXFLAGS=$SAVE_CXXFLAGS
 AC_LANG_RESTORE
])
if test "$ax_cv_cxx_noexcept" = yes; then
  AC_DEFINE(HAVE_NOEXCEPT,,[define if the compiler supports the noexcept token])
fi
])
