libtiff_srcs = ['tif_aux.c'
                , 'tif_close.c'
                , 'tif_codec.c'
                , 'tif_color.c'
                , 'tif_compress.c'
                , 'tif_dir.c'
                , 'tif_dirinfo.c'
                , 'tif_dirread.c'
                , 'tif_dirwrite.c'
                , 'tif_dumpmode.c'
                , 'tif_error.c'
                , 'tif_extension.c'
                , 'tif_fax3.c'
                ,  'tif_fax3sm.c'
                , 'tif_flush.c'
                , 'tif_getimage.c'
                , 'tif_jbig.c'
                , 'tif_jpeg.c'
                , 'tif_luv.c'
                , 'tif_lzw.c'
                , 'tif_next.c'
                , 'tif_ojpeg.c'
                , 'tif_open.c'
                , 'tif_packbits.c'
                , 'tif_pixarlog.c'
                , 'tif_predict.c'
                ,  'tif_print.c'
                , 'tif_read.c'
                , 'tif_strip.c'
                , 'tif_swab.c'
                , 'tif_thunder.c'
                , 'tif_tile.c'
                , 'tif_version.c'
                , 'tif_warning.c'
                , 'tif_write.c'
                , 'tif_zip.c'
                , 'tif_jpeg_12.c'
                , 'tif_lzma.c']

libtiff_hdrs = ['t4.h', 'tiffiop.h', 'tif_dir.h', 'tif_fax3.h', 'tif_predict.h',  'uvcode.h']


inc_dirs = include_directories('.')
c_tif_config_data = configuration_data()
c_tiffconf_data = configuration_data()

libtiff_c_args = []

if host_system == 'linux'
  libtiff_srcs +=  ['tif_unix.c']
  libtiff_c_args += ['-D_FILE_OFFSET_BITS=64']
  # Large file support
  # This might not catch every possibility catered for by
  # AC_SYS_LARGEFILE.
  c_tif_config_data.set('_FILE_OFFSET_BITS',64)
else
  libtiff_srcs +=  ['tif_win32.c']
endif

# test_big_endian('WORDS_BIGENDIAN')
# host_bigendian = words_bigendian
if host_machine.endian() == 'big'
  c_tiffconf_data.set('HOST_FILLORDER', 'FILLORDER_MSB2LSB')
else
  c_tiffconf_data.set('HOST_FILLORDER', 'FILLORDER_LSB2MSB')

endif

libjpeg_dep = dependency('libjpeg')
c_tiffconf_data.set('JPEG_SUPPORT', 0)
if libjpeg_dep.found()
  c_tiffconf_data.set('JPEG_SUPPORT', 1)
endif

c_tiffconf_data.set('ZIP_SUPPORT', 0)
if zlib_dep.found()
  c_tiffconf_data.set('ZIP_SUPPORT', 1)
  c_tiffconf_data.set('PIXARLOG_SUPPORT', 1)
endif


check_headers = [
  ['HAVE_ASSERT_H', 'assert.h'],
  ['HAVE_CTYPE_H', 'ctype.h'],
  ['HAVE_DLFCN_H', 'dlfcn.h'],
  ['HAVE_FCNTL_H', 'fcntl.h'],
  ['HAVE_FLOAT_H', 'float.h'],
  ['HAVE_INTTYPES_H', 'inttypes.h'],
  ['HAVE_IEEEFP_H', 'ieeefp.h'],
  ['HAVE_IO_H', 'io.h'],
  ['HAVE_LIMITS_H', 'limits.h'],
  ['HAVE_MALLOC_H', 'malloc.h'],
  ['HAVE_MATH_H', 'math.h'],
  ['HAVE_MEMORY_H', 'memory.h'],
  ['HAVE_SEARCH_H', 'search.h'],
  ['HAVE_STDARG_H', 'stdarg.h'],
  ['HAVE_STDINT_H', 'stdint.h'],
  ['HAVE_STDLIB_H', 'stdlib.h'],
  ['HAVE_STDIO_H', 'stdio.h'],
  ['HAVE_STRING_H', 'string.h'],
  ['HAVE_STRINGS_H', 'strings.h'],
  ['HAVE_SYS_STAT_H', 'sys/stat.h'],
  ['HAVE_SYS_TIME_H', 'sys/time.h'],
  ['HAVE_SYS_TYPES_H', 'sys/types.h'],
  ['HAVE_TIME_H', 'time.h'],
  ['HAVE_UNISTD_H', 'unistd.h'],
  ['HAVE_WINDOWS_H', 'windows.h'],
  ['HAVE_WINSOCK2_H', 'winsock2.h']
]

foreach h : check_headers
  if cc.has_header(h.get(1))
    c_tif_config_data.set(h.get(0), 1)
  endif
endforeach




# wotsit ?
if  c_tif_config_data.get('HAVE_TIME_H') == 1 and c_tif_config_data.get('HAVE_SYS_TIME_H') == 1
  c_tiffconf_data.set('TIME_WITH_SYS_TIME', 1)
endif

c_tiffconf_data.set('LZMA_SUPPORT', 0)
c_tiffconf_data.set('MDI_SUPPORT', 1)
c_tiffconf_data.set('STDC_HEADERS', 1)

# why not ?
c_tiffconf_data.set('DEFAULT_EXTRASAMPLE_AS_ALPHA', 1)
c_tif_config_data.set('CHECK_JPEG_YCBCR_SUBSAMPLING', 1)
c_tif_config_data.set('CCITT_SUPPORT', 1)

c_tiffconf_data.set('DEFER_STRILE_LOAD', 0)
c_tiffconf_data.set('HAVE_JBG_NEWLEN', 0)
# FIXME: jbigkit stuff
c_tiffconf_data.set('STRIPCHOP_DEFAULT', 'TIFF_STRIPCHOP')
c_tif_config_data.set('STRIP_SIZE_DEFAULT', 8192)
c_tiffconf_data.set('SUBIFD_SUPPORT', 1)
c_tiffconf_data.set('THUNDER_SUPPORT', 1)

check_functions = [
  ['getopt', 'HAVE_GETOPT'],
  ['isascii', 'HAVE_ISASCII'],
  ['memmove', 'HAVE_MEMMOVE'],
  ['memset', 'HAVE_MEMSET'],
  ['mmap', 'HAVE_MMAP'],
  ['strcasecmp', 'HAVE_STRCASECMP'],
  ['strchr', 'HAVE_STRCHR'],
  ['strrchr', 'HAVE_STRRCHR'],
  ['strstr', 'HAVE_STRSTR'],
  ['strtol', 'HAVE_STRTOL'],
  ['strtoul', 'HAVE_STRTOUL'],
  ['strtoull', 'HAVE_STRTOULL'],
]
foreach h : check_functions
  if cc.has_function(h.get(0))
    c_tif_config_data.set(h.get(1), 1)
  else
    c_tif_config_data.set(h.get(1), 0)
  endif
endforeach



if cc.has_function('snprintf') or cc.has_header_symbol('stdio.h', 'snprintf')
  c_tif_config_data.set('HAVE_SNPRINTF', 1)
else
  libtiff_srcs += ['snprintf.c']
endif

c_tif_config_data.set('SIZEOF_SIGNED_INT', cc.sizeof('signed int'))
c_tif_config_data.set('SIZEOF_SIGNED_LONG', cc.sizeof('signed long'))
c_tif_config_data.set('SIZEOF_SIGNED_LONG_LONG', cc.sizeof('signed long long'))
c_tif_config_data.set('SIZEOF_SIGNED_SHORT', cc.sizeof('signed short'))
c_tif_config_data.set('SIZEOF_UNSIGNED_INT', cc.sizeof('unsigned int'))
c_tif_config_data.set('SIZEOF_UNSIGNED_LONG', cc.sizeof('unsigned long'))
c_tif_config_data.set('SIZEOF_UNSIGNED_LONG_LONG', cc.sizeof('unsigned long long'))
c_tif_config_data.set('SIZEOF_UNSIGNED_SHORT', cc.sizeof('unsigned short'))
c_tif_config_data.set('SIZEOF_UNSIGNED_CHAR_P', cc.sizeof('unsigned char*'))

if cc.sizeof('ssize_t') != -1
  c_tiffconf_data.set('HAVE_SSIZE_T', 1)
endif


if c_tif_config_data.get('HAVE_STDINT_H') == 1
  c_tiffconf_data.set('TIFF_INT8_T', 'int8_t')
  c_tiffconf_data.set('TIFF_INT16_T', 'int16_t')
  c_tiffconf_data.set('TIFF_INT32_T', 'int32_t')
  c_tiffconf_data.set('TIFF_INT64_T', 'int64_t')
  c_tiffconf_data.set('TIFF_UINT8_T', 'uint8_t')
  c_tif_config_data.set('TIFF_UINT8_T', 'uint8_t')
  c_tiffconf_data.set('TIFF_UINT16_T', 'uint16_t')
  c_tiffconf_data.set('TIFF_UINT32_T', 'uint32_t')
  c_tiffconf_data.set('TIFF_UINT64_T', 'uint64_t')
  c_tiffconf_data.set('TIFF_PTRDIFF_T', 'ptrdiff_t')

  if not c_tiffconf_data.has('HAVE_SSIZE_T')
    voidp_size = cc.sizeof('void*')
    if voidp_size == 8
      c_tiffconf_data.set('TIFF_SSIZE_T', 'int64_t')
      c_tif_config_data.set('TIFF_SSIZE_FORMAT', '%lld')
      c_tif_config_data.set('TIFF_SIZE_FORMAT', '%llu')
    elif voidp_size == 4
      c_tiffconf_data.set('TIFF_SSIZE_T', 'int32_t')
      c_tif_config_data.set('TIFF_SSIZE_FORMAT', '%d')
      c_tif_config_data.set('TIFF_SIZE_FORMAT', '%u')
    else
      error('FATAL_ERROR: unknown ssize_t')
    endif
  else
    c_tif_config_data.set('TIFF_SSIZE_FORMAT', '%ld')
    c_tiffconf_data.set('TIFF_SSIZE_T', 'ssize_t')
  endif

  c_tif_config_data.set('TIFF_INT32_FORMAT', '%d')
  c_tif_config_data.set('TIFF_UINT32_FORMAT', '%u')
  c_tif_config_data.set('TIFF_INT64_FORMAT', '%lld')
  c_tif_config_data.set('TIFF_UINT64_FORMAT', '%lu')
  c_tif_config_data.set('TIFF_PTRDIFF_FORMAT', '%ld')
else
  c_tiffconf_data.set('TIFF_INT8_T', 'signed __int8')
  c_tiffconf_data.set('TIFF_INT16_T', 'signed __int16')
  c_tiffconf_data.set('TIFF_INT32_T', 'signed __int32')
  c_tiffconf_data.set('TIFF_INT64_T', 'signed __int64')
  c_tif_config_data.set('TIFF_UINT8_T', 'unsigned __int8')
  c_tiffconf_data.set('TIFF_UINT8_T', 'unsigned __int8')
  c_tiffconf_data.set('TIFF_UINT16_T', 'unsigned __int16')
  c_tiffconf_data.set('TIFF_UINT32_T', 'unsigned __int32')
  c_tiffconf_data.set('TIFF_UINT64_T', 'unsigned __int64')
  c_tiffconf_data.set('TIFF_PTRDIFF_T', 'ptrdiff_t')

  if not c_tiffconf_data.has('HAVE_SSIZE_T')
    voidp_size = cc.sizeof('void*')
    if voidp_size == 8
      c_tiffconf_data.set('TIFF_SSIZE_T', 'signed __int64')
      c_tiffconf_data.set('TIFF_SSIZE_FORMAT', '%lld')
      c_tiffconf_data.set('TIFF_SIZE_FORMAT', '%llu')
    elif voidp_size == 4
      c_tiffconf_data.set('TIFF_SSIZE_T', 'signed __int32')
      c_tiffconf_data.set('TIFF_SSIZE_FORMAT', '%d')
      c_tiffconf_data.set('TIFF_SIZE_FORMAT', '%u')
    else
      # message(['FATAL_ERROR', 'unknown ssize_t'])
    endif
  else
    c_tiffconf_data.set('TIFF_SSIZE_T', 'ssize_t')
  endif
  c_tif_config_data.set('TIFF_INT32_FORMAT', '%d')
  c_tif_config_data.set('TIFF_UINT32_FORMAT', '%u')
  c_tif_config_data.set('TIFF_INT64_FORMAT', '%lld')
  c_tif_config_data.set('TIFF_UINT64_FORMAT', '%llu')
  c_tif_config_data.set('TIFF_PTRDIFF_FORMAT', '%ld')
endif
#

version = '"4.0.6"'
c_tif_config_data.set('VERSION', version)
c_tif_config_data.set('PACKAGE_VERSION', version)
c_tif_config_data.set('PACKAGE', '"tiff"')

# universal build ?
if host_system == 'darwin'
  ac_apple_universal_build = '1'
endif
if host_system == 'linux'
  cmake_required_libraries = 'm'
  have_libm = '1'
endif


check_funcs = [
  ['floor', 'HAVE_FLOOR', ''],
  ['sqrt', 'HAVE_SQRT', ''],
  ['pow', 'HAVE_POW', ''],
  ['lfind', 'HAVE_LFIND', ''],
  ['setmod', 'HAVE_SETMODE', '']
]

foreach h : check_funcs
  if cc.has_function(h.get(0))
    c_tif_config_data.set(h.get(1), 1)
  endif
endforeach


code = '''
typedef int foo_t;
static inline foo_t static_foo(){return 0;}
foo_t foo(){return 0;}
int main(int argc, char *argv[]){return 0;}
'''
c_inline = false
foreach keyword : ['inline', '__inline__', '__inline']
  c_args = '-Dinline=' + keyword
  if not c_inline
    result = cc.compiles(code, name : 'basic inline', args : c_args)
    if result
      c_tif_config_data.set('INLINE_KEYWORD', keyword)
      c_inline = true
    endif
  endif
endforeach

configure_file(input: 'tif_config.h.cmake.in', output : 'tif_config.h', configuration : c_tif_config_data, format : 'cmake@')
configure_file(input: 'tiffconf.h.cmake.in', output : 'tiffconf.h', configuration : c_tiffconf_data)

libtiff_lib = static_library('libtiff', libtiff_srcs)

libtiff_dep = declare_dependency(link_with : libtiff_lib,
              include_directories : inc_dirs)
