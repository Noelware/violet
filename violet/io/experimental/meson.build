libviolet_io_experimental_srcs = files(
    '../../../src/io/experimental/input/buffered.cc',
    '../../../src/io/experimental/input/bytearray.cc',
    '../../../src/io/experimental/input/file.cc',
    '../../../src/io/experimental/input/stdin.cc',
    '../../../src/io/experimental/input/string.cc',

    '../../../src/io/experimental/output/buffered.cc',
    '../../../src/io/experimental/output/bytearray.cc',
    '../../../src/io/experimental/output/file.cc',
    '../../../src/io/experimental/output/stderr.cc',
    '../../../src/io/experimental/output/stdout.cc',
    '../../../src/io/experimental/output/string.cc',
)

if host_machine.system() == 'linux' or host_machine.system() == 'darwin'
    libviolet_io_experimental_srcs += files(
        '../../../src/io/experimental/input/file/unix.cc',
        '../../../src/io/experimental/input/stdin/unix.cc',
        '../../../src/io/experimental/output/stderr/unix.cc',
        '../../../src/io/experimental/output/stdout/unix.cc',
    )
elif host_machine.system() == 'windows'
    libviolet_io_experimental_srcs += files(
        '../../../src/io/experimental/input/file/windows.cc',
        '../../../src/io/experimental/input/stdin/windows.cc',
        '../../../src/io/experimental/output/stderr/windows.cc',
        '../../../src/io/experimental/output/stdout/windows.cc',
    )
else
    libviolet_io_experimental_srcs += files(
        '../../../src/io/experimental/input/file/unsupported.cc',
        '../../../src/io/experimental/input/stdin/unsupported.cc',
        '../../../src/io/experimental/output/stderr/unsupported.cc',
        '../../../src/io/experimental/output/stdout/unsupported.cc',
    )
endif

violet_io_experimental_lib = library(
    'violet_io_experimental',
    libviolet_io_experimental_srcs,
    include_directories: violet_include_dir,
    dependencies: [violet_io_dep, violet_filesystem_dep],
    cpp_args: violet_build_defines,
    install: true,
)

violet_io_experimental_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: violet_io_experimental_lib,
    dependencies: [violet_io_dep, violet_filesystem_dep],
    compile_args: violet_consumer_defines,
)

configure_file(
    input: '../../../hack/pkg-config/libviolet_io_experimental.pc',
    output: 'libviolet_io_experimental.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig',
)
