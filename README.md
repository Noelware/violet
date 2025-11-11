### 🌺💜 Violet
### *Extended standard library for C++26*

**Violet** is a C++26 library that extends C++'s standard library with better enriching types and provide a common framework for building C++ applications from the ground up without much effort. **Violet** was made to help develop our projects to bridge common code between our products and services using C++ a lot easier.

> [!IMPORTANT]
> **Violet** is 100% experimental software and may not work as intended! Please report bugs via [GitHub Issues](https://github.com/Noelware/Violet/issues/new)!
>
> ---
>
> **Violet** uses a rather unique versioning scheme: `year.month.revision` (i.e, `25.09.01`).
>
> We decided on this because it is very easy to see when a version of the library was released and doesn't
> enforce "breaking/change" semantics like [SemVer](https://semver.org), we depend on release timing for
> the scope of the library.

## Goals
The goal of **Violet** is to provide a enriched version of C++'s standard library by enhancing types that are pretty finicky (`std::expected`, `std::optional`) and providing new types and libraries (because let's face it, C++ iterators and ranges suck).

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
bazel_dep(name = "violet", version = "25.11") # you can pin `version` to what .violet-version says
git_override(
    module_name = "violet",
    remote = "https://github.com/Noelware/violet.git",
    commit = "<pinned commit here>"
)
```

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
