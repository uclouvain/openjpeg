#ifndef _FLVIEWER_CONFIG_H_
#define _FLVIEWER_CONFIG_H_

#define PACKAGE_VERSION "@FLVIEWER_VERSION@"
#define PACKAGE_STRING "@FLVIEWER_PACKAGE_STRING@"

#cmakedefine WITH_ENGLISH
#cmakedefine WITH_GERMAN

#cmakedefine HAVE_THREADS
#cmakedefine HAVE_WINPORT

#cmakedefine HAVE_FSEEKI64 @HAVE_FSEEKI64@
#cmakedefine HAVE_FSEEKO

#cmakedefine _LARGEFILE_SOURCE
#cmakedefine _FILE_OFFSET_BITS @_FILE_OFFSET_BITS@

#if !defined(__APPLE__)
#cmakedefine OPJ_BIG_ENDIAN
#elif defined(__BIG_ENDIAN__)
#cmakedefine OPJ_BIG_ENDIAN
#endif
#
#endif /* _FLVIEWER_CONFIG_H_ */
