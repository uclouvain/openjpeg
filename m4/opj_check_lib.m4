dnl Copyright (C) 2011 Vincent Torri <vtorri at univ-evry dot fr>
dnl That code is public domain and can be freely used or copied.

dnl Macro that check if a library is in a specified directory.

dnl Usage: OPJ_CHECK_LIB(prefix, header, lib, func [, ACTION-IF-FOUND [, ACTION-IF-NOT-FOUND]])
dnl Call AC_SUBST(THELIB_CFLAGS)
dnl Call AC_SUBST(THELIB_LIBS)
dnl where THELIB is the uppercase value of 'lib'

AC_DEFUN([OPJ_CHECK_LIB],
[

m4_pushdef([UP], m4_toupper([$3]))
m4_pushdef([DOWN], m4_tolower([$3]))

__opj_prefix=$1
__opj_header=$2
__opj_lib=$3
__opj_func=$4
__opj_have_dep="no"

save_CPPFLAGS=${CPPFLAGS}
CPPFLAGS="${CPPFLAGS} -I${__opj_prefix}/include"
AC_CHECK_HEADER([${__opj_header}], [__opj_have_dep="yes"], [__opj_have_dep="no"])
CPPFLAGS=${save_CPPFLAGS}

if test "x${__opj_have_dep}" = "xyes" ; then
   save_LDFLAGS=${LDFLAGS}
   LDFLAGS="${LDFLAGS} -L${__opj_prefix}/lib"
   AC_CHECK_LIB([${__opj_lib}],
      [${__opj_func}],
      [__opj_have_dep="yes"],
      [__opj_have_dep="no"])
   LDFLAGS=${save_LDFLAGS}
fi

if test "x${__opj_have_dep}" = "xyes" ; then
   if ! test "x$1" = "x/usr" ; then
      UP[]_CFLAGS="-I${__opj_prefix}/include"
      UP[]_LIBS="-L${__opj_prefix}/lib"
   fi
   UP[]_LIBS="${UP[]_LIBS} -l${__opj_lib}"
fi

AC_ARG_VAR(UP[_CFLAGS], [preprocessor flags for lib$3])
AC_SUBST(UP[_CFLAGS])
AC_ARG_VAR(UP[_LIBS], [linker flags for lib$3])
AC_SUBST(UP[_LIBS])

AS_IF([test "x${__opj_have_dep}" = "xyes"], [$5], [$6])

m4_popdef([UP])
m4_popdef([DOWN])

])
