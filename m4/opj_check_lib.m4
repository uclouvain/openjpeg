dnl Copyright (C) 2011 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if a library is in a specified directory.

dnl Usage: OPJ_CHECK_LIB_WITH_PREFIX(prefix, header, lib, func [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Call AC_SUBST(THELIB_CFLAGS)
dnl Call AC_SUBST(THELIB_LIBS)
dnl where THELIB is the uppercase value of 'lib'

AC_DEFUN([OPJ_CHECK_LIB_WITH_PREFIX],
[

m4_pushdef([UP], m4_toupper([$3]))
m4_pushdef([DOWN], m4_tolower([$3]))

__opj_prefix=$1
__opj_header=$2
__opj_lib=$3
__opj_func=$4
__opj_have_dep="no"

save_CPPFLAGS=${CPPFLAGS}
save_LDFLAGS=${LDFLAGS}
save_LIBS=${LIBS}

if test "x${__opj_prefix}" = "x" ; then
   CPPFLAGS="${CPPFLAGS} $UP[_CFLAGS]"
   LDFLAGS="${LDFLAGS} $UP[_LIBS]"
else
   CPPFLAGS="${CPPFLAGS} -I${__opj_prefix}/include"
   LDFLAGS="${LDFLAGS} -L${__opj_prefix}/lib"
   LIBS="${LIBS} -l${__opj_lib}"
fi

AC_LINK_IFELSE(
   [AC_LANG_PROGRAM(
       [[
#include <${__opj_header}>
       ]],
       [[
${__opj_func}();
       ]])],
       [__opj_have_dep="yes"],
       [__opj_have_dep="no"])

CPPFLAGS=${save_CPPFLAGS}
LDFLAGS=${save_LDFLAGS}
LIBS=${save_LIBS}

if test "x${__opj_prefix}" = "x" ; then
   AC_MSG_CHECKING([whether ]UP[ library is available in standard or predefined directories])
else
   AC_MSG_CHECKING([whether ]UP[ library is available in ${__opj_prefix}])
fi
AC_MSG_RESULT([${__opj_have_dep}])

if test "x${__opj_have_dep}" = "xyes" && ! test "x${__opj_prefix}" = "x"; then
   if test "x${UP[]_CFLAGS}" = "x" ; then
      UP[]_CFLAGS="-I${__opj_prefix}/include"
   fi
   if test "x${UP[]_LIBS}" = "x" ; then
      UP[]_LIBS="-L${__opj_prefix}/lib -l${__opj_lib}"
   fi
fi

AS_IF([test "x${__opj_have_dep}" = "xyes"], [$5], [$6])

m4_popdef([UP])
m4_popdef([DOWN])

])

dnl Macro that check if a library is in a set of directories.

dnl Usage: OPJ_CHECK_LIB(header, lib, func [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])

AC_DEFUN([OPJ_CHECK_LIB],
[

m4_pushdef([UP], m4_toupper([$2]))

__opj_have_dep="no"

OPJ_CHECK_LIB_WITH_PREFIX([],
   [$1],
   [$2],
   [$3],
   [__opj_have_dep="yes"],
   [__opj_have_dep="no"])

if ! test "x${__opj_have_dep}" = "xyes" ; then
   OPJ_CHECK_LIB_WITH_PREFIX([/usr],
      [$1],
      [$2],
      [$3],
      [__opj_have_dep="yes"],
      [__opj_have_dep="no"])
fi

if ! test "x${__opj_have_dep}" = "xyes" ; then
   OPJ_CHECK_LIB_WITH_PREFIX([/usr/local],
      [$1],
      [$2],
      [$3],
      [__opj_have_dep="yes"],
      [__opj_have_dep="no"])
fi

if ! test "x${__opj_have_dep}" = "xyes" ; then
   OPJ_CHECK_LIB_WITH_PREFIX([/opt/local],
      [$1],
      [$2],
      [$3],
      [__opj_have_dep="yes"],
      [__opj_have_dep="no"])
fi

AC_ARG_VAR(UP[_CFLAGS], [preprocessor flags for lib$2])
AC_SUBST(UP[_CFLAGS])
AC_ARG_VAR(UP[_LIBS], [linker flags for lib$2])
AC_SUBST(UP[_LIBS])

AS_IF([test "x${__opj_have_dep}" = "xyes"], [$4], [$5])

m4_popdef([UP])

])
