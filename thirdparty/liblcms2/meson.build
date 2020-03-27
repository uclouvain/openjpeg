lcms2_srcs = [ 'src/cmsalpha.c'
        , 'src/cmscgats.c'
        , 'src/cmserr.c'
        , 'src/cmsgmt.c'
        , 'src/cmsintrp.c'
        , 'src/cmsio1.c'
        , 'src/cmsmd5.c'
        , 'src/cmsnamed.c'
        , 'src/cmspack.c'
        , 'src/cmsplugin.c'
        , 'src/cmssamp.c'
        , 'src/cmstypes.c'
        , 'src/cmswtpnt.c'
        , 'src/cmscam02.c'
        , 'src/cmscnvrt.c'
        , 'src/cmsgamma.c'
        , 'src/cmshalf.c'
        , 'src/cmsio0.c'
        , 'src/cmslut.c'
        , 'src/cmsmtrx.c'
        , 'src/cmsopt.c'
        , 'src/cmspcs.c'
        , 'src/cmsps2.c'
        , 'src/cmssm.c'
        , 'src/cmsvirt.c'
        , 'src/cmsxform.c'
]


inc_dirs = include_directories('include')

liblcms2_lib = static_library('lcms2', lcms2_srcs,
                          include_directories : inc_dirs
                          )
liblcms2_dep = declare_dependency(link_with : liblcms2_lib,
          include_directories : inc_dirs)
