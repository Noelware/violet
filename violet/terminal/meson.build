libviolet_terminal_srcs = files('../../src/support/terminal.cc')

if host_machine.system() == 'linux' or host_machine.system() == 'darwin'
    libviolet_terminal_srcs += files('../../src/support/platform/terminal/unix.cc')
elif host_machine.system() == 'windows'
    libviolet_terminal_srcs += files('../../src/support/platform/terminal/windows.cc')
else
    libviolet_terminal_srcs += files('../../src/support/platform/terminal/unsupported.cc')
endif

violet_terminal_lib = library(
    'violet_terminal',
    libviolet_terminal_srcs,
    include_directories: violet_include_dir,
    dependencies: [violet_dep, violet_io_dep],
    cpp_args: violet_build_defines,
    install: true,
)

violet_terminal_dep = declare_dependency(
    include_directories: violet_include_dir,
    link_with: violet_terminal_lib,
    dependencies: [violet_dep, violet_io_dep],
    compile_args: violet_consumer_defines,
)

configure_file(
    input: '../../hack/pkg-config/libviolet_terminal.pc',
    output: 'libviolet_terminal.pc',
    configuration: conf,
    install: true,
    install_dir: get_option('prefix') / 'lib' / 'pkgconfig',
)
