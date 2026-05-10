libviolet_io_srcs = files(
    '../../src/io/error.cc',
    '../../src/io/read.cc',
    '../../src/io/write.cc'
)

if host_machine.system() == 'linux' or host_machine.system() == 'darwin'
    libviolet_io_srcs += files(
        '../../src/io/platform/descriptor/unix.cc',
        '../../src/io/platform/error/unix.cc',
        '../../src/system/platform/unix.cc'
    )
elif host_machine.system() == 'windows'
    libviolet_io_srcs += files(
        '../../src/io/platform/descriptor/windows.cc',
        '../../src/io/platform/error/windows.cc',
        '../../src/system/platform/windows.cc'
    )
else
    libviolet_io_srcs += files(
        '../../src/io/platform/descriptor/unsupported.cc',
        '../../src/io/platform/error/unsupported.cc',
        '../../src/system/platform/unsupported.cc'
    )
endif

violet_io_lib = library(
    'violet_io',
    libviolet_io_srcs,
    include_directories: violet_include_dir,
    cpp_args: violet_build_defines,
    install: true
)

violet_io_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: violet_io_lib,
    dependencies: [violet_dep],
    compile_args: violet_consumer_defines
)

configure_file(
    input: '../../hack/pkg-config/libviolet_io.pc',
    output: 'libviolet_io.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig'
)
