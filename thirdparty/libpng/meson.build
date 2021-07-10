libpng_srcs = ['png.c',
              'pngmem.c',
              'pngrio.c',
               'pngset.c',
               'pngwrite.c',
               'pngerror.c',
               'pngpread.c',
               'pngrtran.c',
               'pngtrans.c',
               'pngwtran.c',
               'pngget.c',
               'pngread.c',
               'pngrutil.c',
               'pngwio.c',
               'pngwutil.c'
               ]

libpng_headers = [ 'pngconf.h'
                   , 'pngdebug.h'
                   , 'png.h'
                   , 'pnginfo.h'
                   , 'pnglibconf.h'
                   , 'pngpriv.h'
                   , 'pngstruct.h'
                 ]


inc_dirs = include_directories('.')

ext_hdrs = ['/thirdparty/include/zlib.h', '/thirdparty/include/zconf.h']

libpng_lib = static_library('png', libpng_srcs
                            , include_directories : inc_dirs
                            , dependencies : [zlib_dep]
                            )

libpng_dep = declare_dependency(link_with : libpng_lib,
              include_directories : inc_dirs)

