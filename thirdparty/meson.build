cappdata = configuration_data()

# 3rd party libs
#------------
# Try to find lib Z
if get_option('build_thirdparty')
  message('STATUS: We will build Z lib from thirdparty')
  subdir('libz')
  message('STATUS: We will build PNG lib from thirdparty')
  cappdata.set('OPJ_HAVE_LIBPNG', 1)
  cappdata.set('OPJ_HAVE_PNG_H', 1)
  subdir('libpng')
  message('STATUS: We will build TIFF lib from thirdparty')
  cappdata.set('OPJ_HAVE_LIBTIFF', 1)
  cappdata.set('OPJ_HAVE_TIFF_H', 1)
  subdir('libtiff')
  message('STATUS: We will build lcms2 lib from thirdparty')
  cappdata.set('OPJ_HAVE_LIBLCMS2', 1)
  cappdata.set('OPJ_HAVE_LCMS2_H', 1)
  subdir('liblcms2')
else
  zlib_dep = dependency('zlib', required : false)
  if not zlib_dep.found()
    if cc.get_id() != 'msvc' and cc.get_id() != 'clang-cl'
      zlib_dep = cc.find_library('z', required : false)
    else
      zlib_dep = cc.find_library('zlib1', required : false)
      if not zlib_dep.found()
        zlib_dep = cc.find_library('zlib', required : false)
      endif
    endif
    if not zlib_dep.found() or not cc.has_header('zlib.h')
      zlib_dep = subproject('zlib').get_variable('zlib_dep')
    endif
    # fallback on third party folder
    if  zlib_dep.found()
      message('STATUS: Your system seems to have a Z lib available, we will use it to generate Z lib')
    else
      message('STATUS: We will build Z lib from thirdparty folder')
      subdir('libz')
    endif
  endif

  # Try to find libpng first
  cappdata.set('OPJ_HAVE_LIBPNG', 1)
  cappdata.set('OPJ_HAVE_PNG_H', 1)
  libpng_dep = dependency('libpng', required : false)
  if libpng_dep.found()
    message('STATUS: Your system seems to have a png lib available, we will use it to generate PNG lib')
  else
    subdir('libpng')
    message('STATUS: libpng not found, We will build libpng from thirdparty folder')
  endif

  # Try to find libtiff first
  cappdata.set('OPJ_HAVE_LIBTIFF', 1)
  cappdata.set('OPJ_HAVE_TIFF_H', 1)
  libtiff_dep = dependency('libtiff', required : false)
  if libtiff_dep.found()
    message('STATUS: Your system seems to have a tiff lib available, we will use it instead of thirdparty')
  else
    subdir('libtiff')
    message('STATUS: tiff not found, We will build libtiff from thirdparty folder')
  endif

  # Try to find lcms2 first
  cappdata.set('OPJ_HAVE_LIBLCMS2', 1)
  cappdata.set('OPJ_HAVE_LCMS2_H', 1)
  liblcms2_dep = dependency('lcms2', required : false)
  if liblcms2_dep.found()
    message('STATUS: Your system seems to have a lcms2 lib available, we will use it instead of thirdparty folder')
  else
    subdir('liblcms2')
    message('STATUS: tiff not found, We will build libtiff from thirdparty folder')
  endif
endif

#------------TODO
#if get_option('WITH_ASTYLE')
#  subdir('astyle')
#endif

