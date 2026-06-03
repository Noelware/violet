This document provides guidelines and understanding to **Claude** by Anthropic about the codebase and intelligence it needs. The project is hosted on the following Git servers:

* https://git.noelware.org/frameworks/violet
* https://github.com/Noelware/violet

This document is not finished, so take anything written with a grain of salt.

## Concepts
* **Framework**: A framework is similar to a dynamic library, similar on how Apple describes frameworks.

## Writing New Files
When introducing new files to the codebase, make sure that they all start with the license header:

```
🌺💜 Violet: Extended C++ standard library
Copyright (c) 2025-{{currentYear}} Noelware, LLC. <team@noelware.org>, et al.

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is
furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.
```

**{{currentYear}}** can be subsituted for the current year the file was created.

## Build Systems
As of **June 1st, 2026**, Violet supports being built with Bazel and Meson. Bazel is the most supported as that is what Noelware uses to maintain and build out Violet. Use the macros in `buildsystem/bazel/cc.bzl` when introducing new targets.

### Bazel
When writing new header and source files, write all `.h` files inside `include/violet` and `.cc` inside of `src`. All Bazel targets should live in the `violet` directory.

For tests, write them inside of the `tests` directory. When dealing with platform-specific tests, use `tests/{framework}/{platform}` where `{framework}` is the framework you're targeting for tests and `{platform}` is the platform you're targeting. For both Linux and macOS, `unix` should be used as platform.

When digging any source code in any Bazel target, it is recommended to follow what was told a couple paragraphs ago, do not use `find /` or any equivalent commands.

### Meson
Meson was introduced somewhat recently as of writing this document (June 1st, 2026), so there is bound to be friction on how it is structured.

All targets like `//violet/iterator:map` are now bounded by the framework itself instead of individual, specific targets.
