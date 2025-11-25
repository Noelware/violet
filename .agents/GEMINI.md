This document provides guidelines for Google Gemini when interacting with the Noelware Violet project codebase. The project is hosted on the following Git servers:

* https://git.noelware.org/libraries/violet
* https://github.com/Noelware/violet

> **NOTE**: Use the GitHub version as you most likely don't have access to Noelware's Git server.

## Guides
### Writing new files
When introducing new files into the project (excluding third-party code), ensure that the license header is included in all generated files. Here's the standard license header:

````
🌺💜 Violet: Extended standard library for C++26
Copyright (c) 2025 Noelware, LLC. <team@noelware.org> & other contributors

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
````

### Documenting code
When documenting C++ code, avoid Doxygen-style comments. As we utilize Noelware's Noeldoc, please use Markdown formatting similar to Rust documentation:

````
<single line about this function>

<... more information about this function ...>

## Remarks
<Additional notes or remarks, if applicable>

## Example
<a simple example that can be copied and pasted>
````

Using `@param`, `@tparam`, `@throws`, and `@return` is the only things you should use.

### Building
When building Violet, we provide support for **CMake**, **Meson**, and **Bazel**. Please use Bazel as the primary build system for this library before introducting CMake and Meson targets.

### Finding source files and tests when requested [Bazel only]
Violet uses a different structure than most Bazel projects. DO NOT LOOK IN `src`, `tests`, or `include/violet` FOR TARGETS AS THEY ONLY EXPOSE FILES, NOT `cc_library`/`cc_test` DEFINITIONS; NEVER SUGGEST `cc_library`/`cc_test` IN `src`, `tests`, OR `include/violet`.
