z_c_args = []
z_link_args = []
inc_dirs = include_directories('.', '../include')

check_headers = [
  ['HAVE_STDINT_H', 'stdint.h'],
  ['HAVE_STDIO_H', 'stddef.h'],
  ['HAVE_SYS_TYPES_H', 'sys/types.h'],
  ['Z_HAVE_UNISTD_H', 'unistd.h'],
]

foreach h : check_headers
  if cc.has_header(h.get(1))
    z_c_args += ['-D'+h.get(0) + '=' + '1']
  endif
endforeach


if cc.sizeof('off64_t') != -1
  z_c_args += '-D_LARGEFILE64_SOURCE=1'
endif

if not cc.has_function('fseeko')
  z_c_args += '-DNO_FSEEKO'
endif

if cc.get_id() == 'gcc' and host_machine.system() == 'linux'
  vflag = '-Wl,--version-script,@0@/zlib.map'.format(meson.current_source_dir())
  z_link_args += [vflag]
endif

z_src = ['adler32.c', 'crc32.c', 'deflate.c', 'infback.c', 'inffast.c', 'inflate.c',
'inftrees.c', 'trees.c', 'zutil.c',
'compress.c', 'uncompr.c', 'gzclose.c', 'gzlib.c', 'gzread.c', 'gzwrite.c']

z_headers = ['../include/zconf.h', '../include/zlib.h']

zlib = static_library('z', z_src,
  c_args : z_c_args,
  link_args : z_link_args,
  include_directories : inc_dirs,
  install : true,
  version : meson.project_version())


zlib_dep = declare_dependency(link_with : zlib,
  include_directories : inc_dirs)

install_headers(z_headers)
