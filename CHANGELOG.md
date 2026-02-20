---
title: Violet | Changelog
description: "Changelog for all of Noelware.Violet base frameworks"
availableAt:
    - https://github.com/Noelware/violet/blob/master/CHANGELOG.md
    - https://noelware.org/changelogs/violet
---

### unreleased

**Git History**: <https://github.com/Noelware/violet/compare/26.03.04...master>

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
