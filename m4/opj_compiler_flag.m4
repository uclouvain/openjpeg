dnl Copyright (C) 2010 Vincent Torri <vtorri at univ-evry dot fr>
dnl                and Albin Tonnerre <albin dot tonnerre at gmail dot com>
dnl That code is public domain and can be freely used or copied.

dnl Macro that checks if a compiler flag is supported by the compiler.

dnl Usage: OPJ_COMPILER_FLAG(flag)
dnl flag is added to CFLAGS if supported.

AC_DEFUN([OPJ_COMPILER_FLAG],
[

CFLAGS_save="${CFLAGS}"
CFLAGS="${CFLAGS} $1"

AC_LANG_PUSH([C])
AC_MSG_CHECKING([whether the compiler supports $1])

AC_COMPILE_IFELSE(
   [AC_LANG_PROGRAM([[]])],
   [have_flag="yes"],
   [have_flag="no"])
AC_MSG_RESULT([${have_flag}])

if test "x${have_flag}" = "xno" ; then
   CFLAGS="${CFLAGS_save}"
fi
AC_LANG_POP([C])

])
