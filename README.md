### 🌺💜 Violet
### *Extended C++ standard library*

**Violet** is a C++20 and higher library that extends the current C++ standard library with enriching types coming from a Rust developer and cross-platform libraries that are readily avaliable.

> [!IMPORTANT]
> **Violet** is 100% experimental software and may not work as intended! Please report bugs via [GitHub Issues](https://github.com/Noelware/Violet/issues/new)! **Violet** is also NOT production ready either! things will break!!!
>
> ---
>
> **Violet** (and other frameworks) uses a rather unique versioning scheme: `year.month.revision[-channel[.patch]]`:
>
> * `25.09`
> * `25.09.01`
> * `25.09.01-dev`
> * `25.09.01-dev.1`
>
> We decided on this because it is very easy to see when a version of the library was released and doesn't
> enforce "breaking/change" semantics like [SemVer](https://semver.org), we depend on release timing for
> the scope of the library.

## Building
**Violet** aims to be buildable on most popular C++ build systems, which include:

* Bazel 7.6 or higher
* CMake 3.25 or higher
* Meson 1.1 or higher

and the following compilers:

* Clang 20 or higher
* GCC 14 or higher

**Violet** also supports C++ 20 or higher.

### Bazel
Using **Violet** in Bazel is the most trivial and most supported build system as we use Bazel extensively at Noelware. We only support Bzlmod and not the legacy WORKSPACE system.

We also aim to have Violet avaliable on BCR soon.

```python
bazel_dep(name = "violet", version = "26.01") # you can pin `version` to what .violet-version says
git_override(
    module_name = "violet",
    remote = "https://github.com/Noelware/violet.git",
    commit = "<pinned commit here>"
)
```

#### Flags
> [!NOTE]
> All flags can be aliased using `@violet//:[flag-name]` like using `--@violet//:asan`  to use `--@violet//buildsystem/bazel/flags:asan`

<!-- flag-table-start -->
| Flag | Description | Default Value |
| :--- | :--------- | ------------: |
| <a id="bazel_flag_asan"/> `--@violet//buildsystem/bazel/flags:asan=[True\|False]` | Enables the **Address** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions. | `False` |
| <a id="bazel_flag_bitflags_free_function_impls"/> `--@violet//buildsystem/bazel/flags:bitflags_free_function_impls` | Enables the free-functions for the following operators when using the `violet::Bitflags` class: <br><br> * `operator\|` <br> * `operator&` <br> * `operator^` <br> * `operator~` | `False` |
| <a id="bazel_flag_dwarf_backend"/> `--@violet//buildsystem/bazel/flags:dwarf_backend=[libdwarf\|llvm\|violet\|none]` | The backend to use when parsing DWARF object files, primarily used in Linux. This is mainly used by the `Noelware.Violet.Experimental.Debugging` framework. When you don't have dependents on any target inside of `//violet/experimental/debugging`, this is disabled. <br><br>By default, this will use the **elfutils** `libdw` library. <br><br> * `libdwarf`: Uses **elfutils**' `libdw` library to parse DWARF object files. Recommended on most instances. <br> * `llvm`: Uses LLVM's machinery to parse DWARF object files. Only recommended if you want strong support, but at the cost of more compilation time as libLLVM is a huge project. <br> * `violet` (**EXPERIMENTAL**): Uses Noelware's implementation for parsing DWARF object files. Not recommended at the slightest; this is VERY experimental and things are subject to break. We're not sure if we want to do this. <br> * `none`: Allows you to build your own DWARF object-file backend. Very not recommended unless you want to have full control over the parsing yourself. | `"libdwarf"` |
| <a id="bazel_flag_mach_o_backend"/> `--@violet//buildsystem/bazel/flags:mach-o_backend=[llvm\|violet\|disable\|none]` | The backend to use when parsing Mach-O binaries on macOS. This is mainly used by the `Noelware.Violet.Experimental.Debugging` framework. When you don't have dependents on any target inside of `//violet/experimental/debugging`, this is disabled. <br><br> By default, it'll use `libLLVM` to parse Mach-O binaries. Unfortunately, `libLLVM` is VERY large, so that is the penalty right now unless you want to use very experimental machinery that the Violet team implements theirselves. <br><br> * `llvm`: Uses LLVM's machinery to parse DWARF object files. Only recommended if you want strong support, but at the cost of more compilation time as libLLVM is a huge project. <br> * `violet` (**EXPERIMENTAL**): Uses Noelware's implementation for parsing DWARF object files. Not recommended at the slightest; this is VERY experimental and things are subject to break. We're not sure if we want to do this. <br> * `disable`: Disables parsing Mach-O binaries alltogether. The implementation won't resolve line numbers or function names if disabled. <br> * `none`: Allows you to build your own Mach-O backend. Very not recommended unless you want to have full control over the parsing yourself. | `"llvm"` |
| <a id="bazel_flag_msan"/> `--@violet//buildsystem/bazel/flags:msan` | Enables the **Memory** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions. <br><br>When invoked on any C++ target, the C++ standard library implementation will require to be compiled with **MemorySanitizer**. This will always fail in libstdc++, but libc++ has MSan support, but you will need to compile it yourself; default toolchains of libc++ don't compile with MSan by default. | `False` |
| <a id="bazel_flag_runfiles_logs"/> `--@violet//buildsystem/bazel/flags:runfiles_logs` | Enables verbose logging on each test that invokes [`violet::testing::runfiles::Init`][runfiles-init] (only applicable on the **Noelware.Violet.Testing** framework when using Runfiles) | `True` |
| <a id="bazel_flag_tsan"/> `--@violet//buildsystem/bazel/flags:tsan` | Enables the **Thread** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions. | `False` |
| <a id="bazel_flag_ubsan"/> `--@violet//buildsystem/bazel/flags:ubsan` | Enables the **Undefined Behaviour** Sanitizer on each C++ target. Usually, this is meant for Bazel workspaces that don't provide custom C++ toolchain definitions. | `False` |
| <a id="bazel_flag_win32_dllexport"/> `--@violet//buildsystem/bazel/flags:win32_dllexport` | When set to **true**, this will use `__declspec(dllexport)` on MSVC toolchains instead of `__declspec(dllimport)`. This is a no-op on non-MSVC toolchains. | `False` |
<!-- flag-table-end -->

[runfiles-init]: #

### CMake
**CMake** is officially supported as of Violet 25.11. It's not usually up-to-date since at Noelware, we use Bazel but we try to.

At the moment, we don't provide `pkg-config` recipes yet but you can use CMake's [`FetchContent`]:

```cmake
FetchContent_Declare(
    violet
    GIT_REPOSITORY "https://github.com/Noelware/violet.git"
    GIT_TAG "<git tag here>"
)

# Disable installing for Violet
set(VIOLET_INSTALL OFF CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(violet)
```

[`FetchContent`]: https://cmake.org/cmake/help/latest/module/FetchContent.html

### Meson
**Meson** is not supported as of this current release. We have a basic `meson.build` but we won't add support unless necessary or if multiple people need support.

## License
**Violet** is licensed under the **Apache 2.0** License with love and care by [Noelware, LLC.](https://noelware.org).
