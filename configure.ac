
# According to http://www.gnu.org/software/libtool/manual/libtool.html#Updating-version-info :
#
# 1) when bugs are fixed or internal code is changed: increase MICRO
# 2) if API is added, increase MINOR and set MICRO to 0
# 3) if API or ABI is broken (this case should (must) never happen as it's very bad for a library), or a new designed library, increase MAJOR and set MINOR and MICRO to 0

m4_define([OPJ_MAJOR], [1])
m4_define([OPJ_MINOR], [5])
m4_define([OPJ_MICRO], [0])

m4_define([lt_cur], m4_eval(OPJ_MAJOR + OPJ_MINOR))
m4_define([lt_rev], OPJ_MICRO)
m4_define([lt_age], OPJ_MINOR)

AC_PREREQ([2.62])
AC_INIT([OpenJPEG],
   [OPJ_MAJOR.OPJ_MINOR.OPJ_MICRO],
   [openjpeg@googlegroups.com],
   [openjpeg],
   [http://www.openjpeg.org])
AC_CONFIG_SRCDIR([configure.ac])
AC_CONFIG_MACRO_DIR([m4])
AC_CONFIG_HEADERS([opj_config.h])

AC_CANONICAL_SYSTEM
AC_CANONICAL_HOST

AM_INIT_AUTOMAKE([1.11 foreign dist-bzip2 dist-xz dist-zip])
m4_ifdef([AM_SILENT_RULES], [AM_SILENT_RULES([yes])])

MAJOR_NR=OPJ_MAJOR
MINOR_NR=OPJ_MINOR
MICRO_NR=OPJ_MICRO
AC_SUBST([MAJOR_NR])
AC_SUBST([MINOR_NR])
AC_SUBST([MICRO_NR])

LT_PREREQ([2.0])
LT_INIT([win32-dll])

lt_version=lt_cur:lt_rev:lt_age
AC_SUBST([lt_version])


### Needed information

have_win32=no
have_darwin="no"
case "$host_os" in
   mingw*)
      have_win32="yes"
      ;;
   darwin*)
      have_darwin="yes"
      ;;
esac

AM_CONDITIONAL([HAVE_WIN32], [test "x${have_win32}" = "xyes"])
AM_CONDITIONAL([HAVE_DARWIN], [test "x${have_darwin}" = "xyes"])


### Additional options to configure

# debug

AC_ARG_ENABLE([debug],
   [AC_HELP_STRING([--enable-debug], [build with debug symbols @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_debug="yes"
    else
       want_debug="no"
    fi
   ],
   [want_debug="no"])

AC_MSG_CHECKING([whether to build in debug mode])
AC_MSG_RESULT([${want_debug}])

# MJ2

AC_ARG_ENABLE([mj2],
   [AC_HELP_STRING([--enable-mj2], [build mj2 binaries @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_mj2="yes"
    else
       want_mj2="no"
    fi
   ],
   [want_mj2="no"])

AC_MSG_CHECKING([whether to build the MJ2 binaries])
AC_MSG_RESULT([${want_mj2}])

if test "x${want_mj2}" = "xyes" ; then
   AC_DEFINE(USE_MJ2, [1], [define to 1 if you use mj2])
fi

AM_CONDITIONAL([WANT_MJ2], [test "x${want_mj2}" = "xyes"])

# JPWL

AC_ARG_ENABLE([jpwl],
   [AC_HELP_STRING([--enable-jpwl], [build JPWL library @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_jpwl="yes"
    else
       want_jpwl="no"
    fi
   ],
   [want_jpwl="no"])

AC_MSG_CHECKING([whether to build the JPWL library])
AC_MSG_RESULT([${want_jpwl}])

AM_CONDITIONAL([WANT_JPWL], [test "x${want_jpwl}" = "xyes"])

# JPIP

AC_ARG_ENABLE([jpip],
   [AC_HELP_STRING([--enable-jpip], [build jpip library @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_jpip="yes"
    else
       want_jpip="no"
    fi
   ],
   [want_jpip="no"])

AC_MSG_CHECKING([whether to build the JPIP library])
AC_MSG_RESULT([${want_jpip}])

AC_ARG_ENABLE([jpip-server],
   [AC_HELP_STRING([--enable-jpip-server], [build jpip server @<:@default=disabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_jpip_server="yes"
    else
       want_jpip_server="no"
    fi
   ],
   [want_jpip_server="no"])

AC_MSG_CHECKING([whether to build the JPIP server])
AC_MSG_RESULT([${want_jpip_server}])


### Checks for programs

AC_PROG_CC

# pkg-config

PKG_PROG_PKG_CONFIG

# Check whether pkg-config supports Requires.private
if $PKG_CONFIG --atleast-pkgconfig-version 0.22 ; then
   pkgconfig_requires_private="Requires.private"
else
   pkgconfig_requires_private="Requires"
fi
AC_SUBST([pkgconfig_requires_private])
AC_SUBST([requirements])

# doxygen

OPJ_CHECK_DOXYGEN

#  ln -s

AC_PROG_LN_S


### Checks for libraries

# libpng

have_libpng="no"

AC_ARG_ENABLE([png],
   [AC_HELP_STRING([--disable-png], [disable PNG support @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_png="yes"
    else
       want_png="no"
    fi
   ],
   [want_png="yes"])

AC_MSG_CHECKING([whether to build with PNG support])
AC_MSG_RESULT([${want_png}])

if test "x${want_png}" = "xyes" ; then

   PKG_CHECK_MODULES([PNG], [libpng15 zlib],
      [have_libpng="yes"],
      [
       PKG_CHECK_MODULES([PNG], [libpng14 zlib],
          [have_libpng="yes"],
          [
           PKG_CHECK_MODULES([PNG], [libpng12 zlib],
              [have_libpng="yes"],
              [
               PKG_CHECK_MODULES([PNG], [libpng zlib],
                  [have_libpng="yes"],
                  [have_libpng="no"])
              ])
          ])
      ])

   if ! test "x${have_libpng}" = "xyes" ; then

      OPJ_CHECK_LIB([zlib.h],
         [z],
         [zlibVersion],
         [
          OPJ_CHECK_LIB([png.h],
             [png],
             [png_access_version_number],
             [
              have_libpng="yes"
              PNG_CFLAGS="${PNG_CFLAGS} ${Z_CFLAGS}"
              PNG_LIBS="${PNG_LIBS} ${Z_LIBS}"
             ],
             [have_libpng="no"])
         ],
         [have_libpng="no"])

   fi

   if test "x${have_libpng}" = "xno" ; then
      AC_MSG_WARN([Can not find a usuable PNG library. Make sure that CPPFLAGS and LDFLAGS are correctly set.])
   fi

fi

AC_MSG_CHECKING([whether PNG is available])
AC_MSG_RESULT([${have_libpng}])

if test "x${have_libpng}" = "xyes" ; then
   AC_DEFINE(HAVE_LIBPNG, [1], [define to 1 if you have libpng])
fi

AM_CONDITIONAL([with_libpng], [test x${have_libpng} = "xyes"])

# libtiff

have_libtiff="no"

AC_ARG_ENABLE([tiff],
   [AC_HELP_STRING([--disable-tiff], [disable TIFF support @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_tiff="yes"
    else
       want_tiff="no"
    fi
   ],
   [want_tiff="yes"])

AC_MSG_CHECKING([whether to build with TIFF support])
AC_MSG_RESULT([${want_tiff}])

if test "x${want_tiff}" = "xyes" ; then

   OPJ_CHECK_LIB(
      [tiff.h],
      [tiff],
      [TIFFOpen],
      [have_libtiff="yes"],
      [have_libtiff="no"])

   if test "x${have_libtiff}" = "xno" ; then
      AC_MSG_WARN([Can not find a usuable TIFF library. Make sure that CPPFLAGS and LDFLAGS are correctly set.])
   fi
fi

AC_MSG_CHECKING([whether TIFF is available])
AC_MSG_RESULT([${have_libtiff}])

if test "x${have_libtiff}" = "xyes" ; then
   AC_DEFINE(HAVE_LIBTIFF, [1], [define to 1 if you have libtiff])
fi

AM_CONDITIONAL([with_libtiff], [test "x${have_libtiff}" = "xyes"])

# libcms2

lcms_output="no"
have_lcms2="no"

AC_ARG_ENABLE([lcms2],
   [AC_HELP_STRING([--disable-lcms2], [disable LCMS-2 support @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_lcms2="yes"
    else
       want_lcms2="no"
    fi
   ],
   [want_lcms2="yes"])

AC_MSG_CHECKING([whether to build with LCMS-2 support])
AC_MSG_RESULT([${want_lcms2}])

if test "x${want_lcms2}" = "xyes" ; then
   PKG_CHECK_MODULES([LCMS2], [lcms2],
      [have_lcms2="yes"],
      [have_lcms2="no"])
fi

AC_MSG_CHECKING([whether LCMS-2 is available])
AC_MSG_RESULT([${have_lcms2}])

if test "x${have_lcms2}" = "xyes" ; then
   AC_DEFINE(HAVE_LIBLCMS2, [1], [define to 1 if you have lcms version 2.x])
   lcms_output="lcms version 2.x"
fi

# libcms1

AC_ARG_ENABLE([lcms1],
   [AC_HELP_STRING([--disable-lcms1], [disable LCMS-1 support @<:@default=enabled@:>@])],
   [
    if test "x${enableval}" = "xyes" ; then
       want_lcms1="yes"
    else
       want_lcms1="no"
    fi
   ],
   [want_lcms1="yes"])

AC_MSG_CHECKING([whether to build with LCMS-1 support])
AC_MSG_RESULT([${want_lcms1}])

if test "x${have_lcms2}" = "xno" ; then

   if test "x${want_lcms1}" = "xyes" ; then
      PKG_CHECK_MODULES([LCMS1], [lcms1],
         [have_lcms1="yes"],
         [PKG_CHECK_MODULES([LCMS1], [lcms],
            [have_lcms1="yes"],
            [have_lcms1="no"])])
   fi

   AC_MSG_CHECKING([whether LCMS-1 is available])
   AC_MSG_RESULT([${have_lcms1}])

   if test "x${have_lcms1}" = "xyes" ; then
      AC_DEFINE(HAVE_LIBLCMS1, [1], [define to 1 if you have lcms version 1.x])
      lcms_output="lcms version 1.x"
   fi

fi

# threads

if test "x${want_jpip_server}" = "xyes" ; then

   if test "x${have_win32}" = "xno" ; then

      SAVE_CFLAGS=${CFLAGS}
      CFLAGS="${CFLAGS} -pthread"
      SAVE_LIBS=${LIBS}
      LIBS="${LIBS} -pthread"
      AC_LINK_IFELSE(
         [AC_LANG_PROGRAM([[
#include <pthread.h>
                          ]],
                          [[
pthread_t id;
id = pthread_self();
                          ]])],
         [have_pthread="yes"],
         [have_pthread="no"])
      CFLAGS=${SAVE_CFLAGS}
      LIBS=${SAVE_LIBS}

      AC_MSG_CHECKING([whether Pthread library is available])
      AC_MSG_RESULT([${have_pthread}])

      if ! test "x${have_pthread}" = "xyes" ; then
         AC_MSG_WARN([Pthread library not found. OpenJPIP server will not be compiled.])
         want_jpip_server="no"
      else
         THREAD_CFLAGS="-pthread"
         THREAD_LIBS="-pthread"
      fi

   else
      THREAD_LIBS="-lws2_32"
   fi

fi

AC_ARG_VAR([THREAD_CFLAGS], [compiler flag for the thread library])
AC_SUBST([THREAD_CFLAGS])
AC_ARG_VAR([THREAD_LIBS], [linker flags for thread library])
AC_SUBST([THREAD_LIBS])

# libfcgi

if test "x${want_jpip_server}" = "xyes" ; then

   have_fcgi="no"
   OPJ_CHECK_LIB(
      [fcgi_stdio.h],
      [fcgi],
      [FCGI_Accept],
      [have_fcgi="yes"],
      [have_fcgi="no"])

   if ! test "x${have_fcgi}" = "xyes" ; then
      AC_MSG_WARN([FastCGI library not found. OpenJPIP server will not be compiled.])
      want_jpip_server="no"
   fi

fi

# libcurl

if test "x${want_jpip_server}" = "xyes" ; then

   PKG_CHECK_MODULES([LIBCURL], [libcurl],
      [have_libcurl="yes"],
      [have_libcurl="no"])

   if ! test "x${have_libcurl}" = "xyes" ; then
      AC_MSG_WARN([libcurl library not found. OpenJPIP server will not be compiled.])
      want_jpip_server="no"
   fi

fi

if test "x${want_jpip}" = "xyes" ; then
   AC_DEFINE(USE_JPIP, [1], [define to 1 if you use jpip])
fi

AM_CONDITIONAL([WANT_JPIP], [test "x${want_jpip}" = "xyes"])

if test "x${want_jpip_server}" = "xyes" ; then
   AC_DEFINE(USE_JPIP_SERVER, [1], [define to 1 if you use jpip server])
fi

AM_CONDITIONAL([WANT_JPIP_SERVER], [test "x${want_jpip_server}" = "xyes"])

AM_CONDITIONAL([WANT_JPIP_CODE], [test "x${want_jpip}" = "xyes" || test "x${want_jpip_server}" = "xyes"])

### Checks for header files

## FIXME: declarations must be fixed in source code. See autoconf manual
AC_HEADER_DIRENT


### Checks for types


### Checks for structures


### Checks for compiler characteristics

AM_PROG_CC_C_O
AC_C_BIGENDIAN

#OPJ_COMPILER_FLAG([-Wall])
#OPJ_COMPILER_FLAG([-Wextra])
#OPJ_COMPILER_FLAG([-Wshadow])
#OPJ_COMPILER_FLAG([-Wpointer-arith])
OPJ_COMPILER_FLAG([-Wno-unused-result])

if test "x${want_debug}" = "xyes" ; then
   OPJ_COMPILER_FLAG([-g])
   OPJ_COMPILER_FLAG([-O0])
else
   OPJ_COMPILER_FLAG([-O3])
fi


### Checks for linker characteristics


### Checks for library functions


### Post configuration

AM_CONDITIONAL([BUILD_SHARED], [test "x${enable_shared}" = "xyes"])
AM_CONDITIONAL([BUILD_STATIC], [test "x${enable_static}" = "xyes"])


AC_CONFIG_FILES([
Makefile
libopenjpeg1.pc
libopenjpeg-jpwl.pc
libopenjpeg/Makefile
libopenjpeg/jpwl/Makefile
applications/Makefile
applications/codec/Makefile
applications/mj2/Makefile
applications/jpip/Makefile
applications/jpip/libopenjpip/Makefile
applications/jpip/util/Makefile
doc/Makefile
])

AC_OUTPUT

### Summary

echo
echo
echo
echo "------------------------------------------------------------------------"
echo "$PACKAGE $VERSION"
echo "------------------------------------------------------------------------"
echo
echo
echo "Configuration Options Summary:"
echo
echo "  Debug...............: ${want_debug}"
echo
echo "  Optional support:"
echo "    libpng............: ${have_libpng}"
echo "    libtiff...........: ${have_libtiff}"
echo "    libcms............: ${lcms_output}"
echo
echo "  Documentation.......: ${opj_have_doxygen}"
echo "    Build.............: make doc"
echo
echo "  mj2.................: ${want_mj2}"
echo "  jpwl................: ${want_jpwl}"
echo "  jpip................: ${want_jpip}"
echo "  jpip server.........: ${want_jpip_server}"
echo
