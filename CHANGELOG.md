---
title: Violet | Changelog
description: "Changelog for all of Noelware.Violet base frameworks"
availableAt:
    - https://github.com/Noelware/violet/blob/master/CHANGELOG.md
    - https://noelware.org/changelogs/violet
---

### unreleased
- **Breaking**: All Bazel targets now use hidden visibility by default, public APIs will be marked with `VIOLET_API` ([`@auguwu`])
    - On Windows, you can set the `win32_dllexport` flag to **True** (`--@violet//:win32_dllexport=[True|False]`) so that MSVC will see `__declspec(dllexport)` instead of `__declspec(dllimport)`.
- There is now more annotations related to lifetime bounds and more. ([`@auguwu`])

#### Noelware.Violet
- Add **operator bool()** definition to `violet::Bitflags` ([`@auguwu`])
- Make the default constructor for **violet::terminal::Style** be the "reset" marker (`\033[0m`) ([`@auguwu`])
- Add **violet::Defer** and **violet::CancellableDefer** (and C-style macros) ([`@auguwu`])
- Add **violet::strings::Join** and **violet::strings::Lines** utility ([`@auguwu`])
- Fix return type errors in **violet::Result** ([`@auguwu`])
- Improve iterators framework ([`@auguwu`])
    - `MkIterable` now supports rvalues **correctly**
    - Containers that have `.begin()` and `.end()` can resolve into `MkIterable` correctly

#### Noelware.Violet.IO
- Add implicit copy/move constructors for **Error** and **PlatformError** ([`@auguwu`])

#### Noelware.Violet.Experimental
- Delete copy constructors for **OneOf<Ts...>** if `<Ts...>` has one or more non-copyable constructors/copy assignments ([`@auguwu`])

#### Noelware.Violet.Experimental.Threading
- Add thread cancellation (**CancellationToken**, **CancellationTokenSource**) ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.04.06...master>

### 26.04.06
#### Noelware.Violet
- Fix GCC errors when **__builtin_COLUMN()** doesn't exist and `#pragma` errors as well ([`@auguwu`])
- **Optional** and **Result** now use the infrastructure of **violet::SourceLocation** and **violet::PrintErrln** ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.04.05...26.04.06>

### 26.04.05
#### Noelware.Violet
- Add **violet/SourceLocation.h** header to bridge **std::source_location** so it can be constructible ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.04.04...26.04.05>

### 26.04.04
#### Noelware.Violet
- Add **violet/Print.h** header to bridge the `<print>` header in C++23 that works in C++20 ([`@auguwu`])
- Moved all assertion logic into `violet/Language/Assert.h` so that it doesn't have to depend on `violet/Violet.h` if it is not needed ([`@auguwu`])

#### Noelware.Violet.Experimental
- **SmolString** now uses Violet's assertion logic rather than C's `assert` macro ([`@auguwu`])
- Fix bug in **OneOf** where `OneOf<Ts...>::IndexOf<T>` doesn't remove cvref qualifiers and other magic stuff ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.04.03...26.04.04>

### 26.04.03
#### Noelware.Violet
- Fix **missing `dummy.cc` file** when using any framework-specific targets like `@violet//:violet` ([`@auguwu`])

#### Noelware.Violet.IO
- Add **io::Error::FromOSError** to use a OS error instead of `errno` (on Unix) ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.04.02...26.04.03>

### 26.04.02
#### Noelware.Violet
- Fix distribution tool from removing a needed `cc_library` rule ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.04.01...26.04.02>

### 26.04.01
#### Noelware.Violet
- Allow **violet::Optional** to hold references ([`@auguwu`])

#### Noelware.Violet.Experimental
- Introduce **violet::experimental::OneOf**, a experimental type to be used as a discriminated union. ([`@auguwu`])

#### Noelware.Violet.IO
- Reimplement **violet::io::ReadToString** for readable types ([`@auguwu`])
- Fix issues within the `Vec<UInt8>` reader ([`@auguwu`])

#### Noelware.Violet.IO.Experimental
- Fully implement **violet::io::experimental::ReadToString** for any input streams and allow a `String` type to be specified that implements the following constraints: ([`@auguwu`])
    - `type value_type = ...;` must be present
    - `type size_type  = ...;` must be present
    - `String::reserve(size_type)` must be present
    - `String::append(value_type*, size_type)` must be present
- Fix bug in **ReadToString** and **ByteArrayInputStream** when reading will overflow a `UInt8` due to a dev issue ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.04...26.04.01>

### 26.04
- [Bazel] Add framework-specific "uber" libraries under `@violet` ([`@auguwu`])
    - This will ensure that you can pick out which framework you want to use. I.e, `@violet` will point to Noelware.Violet, `@violet//:io` will point to all `@violet//violet/io`.

#### Noelware.Violet
- Fix **VIOLET_VERSION** not being defined in `violet/Runtime/Info.h` ([`@auguwu`])
- Remove `violet/System/Which.h` header and implementation ([`@auguwu`])

#### Noelware.Violet.Experimental
- Add documentation for methods in **SmolString** ([`@auguwu`])

#### Noelware.Violet.Filesystem
- Fix bug where `violet::filesystem::RemoveAllDirs` doesn't properly clean up and throws `I/o error (system error «20»): Not a directory` ([`@auguwu`])
- Add `violet::filesystem::Executable` to check if a file is executable ([`@auguwu`])

#### Noelware.Violet.Testing
- Add **CaptureStream** RAII guard to capture a `std::ostream` for tests ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.03.09...26.04>

### 26.03.09
#### Noelware.Violet
- Add `violet/Runtime/Info.h` header to provide version information about the **Noelware.Violet** framework ([`@auguwu`])
- Add **SmolString** experimental primitive for small strings ([`@auguwu`])
- Add **anyhow::Chain** iterator to iterate over a error's contextual frames ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.03.08...26.03.09>

### 26.03.08
#### Noelware.Violet
- Add **violet::strings::SplitN** utility ([`@auguwu`])
- Added **VIOLET_DEPRECATED** and **VIOLET_DEPRECATED_BECAUSE** macros ([`@auguwu`])
- Fix `VIOLET_TRY_VOID` macro from expanding `#pragma`s ([`@auguwu`])

#### Noelware.Violet.Filesystem
- Fix issues when using `Create`, `CreateNew`, and `Write` open flags ([`@auguwu`])

#### Noelware.Violet.IO.Experimental
- Add output stream support ([`@auguwu`])
    - file
    - stdout, stderr
    - string
    - byte array

**Git History**: <https://github.com/Noelware/violet/compare/26.03.07...26.03.08>

### 26.03.07
#### Noelware.Violet
- Fix out of bounds string in **violet::sys::WorkingDirectory** ([`@auguwu`])
- Don't use `VIOLET_ASSERT` in `violet::sys::SetEnv`, `violet::sys::RemoveEnv`

**Git History**: <https://github.com/Noelware/violet/compare/26.03.07...26.03.06>

### 26.03.06
#### Noelware.Violet
- Fixed memory-related issues in **violet/anyhow.h** ([`@auguwu`])
- Use **std::abort** in `VIOLET_ASSERT`/`VIOLET_DEBUG_ASSERT` macros ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.03.06...26.03.05>

### 26.03.05
#### Noelware.Violet
- Add documentation for all headers inside of this framework ([`@auguwu`])
- Add **optional_type** and **optional_type_t** type traits inside of `violet/Container/Optional.h` rather than as internals ([`@auguwu`])
- Add **result_type**, **result_value_type_t**, and **result_error_type_t** type traits ([`@auguwu`])
- Remove source location printing in **violet::anyhow** ([`@auguwu`])
- Add **VIOLET_TRY** and **VIOLET_TRY_VOID** macros for Clang/GCC ([`@auguwu`])
- Add **VIOLET_UNIQUE_NAME** and **VIOLET_CONCAT** macros ([`@auguwu`])
- **Breaking**: Return `U` instead of an always engaged `Optional<U>` in `Optional::MapOr` ([`@auguwu`])
- **Breaking**: Add rvalue variants of `Result::Error()` and remove `Result::IntoErr()` ([`@auguwu`])
- **Breaking**: Remove Abseil dependency as it is no longer needed ([`@auguwu`], [#27])

#### Noelware.Violet.IO
- Add documentation for all headers inside of this framework ([`@auguwu`])

### Noelware.Violet.Filesystem
- Add documentation for all headers inside of this framework ([`@auguwu`])
- Added **xattr::List** to list a file's extended attributes, if supported ([`@auguwu`])
- **Breaking [Linux]**: In `filesystem::Copy`, return the total amount of bytes rather than the last counter ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.02.05...26.02.04>

### 26.03.04
- **Noelware.Violet.IO.Experimental**: Add `Position` and `EOS` methods to ByteArrayInputStream and StringInputStream ([`@auguwu`])
- **Noelware.Violet.IO.Experimental**: Add documentation to all InputStreams ([`@auguwu`])
- **Noelware.Violet**: Add `violet/Strings.h` header, which provide utilities for strings, original implementation originated from [`Noelware/Eous`](https://github.com/Noelware/Eous ([`@auguwu`])
- **Noelware.Violet**: Implement **violet::anyhow** (original impl from [`Noelware/Eous`](https://github.com/Noelware/Eous)) ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.02.03...26.02.04>

### 26.03.03
- **Noelware.Violet.IO**: Fix `stdin_input_stream` target not including the Unix implementation ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.03.02...26.03.03>

### 26.03.02
- **Noelware.Violet**: Get rid of `violet::StringifyFormatter` and write `std::formatter` in-place ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.03.01...26.03.02>

### 26.03.01
- **Noelware.Violet**: Fix ADL issues with `VIOLET_TO_STRING` macro (and `violet::StringifyFormatter` class) ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.03...26.03.01>

### 26.02.03
- **Noelware.Violet.IO**: Add `Noelware.Violet.IO.Experimental` framework ([`@auguwu`])
- **Noelware.Violet.IO.Experimental**: Add input streams for stdin, files, strings, and byte arrays (`Vec<UInt8>`) ([`@auguwu`])
- **Noelware.Violet.IO**: Fix issue where `free` wasn't found in `violet::util::DemangleCXXName` ([`@auguwu`])
- **Noelware.Violet**: Add `VIOLET_HOT` and `VIOLET_IS_{LITTLE|BIG}_ENDIAN` macros ([`@auguwu`])
- **Noelware.Violet**: Remove numeric framework as it wasn't needed ([`@auguwu`])
- **Noelware.Violet**: Remove `VIOLET_UNREACHABLE_WITH` and `VIOLET_UNREACHABLE` will use `std::unreachable()` in C++23 ([`@auguwu`])
- **Noelware.Violet.IO**, **Noelware.Violet** (`Support` framework): Fix errors with `-fno-exceptions`-enabled code ([`@auguwu`], [#22])
- **Noelware.Violet**: Remove flat hash map/set, btree maps from Abseil ([`@auguwu`])
- **Noelware.Violet**: Remove Numeric framework ([`@auguwu`])
- **Noelware.Violet**: Refractor `violet::Result<T, E>` API ([`@auguwu`])
    - Add stricter requirements for both `violet::Result` and `violet::Err`
    - Add `is_result`, `is_result_v` type traits and `nested_result` C++ concept
    - `Unwrap`, `UnwrapErr`, `Expect` will show source location where it was thrown at
    - Added new functions:
        - `AndThen`
        - `Transpose`
        - `Inspect`
        - `InspectErr`
        - `IntoErr`
        - `OkAnd`
        - `ErrAnd`
- **Noelware.Violet**: Refractor `violet::Optional<T>` API ([`@auguwu`], [PR #23])
    - Added stricter requirements
    - Add `is_optional`, `is_optional_v` type traits
    - `Unwrap`, `Expect` will show source location where it was thrown at

**Git History**: <https://github.com/Noelware/violet/compare/26.02.02...26.02.03>

### 26.02.03-dev.2
- **Noelware.Violet**: Add `VIOLET_HOT` and `VIOLET_IS_{LITTLE|BIG}_ENDIAN` macros ([`@auguwu`])
- **Noelware.Violet**: Remove numeric framework as it wasn't needed ([`@auguwu`])
- **Noelware.Violet**: Remove `VIOLET_UNREACHABLE_WITH` and `VIOLET_UNREACHABLE` will use `std::unreachable()` in C++23 ([`@auguwu`])
- **Noelware.Violet.IO**, **Noelware.Violet** (`Support` framework): Fix errors with `-fno-exceptions`-enabled code ([`@auguwu`], [#22])

**Git History**: <https://github.com/Noelware/violet/compare/26.02.03-dev.1...26.02.03-dev.2>

### 26.02.03-dev.1
- **Noelware.Violet.IO**: Fix issue where `free` wasn't found in `violet::util::DemangleCXXName` ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.02.03-dev...26.02.03-dev.1>

### 26.02.03-dev [prerelease]
- **Noelware.Violet.IO**: Add `Noelware.Violet.IO.Experimental` framework ([`@auguwu`])
- **Noelware.Violet.IO.Experimental**: Add input streams for stdin, files, strings, and byte arrays (`Vec<UInt8>`) ([`@auguwu`])

**Git History**: <https://github.com/Noelware/violet/compare/26.02.02...26.02.03-dev>

### 26.02.02
- **Noelware.Violet.Testing**: Add loose workspace name detection based off context (@auguwu)
- **Noelware.Violet.IO**: `FileDescriptor` flushes with `EINVAL` on non-file descriptors (i.e, `STDOUT_FILENO`) (#19, @auguwu)
- **Noelware.Violet.Filesystem**: Fix out-of-bounds error with `violet::filesystem::Canonicalize` (@auguwu)
- Fix `VIOLET_VERSION_STR` definition not encoded as a string (#20, @auguwu)

**Git History**: https://github.com/Noelware/violet/compare/26.02.01...26.02.02

### 26.02.01
- **Noelware.Violet.IO**: Make `FileDescriptor` copyable and movable

**Git History**: https://github.com/Noelware/violet/compare/26.02...26.02.01

### 26.02
This release indicates that Violet is ready to be shipped as a **MVP**. Report any issues you might have to our [issue tracker](https://github.com/Noelware/violet/issues/new).

## Available Frameworks
- **Noelware.Violet** (`@violet//violet`, `@violet//violet/container`, `@violet//violet/iterator`, `@violet//violet/system`, `@violet//violet/support`, `@violet//violet/numeric`)
- **Noelware.Violet.Events** (`@violet//violet/events`)
- **Noelware.Violet.Filesystem** (`@violet//violet/filesystem`)
- **Noelware.Violet.IO** (`@violet//violet/io`)
- **Noelware.Violet.Testing** (`@violet//violet/testing`)

**Git History**: https://github.com/Noelware/violet/commits/26.02

[`@auguwu`]: https://github.com/auguwu

[PR #23]: https://github.com/Noelware/violet/pull/23
[#27]: https://github.com/Noelware/violet/issues/27
[#22]: https://github.com/Noelware/violet/issues/22
