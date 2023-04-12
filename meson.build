project('libusrio', 'c',
        license : 'GPLv2',
        default_options: ['c_std=c99'])

cc = meson.get_compiler('c')
cflags = cc.get_supported_arguments([
    '-Wall',
    '-Werror=incompatible-pointer-types',
    '-Werror=implicit-function-declaration',
    '-Werror=format-security'
])

inc = include_directories('include')

libdgpc_proj = subproject('libdgpc')
libdpgc_dep = libdgpc_proj.get_variable('libdgpc_dep')
log_macros_tmpl = libdgpc_proj.get_variable('log_macros_tmpl')

subdir('include')
subdir('src')

libusrio_dep = declare_dependency(
    include_directories : inc,
    link_with : libusrio)

pkg_mod = import('pkgconfig')
pkg_mod.generate(
    libraries : libusrio,
    version : '0',
    name : 'libusrio',
    filebase : 'libusrio',
    description : 'Linux i2c/spi userspace interfaces in userspace')
