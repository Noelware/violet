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
