// 🌺💜 Violet: Extended C++ standard library
// Copyright (c) 2025-2026 Noelware, LLC. <team@noelware.org>, et al.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in all
// copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

#pragma once

#include <violet/Container/Optional.h>
#include <violet/IO/Descriptor.h>
#include <violet/IO/Error.h>
#include <violet/Strings.h>

#include <cstring>

namespace violet::filesystem {

#if VIOLET_PLATFORM(UNIX)
/// The platform's preferred path separator: `/` on Unix.
inline constexpr auto PathSeparator = '/';
#else
/// The platform's preferred path separator: `\` on Windows.
inline constexpr auto PathSeparator = '\\';
#endif

namespace path_internal {

template<typename StringType>
concept stdlib_string = std::same_as<StringType, std::string> || std::same_as<StringType, std::string_view>
    || std::same_as<StringType, std::wstring> || std::same_as<StringType, std::wstring_view>
    || std::same_as<StringType, std::u8string> || std::same_as<StringType, std::u8string_view>
    || std::same_as<StringType, std::u16string> || std::same_as<StringType, std::u16string_view>
    || std::same_as<StringType, std::u32string> || std::same_as<StringType, std::u32string_view>;

// TODO(@auguwu/Noel): once Violet's String/Str implementations are stablized, the concept for `violet_string_type`
// can be:
//
// template<typename StringType>
// concept violet_string_type = std::same_as<StringType, String> || std::same_as<StringType, Str>
//      || std::same_as<StringType, WideString> || std::same_as<StringType, WideStr>
//      || std::same_as<StringType, U8String> || std::same_as<StringType, U8Str>
//      || std::same_as<StringType, U16String> || std::same_as<StringType, U16Str>
//      || std::same_as<StringType, U32String> || std::same_as<StringType, U32Str>;

template<typename StringType>
concept violet_string_type = requires(std::remove_cvref_t<StringType> str) {
    typename std::remove_cvref_t<StringType>::size_type;
    typename std::remove_cvref_t<StringType>::character;
    typename std::remove_cvref_t<StringType>::view_type;

    requires std::same_as<std::remove_cvref_t<decltype(std::remove_cvref_t<StringType>::npos)>,
        typename std::remove_cvref_t<StringType>::size_type>;
};

template<typename StringType>
constexpr auto Empty(const StringType& str) noexcept -> bool
{
    if constexpr (violet_string_type<StringType>) {
        return str.Empty();
    } else {
        return str.empty();
    }
}

template<typename StringType>
constexpr auto Size(const StringType& str) noexcept -> StringType::size_type
{
    if constexpr (violet_string_type<StringType>) {
        return str.Size();
    } else {
        return str.size();
    }
}

template<typename StringType, typename... Args>
constexpr auto At(const StringType& str, Args&&... args) noexcept -> const strings::character_t<StringType>&
{
    if constexpr (violet_string_type<StringType>) {
        return str.At(VIOLET_FWD(Args, args)...);
    } else {
        return str.at(VIOLET_FWD(Args, args)...);
    }
}

template<typename StringType>
constexpr auto Length(const StringType& str) noexcept -> StringType::size_type
{
    if constexpr (violet_string_type<StringType>) {
        return str.Length();
    } else {
        return str.length();
    }
}

template<typename StringType, typename... Args>
constexpr auto Substr(const StringType& str, Args&&... args) -> decltype(auto)
{
    if constexpr (violet_string_type<StringType>) {
        return str.Substr(VIOLET_FWD(Args, args)...);
    } else {
        return str.substr(VIOLET_FWD(Args, args)...);
    }
}

template<typename StringType, typename... Args>
constexpr auto Revfind(const StringType& str, Args&&... args) -> StringType::size_type
{
    if constexpr (violet_string_type<StringType>) {
        return str.Revfind(VIOLET_FWD(Args, args)...);
    } else {
        return str.rfind(VIOLET_FWD(Args, args)...);
    }
}

template<typename StringType, typename... Args>
constexpr auto Format(std::format_string<Args...> fmt, Args&&... args) -> StringType
{
    if constexpr (requires { StringType::Format(fmt, VIOLET_FWD(Args, args)...); }) {
        return StringType::Format(fmt, VIOLET_FWD(Args, args)...);
    } else {
        return std::format(fmt, VIOLET_FWD(Args, args)...);
    }
}

template<typename StringType, typename... Args>
constexpr auto Back(const StringType& str) -> const strings::character_t<StringType>&
{
    if constexpr (violet_string_type<StringType>) {
        return str.Back();
    } else {
        return str.back();
    }
}

template<typename StringType, typename... Args>
void AppendBack(StringType& str, Args&&... args)
{
    if constexpr (violet_string_type<StringType>) {
        str.AppendBack(VIOLET_FWD(Args, args)...);
    } else {
        str.push_back(VIOLET_FWD(Args, args)...);
    }
}

template<typename StringType, typename... Args>
auto Append(StringType& str, Args&&... args) -> StringType&
{
    if constexpr (violet_string_type<StringType>) {
        return str.Append(VIOLET_FWD(Args, args)...);
    } else {
        return str.append(VIOLET_FWD(Args, args)...);
    }
}

template<typename StringType>
void Resize(StringType& str, typename StringType::size_type num)
{
    if constexpr (violet_string_type<StringType>) {
        str.Resize(num);
    } else {
        str.resize(num);
    }
}

template<typename StringType>
void PopBack(StringType& str)
{
    if constexpr (violet_string_type<StringType>) {
        str.PopBack();
    } else {
        str.pop_back();
    }
}

template<typename StringType>
constexpr auto DataPtr(const StringType& str) noexcept -> const strings::character_t<StringType>*
{
    if constexpr (violet_string_type<StringType>) {
        return str.Data();
    } else {
        return str.data();
    }
}

constexpr auto IsSeparator(auto ch) noexcept -> bool
{
    return ch == '/' || ch == '\\';
}

/// Strips trailing separators from `value` and returns the position of the last remaining separator (or -1),
/// alongside the stripped string the position refers to.
template<typename StringType>
constexpr auto computeTrailingSlashPos(const StringType& value) noexcept -> Pair<Int64, StringType>
{
    // only trailing separators: a leading one is the root and `Parent()` needs it.
    auto data = strings::TrimEnd(value, [](char ch) noexcept -> bool { return ch == '/' || ch == '\\'; });
    Int64 pos = -1;

    for (auto i = static_cast<Int64>(Length(data)) - 1; i >= 0; i--) {
        char ch = path_internal::At(data, i);
        if (ch == '/' || ch == '\\') {
            pos = i;
            break;
        }
    }

    return {pos, StringType(data)};
}

template auto computeTrailingSlashPos(const std::string&) -> Pair<Int64, std::string>;
template auto computeTrailingSlashPos(const std::string_view&) -> Pair<Int64, std::string_view>;

/// Lexically normalizes `path`: collapses repeated separators, drops `.`, and resolves `..` against the previous
/// component. Never touches the filesystem.
template<typename StringType>
auto canonicalize(const StringType& path) noexcept -> strings::owned_type<StringType>
{
    const auto isSeparator = [](auto ch) noexcept -> bool { return ch == '/' || ch == '\\'; };

    strings::owned_type<StringType> result;
    const auto size = path_internal::Size(path);
    const bool absolute = size > 0 && isSeparator(path[0]);
    if (absolute) {
        path_internal::AppendBack(result, PathSeparator);
    }

    // `..` never pops below this length: the root for absolute paths, or the leading `..`s of
    // a relative path (`../../a` can't be simplified without the filesystem).
    UInt floor = path_internal::Size(result);
    UInt index = 0;
    while (index < size) {
        if (isSeparator(path[index])) {
            index++;
            continue;
        }

        UInt start = index;
        while (index < size && !isSeparator(path[index])) {
            index++;
        }

        strings::view_type<StringType> part(&path[start], index - start);
        if (part == ".") {
            continue;
        }

        if (part == "..") {
            if (path_internal::Size(result) > floor) {
                const auto pos = path_internal::Revfind(result, PathSeparator);
                path_internal::Resize(result, pos == StringType::npos || pos < floor ? floor : pos);

                continue;
            }

            if (absolute) {
                continue;
            }

            if (!path_internal::Empty(result)) {
                path_internal::AppendBack(result, PathSeparator);
            }

            path_internal::Append(result, part);
            floor = path_internal::Size(result);

            continue;
        }

        if (!path_internal::Empty(result) && path_internal::Back(result) != PathSeparator) {
            path_internal::AppendBack(result, PathSeparator);
        }

        path_internal::Append(result, part);
    }

    if (path_internal::Empty(result) && size > 0) {
        path_internal::AppendBack(result, '.');
    }

    return result;
}

template auto canonicalize(const std::string&) -> std::string;

} // namespace path_internal

struct Path;
struct PathRef;
struct Ancestors;
struct Components;

/// CRTP base class for path-like types.
///
/// This class provides common functionality for both immutable, non-owning paths
/// ([`PathRef`]) and owned, mutable paths ([`Path`]). It uses the CRTP (Curiously Recurring Template Pattern)
/// to allow compile-time re-use of methods while leaving `Derived`'s storage type to be flexible.
///
/// @tparam Derived The derived class
/// @tparam StringType The underling string type to use
template<typename Derived, typename StringType>
    requires(path_internal::stdlib_string<StringType> || path_internal::violet_string_type<StringType>)
struct NOELDOC_SINCE("26.02") BasePath {
    /// The size type of `StringType`.
    using size_type = std::remove_cvref_t<StringType>::size_type;

    /// The character type of `StringType`.
    using character = strings::character_t<std::remove_cvref_t<StringType>>;

    /// Returns the underlying string of this path.
    [[nodiscard]]
    constexpr auto Data() const noexcept -> StringType
    {
        return getThisObject().storage();
    }

    /// Returns **true** if this path is empty.
    [[nodiscard]]
    constexpr auto Empty() const noexcept -> bool
    {
        return path_internal::Empty(this->Data());
    }

    /// Returns the length of the path string.
    [[nodiscard]]
    NOELDOC_SINCE("26.11") constexpr auto Size() const noexcept -> size_type
    {
        return path_internal::Size(this->Data());
    }

    /// Returns **true** if this path is absolute.
    ///
    /// ## Platform-specific Remarks
    /// - Unix: the path starts with `/`.
    /// - Windows: the path starts with a drive letter, colon, and separator (`C:\`), or is a UNC path
    ///   (`\\server\share`). A drive-relative path like `C:foo` is **not** absolute.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef absolute = "/usr/bin";
    /// assert(absolute.Absolute());
    ///
    /// PathRef relative = "usr/bin";
    /// assert(!relative.Absolute());
    /// ```
    [[nodiscard]]
    constexpr auto Absolute() const noexcept -> bool
    {
        if (this->Empty()) {
            return false;
        }

        const auto data = this->Data();

#if VIOLET_PLATFORM(UNIX)
        return data[0] == '/';
#elif VIOLET_PLATFORM(WINDOWS)
        const auto size = path_internal::Size(data);
        if (size >= 3 && isDriveLetter(data[0]) && data[1] == ':' && path_internal::IsSeparator(data[2])) {
            return true;
        }

        return size >= 2 && data[0] == '\\' && data[1] == '\\';
#else
#warning implementation not provided for this platform
        return false;
#endif
    }

    /// Returns **true** if this path is relative, i.e, not [absolute](BasePath::Absolute).
    [[nodiscard]]
    constexpr auto Relative() const noexcept -> bool
    {
        return !this->Absolute();
    }

    /// Returns **true** if this path is a root directory.
    ///
    /// ## Platform-specific Remarks
    /// - Unix: the path is `/`.
    /// - Windows: the path is a drive root (`C:` or `C:\`).
    [[nodiscard]]
    constexpr auto Root() const noexcept -> bool
    {
        const auto data = this->Data();
        const auto size = path_internal::Size(data);

#if VIOLET_PLATFORM(UNIX)
        return size == 1 && data[0] == '/';
#elif VIOLET_PLATFORM(WINDOWS)
        return (size == 2 || (size == 3 && path_internal::IsSeparator(data[2]))) && isDriveLetter(data[0])
            && data[1] == ':';
#else
#warning implementation not provided for this platform
        return false;
#endif
    }

    /// Returns the path as its string type.
    [[nodiscard]]
    constexpr auto ToString() const noexcept -> StringType
    {
        return this->Data();
    }

    /// Returns the last component of this path.
    ///
    /// The filename is everything after the final separator (`/` or `\`). Trailing separators are ignored, so
    /// `/home/noeltowa/` has the filename `noeltowa`.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef p1 = "/home/noeltowa/Documents/loveletter.txt";
    /// assert(p1.Filename() == "loveletter.txt");
    ///
    /// PathRef p2 = "/home/noeltowa/";
    /// assert(p2.Filename() == "noeltowa");
    /// ```
    [[nodiscard]]
    auto Filename() const noexcept -> StringType
    {
        if (this->Empty()) {
            return {};
        }

        auto [pos, val] = path_internal::computeTrailingSlashPos(this->Data());
        if (pos == -1) {
            return StringType(val);
        }

        return path_internal::Substr(val, pos + 1);
    }

    /// Returns the extension of this path's [filename](BasePath::Filename), if any.
    ///
    /// The extension is everything after the last `.`. A filename that only has a leading dot (`.gitignore`) has
    /// no extension.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef p1 = "/home/noeltowa/file.txt";
    /// assert(*p1.Extension() == "txt");
    ///
    /// PathRef p2 = "/home/noeltowa/file";
    /// assert(!p2.Extension());
    ///
    /// PathRef p3 = "/home/noeltowa/.gitignore";
    /// assert(!p3.Extension());
    /// ```
    [[nodiscard]]
    auto Extension() const noexcept -> Optional<StringType>
    {
        StringType filename = this->Filename();

        const auto pos = path_internal::Revfind(filename, '.');
        if (pos == StringType::npos || pos == 0) {
            return Nothing;
        }

        return path_internal::Substr(filename, pos + 1);
    }

    /// Returns this path's [filename](BasePath::Filename) without its [extension](BasePath::Extension).
    ///
    /// Leading dots are kept for hidden files like `.gitignore`.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef p1 = "/home/noeltowa/Documents/loveletter.txt";
    /// assert(p1.Stem() == "loveletter");
    ///
    /// PathRef p2 = "/home/noeltowa/.gitignore";
    /// assert(p2.Stem() == ".gitignore");
    /// ```
    [[nodiscard]]
    auto Stem() const noexcept -> StringType
    {
        StringType filename = this->Filename();

        const auto pos = path_internal::Revfind(filename, '.');
        if (pos == StringType::npos || pos == 0) {
            return filename;
        }

        return path_internal::Substr(filename, 0, pos);
    }

    /// Returns this path without its last component, or [`Nothing`] if it has none.
    ///
    /// ## Behaviour
    /// - `/home/user/docs/file.txt` -> `/home/user/docs`
    /// - `/home` -> `/`
    /// - `/` -> [`Nothing`]
    /// - `file.txt` -> [`Nothing`]
    ///
    /// ### Windows
    /// - `C:\Windows\System32\cmd.exe` -> `C:\Windows\System32`
    /// - `C:\Windows` -> `C:\`
    /// - `C:\` -> [`Nothing`]
    ///
    /// ## Example
    /// ```cpp
    /// PathRef path = "/usr/local/bin";
    /// if (auto parent = path.Parent()) {
    ///     assert(parent->Data() == "/usr/local");
    /// }
    /// ```
    [[nodiscard]]
    auto Parent() const noexcept -> Optional<Derived>
    {
        if (this->Empty()) {
            return Nothing;
        }

        auto [pos, val] = path_internal::computeTrailingSlashPos(this->Data());
        if (pos == -1) {
            return Nothing;
        }

#if VIOLET_PLATFORM(UNIX)
        if (pos == 0) {
            return Some<Derived>("/");
        }

        return Some<Derived>(path_internal::Substr(val, 0, pos));
#elif VIOLET_PLATFORM(WINDOWS)
        // keep the separator of a drive root: the parent of `C:\Windows` is `C:\`, not `C:`.
        if (pos == 2 && val[1] == ':') {
            return Some<Derived>(path_internal::Substr(val, 0, pos + 1));
        }

        return Some<Derived>(path_internal::Substr(val, 0, pos));
#else
#warning implementation not provided for this platform
        return Nothing;
#endif
    }

    /// Returns a new path with the last component replaced by `filename`.
    ///
    /// If this path has no parent, the result is just `filename`. On Unix, the root `/` becomes `/<filename>`.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef path = "/home/user/docs/report.txt";
    /// Path newPath = path.WithFilename("summary.txt");
    /// assert(newPath == "/home/user/docs/summary.txt");
    /// ```
    ///
    /// @param filename the new last component
    [[nodiscard]]
    auto WithFilename(strings::view_type<StringType> filename) const -> Path;

    /// Returns a new path with the filename's extension replaced by `ext`.
    ///
    /// If the filename has no extension, `ext` is appended. The leading `.` in `ext` is optional.
    ///
    /// ## Remarks
    /// - An empty `ext` removes the extension.
    /// - The parent path and the filename's stem are kept.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef path = "/home/noeltowa/Documents/loveletter.txt";
    /// Path newPath = path.WithExtension("md");
    /// assert(newPath == "/home/noeltowa/Documents/loveletter.md");
    /// ```
    ///
    /// @param ext the new extension, with or without a leading `.`
    [[nodiscard]]
    auto WithExtension(strings::view_type<StringType> ext) const -> Path;

    /// Returns a new path with `rhs` appended, inserting a separator if needed.
    ///
    /// If `rhs` is absolute, it replaces this path entirely.
    ///
    /// ## Example
    /// ```cpp
    /// Path base = "/home/user";
    /// Path joined = base.Join("documents/report.txt");
    /// assert(joined == "/home/user/documents/report.txt");
    ///
    /// joined = base.Join("/etc/passwd");
    /// assert(joined == "/etc/passwd");
    /// ```
    ///
    /// @param rhs the path or path segment to append
    [[nodiscard]]
    auto Join(strings::view_type<StringType> rhs) const -> Path;

    /// Calls `fun` with a NUL-terminated copy of this path.
    ///
    /// ## Remarks
    /// - The pointer is only valid for the duration of the call.
    /// - Strings that are already NUL-terminated are passed through as-is.
    /// - Views are copied into a stack buffer when they're shorter than 256 characters, and into a heap-allocated
    ///   string otherwise.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef path = "/etc/hosts";
    /// int fd = path.WithCStr([](const char* cstr) { return ::open(cstr, O_RDONLY); });
    /// ```
    ///
    /// @param fun the function to call with the NUL-terminated path
    /// @return whatever `fun` returns
    template<typename Fun>
        requires callable<Fun, const character*>
    auto WithCStr(Fun&& fun) const noexcept(std::is_nothrow_invocable_v<Fun, const character*>)
        -> std::invoke_result_t<Fun, const character*>
    {
        if constexpr (requires(const StringType& str) { str.c_str(); }) {
            return std::invoke(VIOLET_FWD(Fun, fun), this->Data().c_str());
        } else if constexpr (requires(const StringType& str) { str.AsCStr(); }) {
            return std::invoke(VIOLET_FWD(Fun, fun), this->Data().AsCStr());
        } else {
            constexpr static UInt kStackBufferSize = 256;

            const auto data = this->Data();
            const auto size = path_internal::Size(data);
            if (size < kStackBufferSize) {
                Array<character, kStackBufferSize> buf{};
                std::copy_n(path_internal::DataPtr(data), size, buf.data());

                return std::invoke(VIOLET_FWD(Fun, fun), static_cast<const character*>(buf.data()));
            }

            strings::owned_type<std::remove_cvref_t<StringType>> owned(data);
            if constexpr (requires { owned.c_str(); }) {
                return std::invoke(VIOLET_FWD(Fun, fun), owned.c_str());
            } else {
                return std::invoke(VIOLET_FWD(Fun, fun), owned.AsCStr());
            }
        }
    }

    /// Returns **true** if `other` is a leading run of this path's components.
    ///
    /// The comparison is by component, not by raw string: `/usr/libexec` does not start with `/usr/lib`. Every path
    /// starts with an empty path.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef path = "/usr/lib/libc.so";
    /// assert(path.StartsWith(PathRef("/usr/lib")));
    /// assert(!path.StartsWith(PathRef("/usr/li")));
    /// ```
    ///
    /// @param other the path prefix to check for
    [[nodiscard]]
    constexpr auto StartsWith(const Derived& other) const noexcept -> bool
    {
        const auto storage = this->Data();
        const auto prefix = other.storage();
        if (path_internal::Empty(prefix)) {
            return true;
        }

        bool matches = false;
        if constexpr (requires(const StringType& str, const StringType& rhs) { str.StartsWith(rhs); }) {
            matches = storage.StartsWith(prefix);
        } else {
            matches = storage.starts_with(prefix);
        }

        if (!matches) {
            return false;
        }

        // "/usr/lib" is a prefix of "/usr/lib" and "/usr/lib/x", but not "/usr/libexec".
        const auto num = path_internal::Size(prefix);
        return num == path_internal::Size(storage) || path_internal::IsSeparator(storage[num])
            || path_internal::IsSeparator(path_internal::Back(prefix));
    }

    /// Returns **true** if this path's string starts with `args`, forwarded to the string's own `starts_with`.
    ///
    /// Unlike the [`Derived`] overload, this compares raw strings: `PathRef("/usr/libexec").StartsWith("/usr/lib")`
    /// is **true**.
    ///
    /// @param args arguments forwarded to `StringType::starts_with` (or `StartsWith`)
    template<typename... Args>
        requires(sizeof...(Args) != 1 || !std::same_as<std::remove_cvref_t<pack_element_t<0, Args...>>, Derived>)
    [[nodiscard]] NOELDOC_SINCE("26.11") constexpr auto StartsWith(Args&&... args) const noexcept -> bool
    {
        if constexpr (requires(const StringType& str) { str.StartsWith(VIOLET_FWD(Args, args)...); }) {
            return this->Data().StartsWith(VIOLET_FWD(Args, args)...);
        } else {
            return this->Data().starts_with(VIOLET_FWD(Args, args)...);
        }
    }

    /// Returns **true** if this path isn't empty.
    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return !this->Empty();
    }

    /// Converts this path into its underlying string.
    constexpr VIOLET_IMPLICIT operator StringType() const noexcept
    {
        return this->Data();
    }

    /// Compares two paths of the same type by their strings. `!=` is rewritten from this.
    friend constexpr auto operator==(const Derived& lhs, const Derived& rhs) noexcept -> bool
    {
        return lhs.Data() == rhs.Data();
    }

    /// Orders two paths of the same type by their strings.
    NOELDOC_SINCE("26.11")
    friend constexpr auto operator<=>(const Derived& lhs, const Derived& rhs) noexcept
    {
        return lhs.Data() <=> rhs.Data();
    }

    /// Compares this path's string with anything it's equality comparable with (C-style strings, [`String`],
    /// [`Str`], ...). `!=` and the reversed forms are rewritten from this.
    ///
    /// ## Remarks
    /// `Self` is deduced and checked first so that the constraint short-circuits whenever ADL drags this
    /// operator into an unrelated comparison (i.e, `std::allocator<Path> == std::allocator<Path>`), otherwise
    /// `std::equality_comparable_with` re-enters this operator and the constraint depends on itself.
    template<typename Self, typename T>
        requires(std::same_as<Self, Derived> && !std::derived_from<std::remove_cvref_t<T>, BasePath>
            && std::equality_comparable_with<StringType, std::decay_t<T>>)
    friend constexpr auto operator==(const Self& self, const T& rhs) noexcept -> bool
    {
        return self.Data() == rhs;
    }

    /// Orders this path's string against anything it's three-way comparable with. `<`, `>`, `<=`, `>=`, and the
    /// reversed forms are rewritten from this.
    ///
    /// ## Remarks
    /// The constraint checks `std::decay_t<T>` so that string literals are treated as `const character*`; arrays
    /// aren't three-way comparable, and without the decay a literal would fall through to the built-in pointer
    /// comparison via [`PathRef`]'s conversion to `const character*`.
    template<typename Self, typename T>
        requires(std::same_as<Self, Derived> && !std::derived_from<std::remove_cvref_t<T>, BasePath>
            && std::three_way_comparable_with<StringType, std::decay_t<T>>)
    NOELDOC_SINCE("26.11")
    friend constexpr auto operator<=>(const Self& self, const T& rhs) noexcept
    {
        return self.Data() <=> rhs;
    }

    /// Shorthand for [`Join`](BasePath::Join): `base / "docs" / "file.txt"`.
    template<typename T>
        requires std::constructible_from<StringType, const T&>
    NOELDOC_SINCE("26.11")
    friend constexpr auto operator/(const BasePath& self, const T& rhs) noexcept
    {
        return self.Join(StringType(rhs));
    }

    friend auto operator<<(std::ostream& os, const Derived& self) -> std::ostream&
    {
        return os << self.Data();
    }

private:
    friend Derived;

#if VIOLET_PLATFORM(WINDOWS)
    constexpr static auto isDriveLetter(character ch) noexcept -> bool
    {
        return (ch >= 'A' && ch <= 'Z') || (ch >= 'a' && ch <= 'z');
    }
#endif

    constexpr auto getThisObject() noexcept -> Derived&
    {
        return static_cast<Derived&>(*this);
    }

    [[nodiscard]]
    constexpr auto getThisObject() const noexcept -> const Derived&
    {
        return static_cast<const Derived&>(*this);
    }
};

/// A non-owning, immutable view of a filesystem path.
///
/// ## Remarks
/// A `PathRef` doesn't own its string, so the string it was constructed from must outlive it.
struct VIOLET_API NOELDOC_SINCE("26.02") PathRef final: public BasePath<PathRef, Str> {
    VIOLET_DISALLOW_CONSTRUCTOR(PathRef);

    /// Constructs a path from a NUL-terminated string.
    /// @param data the path
    constexpr VIOLET_IMPLICIT PathRef(const character* data) noexcept
        : n_path(data)
    {
    }

    /// Constructs a path from a string view.
    /// @param data the path
    constexpr VIOLET_IMPLICIT PathRef(Str data) noexcept
        : n_path(data)
    {
    }

    /// Constructs a path from a string and its length.
    /// @param data the path
    /// @param length the number of characters in `data`
    constexpr VIOLET_EXPLICIT PathRef(const character* data, UInt length) noexcept
        : n_path(data, length)
    {
    }

    /// Borrows the string of `path`. `path` must outlive this `PathRef`.
    /// @param path the path to borrow
    VIOLET_IMPLICIT PathRef(const Path& path) noexcept;

    /// Disallowed: implicit conversion from a [`String`] makes it too easy to borrow a temporary. Pass `Str(str)`
    /// explicitly, or use a [`Path`].
    VIOLET_IMPLICIT PathRef(String) noexcept = delete;

    /// Disallowed: implicit conversion from a [`String`] makes it too easy to borrow a temporary. Pass `Str(str)`
    /// explicitly, or use a [`Path`].
    VIOLET_IMPLICIT PathRef(const String&) noexcept = delete;

    /// Returns a canonicalized copy of this path with redundant separators, `.`, and `..` components removed.
    ///
    /// ## Remarks
    /// This **will not** check the filesystem or resolve symlinks; it operates purely on the string. Use
    /// [`violet::filesystem::Canonicalize`] to resolve against the filesystem instead.
    ///
    /// ## Behaviour
    /// - `/usr/./local/../bin` -> `/usr/bin`
    /// - `/home/user/../docs/` -> `/home/docs`
    /// - `//usr///bin` -> `/usr/bin`
    /// - `/..` -> `/`
    /// - `../a/../b` -> `../b` (leading `..` in relative paths are kept)
    /// - `a/..` -> `.`
    /// - `` -> ``
    ///
    /// ## Example
    /// ```cpp
    /// PathRef path = "/usr/./local/../bin/clang";
    /// Path canon = path.Canonicalize();
    /// assert(canon == "/usr/bin/clang");
    /// ```
    [[nodiscard]]
    auto Canonicalize() const noexcept -> Path;

    /// Returns the underlying string view.
    constexpr VIOLET_IMPLICIT operator Str() const noexcept
    {
        return this->n_path;
    }

    /// Returns a pointer to the path's characters.
    ///
    /// ## Remarks
    /// The string is **not** guaranteed to be NUL-terminated (i.e, a `PathRef` over part of a larger string). Use
    /// [`WithCStr`](BasePath::WithCStr) when you need a C-style string.
    constexpr VIOLET_IMPLICIT operator const character*() const noexcept
    {
        return this->n_path.data();
    }

private:
    friend struct BasePath<PathRef, Str>;
    friend struct Path;

    [[nodiscard]]
    constexpr auto storage() const noexcept -> Str
    {
        return this->n_path;
    }

    Str n_path;
};

/// An owning, mutable filesystem path.
struct VIOLET_API NOELDOC_SINCE("26.02") Path final: public BasePath<Path, String> {
    /// Constructs an empty path.
    constexpr VIOLET_IMPLICIT Path() = default;

    /// Constructs a path from a NUL-terminated string.
    /// @param data the path
    constexpr VIOLET_IMPLICIT Path(const character* data)
        : n_path(data)
    {
    }

    /// Constructs a path that takes ownership of `data`.
    /// @param data the path
    constexpr VIOLET_IMPLICIT Path(String data)
        : n_path(VIOLET_MOVE(data))
    {
    }

    /// Constructs a path by copying a string view.
    /// @param data the path
    constexpr VIOLET_IMPLICIT Path(const Str& data)
        : n_path(data)
    {
    }

    /// Constructs a path from a string and its length.
    /// @param data the path
    /// @param length the number of characters in `data`
    constexpr VIOLET_EXPLICIT Path(const character* data, UInt length)
        : n_path(data, length)
    {
    }

    /// Constructs a path by copying the string of `path`.
    /// @param path the path to copy
    VIOLET_IMPLICIT Path(PathRef path) noexcept;

    /// Returns the path of an open file descriptor, as reported by the OS.
    /// @param descriptor the file descriptor to look up
    /// @return the path of `descriptor`, or an I/O error if it couldn't be resolved
    static auto FromFileDescriptor(const io::FileDescriptor& descriptor) -> io::Result<Path>;

    /// Canonicalizes this path in place, removing redundant separators, `.`, and `..` components.
    ///
    /// ## Remarks
    /// This **will not** check the filesystem or resolve symlinks; it operates purely on the string. Use
    /// [`violet::filesystem::Canonicalize`] to resolve against the filesystem instead. See
    /// [`PathRef::Canonicalize`] for the exact rules.
    ///
    /// ## Example
    /// ```cpp
    /// Path path = "/usr/./local/../bin/clang";
    /// path.Canonicalize();
    ///
    /// assert(path == "/usr/bin/clang");
    /// ```
    void Canonicalize() noexcept
    {
        this->n_path = path_internal::canonicalize(this->n_path);
    }

    /// Returns a copy of the underlying string.
    constexpr VIOLET_EXPLICIT operator String() const noexcept
    {
        return this->n_path;
    }

    /// Borrows this path as a [`PathRef`]. This path must outlive the returned view.
    VIOLET_EXPLICIT operator violet::filesystem::PathRef() const noexcept;

    /// Compares an owned path with a borrowed one by their strings. `!=` and the reversed forms are rewritten from
    /// this.
    constexpr friend auto operator==(const Path& lhs, const PathRef& rhs) noexcept -> bool
    {
        return lhs.n_path == rhs.n_path;
    }

    /// Orders an owned path against a borrowed one by their strings.
    NOELDOC_SINCE("26.11")
    constexpr friend auto operator<=>(const Path& lhs, const PathRef& rhs) noexcept
    {
        return lhs.n_path <=> rhs.n_path;
    }

private:
    friend struct BasePath<Path, String>;
    friend struct PathRef;

    [[nodiscard]]
    constexpr auto storage() const noexcept -> String
    {
        return this->n_path;
    }

    String n_path;
};

template<typename Derived, typename StringType>
    requires(path_internal::stdlib_string<StringType> || path_internal::violet_string_type<StringType>)
inline auto BasePath<Derived, StringType>::Join(strings::view_type<StringType> rhs) const -> Path
{
    if (path_internal::Empty(rhs)) {
        return {};
    }

    if (PathRef{rhs}.Absolute()) {
        return {rhs};
    }

    strings::owned_type<StringType> result(this->Data());
    if (!path_internal::Empty(result) && path_internal::Back(result) != PathSeparator) {
        path_internal::AppendBack(result, PathSeparator);
    }

    UInt start = 0;
    while (start < path_internal::Size(rhs) && path_internal::IsSeparator(rhs[start])) {
        start++;
    }

    path_internal::Append(result, path_internal::Substr(rhs, start));
    return {VIOLET_MOVE(result)};
}

template<typename Derived, typename StringType>
    requires(path_internal::stdlib_string<StringType> || path_internal::violet_string_type<StringType>)
inline auto BasePath<Derived, StringType>::WithFilename(strings::view_type<StringType> filename) const -> Path
{
    if (this->Empty()) {
        return {filename};
    }

    if (auto parent = this->Parent()) {
        return parent->Join(filename);
    }

#if VIOLET_PLATFORM(UNIX)
    if (this->Root()) {
        return this->Join(filename);
    }
#endif

    return {filename};
}

template<typename Derived, typename StringType>
    requires(path_internal::stdlib_string<StringType> || path_internal::violet_string_type<StringType>)
inline auto BasePath<Derived, StringType>::WithExtension(strings::view_type<StringType> ext) const -> Path
{
    auto stem = this->Stem();
    if (path_internal::Empty(stem)) {
        return {};
    }

    if (path_internal::Empty(ext)) {
        return this->WithFilename(stem);
    }

    const auto* dot = ext[0] == '.' ? "" : ".";
    return this->WithFilename(path_internal::Format<strings::owned_type<StringType>>("{}{}{}", stem, dot, ext));
}

inline PathRef::PathRef(const Path& path) noexcept
    : n_path(path.n_path)
{
}

inline Path::Path(PathRef path) noexcept
    : n_path(path.n_path)
{
}

inline auto PathRef::Canonicalize() const noexcept -> Path
{
    return {path_internal::canonicalize(this->n_path)};
}

inline Path::operator PathRef() const noexcept
{
    return {Str(this->n_path)};
}

} // namespace violet::filesystem

VIOLET_FORMATTER(violet::filesystem::Path);
VIOLET_FORMATTER(violet::filesystem::PathRef);
