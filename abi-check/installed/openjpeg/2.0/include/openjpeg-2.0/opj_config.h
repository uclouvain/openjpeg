/* create config.h for CMake */
#define OPJ_PACKAGE_VERSION "2.0.0"

#define HAVE_INTTYPES_H 1
#define HAVE_MEMORY_H 1
#define HAVE_STDINT_H 1
#define HAVE_STDLIB_H 1
#define HAVE_STRINGS_H 1
#define HAVE_STRING_H 1
#define HAVE_SYS_STAT_H 1
#define HAVE_SYS_TYPES_H 1
#define HAVE_UNISTD_H 1
#define HAVE_LIBPNG 1
#define HAVE_PNG_H 1
#define HAVE_LIBTIFF 1
#define HAVE_TIFF_H 1
#define HAVE_SSIZE_T TRUE

/* #undef _LARGEFILE_SOURCE */
/* #undef _LARGE_FILES */
/* #undef _FILE_OFFSET_BITS */
#define HAVE_FSEEKO ON

/* #undef HAVE_LIBLCMS1 */
#define HAVE_LIBLCMS2
/* #undef HAVE_LCMS1_H */
#define HAVE_LCMS2_H

/* Byte order.  */
/* All compilers that support Mac OS X define either __BIG_ENDIAN__ or
__LITTLE_ENDIAN__ to match the endianness of the architecture being
compiled for. This is not necessarily the same as the architecture of the
machine doing the building. In order to support Universal Binaries on
Mac OS X, we prefer those defines to decide the endianness.
On other platforms we use the result of the TRY_RUN. */
#if !defined(__APPLE__)
/* #undef OPJ_BIG_ENDIAN */
#elif defined(__BIG_ENDIAN__)
# define OPJ_BIG_ENDIAN
#endif

