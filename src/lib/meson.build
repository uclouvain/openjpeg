# source code for openjpeg project:
# Part 1 & 2:
subdir('openjp2')
# optionals components:
if get_option('build_jpwl')
  subdir('openjpwl')
  if get_option('build_mj2')
    subdir('openmj2')
  endif
else
  if get_option('build_mj2')
    message('WARNING: you need to enable JPWL to get MJ2')
  endif
endif

if get_option('build_jpip')
  subdir('openjpip')
endif
if  get_option('build_jp3d')
  subdir('openjp3d')
endif
