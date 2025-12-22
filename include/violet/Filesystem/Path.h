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

#include "violet/Container/Optional.h"
#include "violet/IO/Descriptor.h"
#include "violet/IO/Error.h"
#include "violet/Violet.h"

#include <ostream>

namespace violet::filesystem {

#ifdef VIOLET_WINDOWS
constexpr static auto PathSeparator = '\\';
#else
constexpr static auto PathSeparator = '/';
#endif

namespace detail {

    template<typename StringType>
    constexpr auto computeTrailingSlashPosition(const StringType& val) noexcept -> Pair<Int64, StringType>
    {
        StringType copied = val;

        // trim trailing separators from the *copied* string
        while (!copied.empty() && (copied.back() == '/' || copied.back() == '\\')) {
            copied = copied.substr(0, copied.length() - 1);
        }

        Int64 pos = -1;
        for (auto i = static_cast<Int64>(copied.length()) - 1; i >= 0; i--) {
            char ch = copied[i];
            if (ch == '/' || ch == '\\') {
                pos = i;
                break;
            }
        }

        return Pair(pos, copied);
    }

    template auto computeTrailingSlashPosition(const String&) -> Pair<Int64, String>;
    template auto computeTrailingSlashPosition(const Str&) -> Pair<Int64, Str>;

    template<typename StringType>
    auto canonicalizeImpl(const StringType& path) -> String
    {
        String result;
        UInt index = 0;
        bool lastSep = false;

        while (index < path.size()) {
            // Skip any `/` or `\\`.
            if (path[index] == '/' || path[index] == '\\') {
                if (!lastSep)
                    result.push_back(PathSeparator);

                lastSep = true;
                index++;

                continue;
            }

            UInt start = index;
            while (index < path.size() && path[index] != '/' && path[index] != '\\') {
                index++;
            }

            Str part(&path[start], index - start);
            if (part == "..") {
                UInt pos = result.rfind(PathSeparator);
                if (pos != Str::npos) {
                    result.resize(pos);
                } else {
                    if (!result.empty()) {
                        result.push_back(PathSeparator);
                    } else {
                        result.append(part);
                    }
                }

                lastSep = false;
            } else if (part != ".") {
                if (!result.empty() && result.back() != PathSeparator) {
                    result.push_back(PathSeparator);
                } else {
                    result.append(part);
                }

                lastSep = false;
            }

            if (result.size() > 1 && result.back() == PathSeparator) {
                result.pop_back();
            }
        }

        return result;
    }

    template auto canonicalizeImpl(const String&) -> String;
    template auto canonicalizeImpl(const Str&) -> String;

} // namespace detail

struct Path;
struct PathRef;
// struct Ancestors;
// struct Components;

/// CRTP base class for path-like types.
///
/// This class provides common functionality for both immutable, non-owning paths
/// ([`PathRef`]) and owned, mutable paths ([`Path`]). It uses the CRTP (Curiously Recurring Template Pattern)
/// to allow compile-time re-use of methods while leaving `Derived`'s storage type to be flexible.
///
/// @tparam Derived The derived class
/// @tparam StringType The underling string type to use
template<typename Derived, typename StringType>
struct VIOLET_API BasePath {
    [[nodiscard]] constexpr auto Data() const noexcept -> StringType
    {
        return getThisObject().storage();
    }

    /// Returns **true** if this path is considered empty.
    [[nodiscard]] constexpr auto Empty() const noexcept -> bool
    {
        return Data().empty();
    }

    /// Returns **true** if this path is an absolute path.
    ///
    /// ## Platform-specific Remarks
    /// - Unix: Considered **true** if the path starts with `/`.
    /// - Windows: Considered **true** if it starts with a drive letter and colon (e.g. `"C:\\"`) or a UNC path
    /// (`"\\\\"`).
    [[nodiscard]] constexpr auto Absolute() const noexcept -> bool
    {
        if (Empty()) {
            return false;
        }

        StringType storage = Data();
#ifdef VIOLET_WINDOWS
        // drive letter (i.e, `C:\`)
        if (storage.size() >= 2 && std::isalpha(static_cast<UInt8>(storage[0])) && storage[0] == ':') {
            return true;
        }

        /// A UNC path like `\\server\share`
        if (storage.size() >= 2 && storage[0] == '\\' && storage[1] == '\\') {
            return true;
        }

        return false;
#else
        return storage[0] == '/';
#endif
    }

    /// Returns **true** if this path is considered relative.
    [[nodiscard]] constexpr auto Relative() const noexcept -> bool
    {
        return !this->Absolute();
    }

    /// Returns **true** if the path represents a root directory.
    ///
    /// ## Platform-specific Remarks
    /// - Unix: True if the path is `/`
    /// - Windows: True if the path is a drive root (i.e, `C:`)
    [[nodiscard]] constexpr auto Root() const noexcept -> bool
    {
        StringType storage = getThisObject().storage();

#ifdef VIOLET_WINDOWS
        return storage.size() == 2 && std::isalpha(storage[0]) && storage[1] == ':';
#else
        return storage == "/";
#endif
    }

    /// Returns the path as a string.
    [[nodiscard]] constexpr auto ToString() const noexcept -> String
    {
        return String(getThisObject().storage());
    }

    /// Returns the extension of the path's filename, if any.
    ///
    /// The extension is the portion of the filename after the last `.`. Leading dots
    /// are ignored for hidden files like `.gitignore`.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef p1("/home/noeltowa/file.txt");
    /// ASSERT_EQUALS(*p1.Extension(), "txt");
    ///
    /// PathRef p2("/home/noeltowa/file");
    /// ASSERT(!p2.Extension());
    ///
    /// PathRef p3("/home/noeltowa/.gitignore");
    /// ASSERT(!p1.Extension());
    /// ```
    [[nodiscard]] auto Extension() const noexcept -> Optional<String>;

    /// Returns the filename portion from this path.
    ///
    /// The filename is usually the last component of the path, after the final
    /// path separator (`/` on Unix, `\\` on Windows). If the path ends with a separator,
    /// the filename is considered ***empty***.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef p1("/home/noeltowa/Documents/loveletter.txt");
    /// ASSERT_EQUALS(p1.Filename(), "loveletter.txt");
    ///
    /// PathRef p2("/home/noeltowa/");
    /// ASSERT_EQUALS(p2.Filename(), "");
    /// ```
    [[nodiscard]] auto Filename() const noexcept -> String;

    /// Returns the **stem** portion of the path's filename.
    ///
    /// The **stem** is the filename without ***its extension***. Leading dots
    /// are preserved for hidden files like `.gitignore`.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef p1("/home/noeltowa/Documents/loveletter.txt");
    /// ASSERT_EQUALS(p1.Stem(), "loveletter");
    ///
    /// PathRef p2("/home/noeltowa/.gitignore");
    /// ASSERT_EQUALS(p2.Stem(), ".gitignore");
    /// ```
    [[nodiscard]] auto Stem() const noexcept -> String;

    /// Returns the **parent** path. If the path is considered empty, [`Nothing`] is returned.
    ///
    /// ## Remarks
    /// This method ***will not*** check the filesystem; it purely operates on the path string itself.
    ///
    /// ## Behaviour
    /// * `/home/user/docs/file.txt` -> `/home/user/docs`
    /// * `/home` -> `/`
    /// * `/` -> Nothing
    ///
    /// ### Windows
    /// - `C:\Windows\System32\cmd.exe` -> `C:\Windows\System32`
    /// - `C:\` -> Nothing
    ///
    /// ## Example
    /// ```cpp
    /// PathRef path = "/usr/local/bin";
    /// if (auto parent = path.Parent()) {
    ///     ASSERT_EQUALS(parent.Value(), "/usr/local");
    /// }
    /// ```
    [[nodiscard]] auto Parent() const noexcept -> Optional<Path>;

    /// Provides a new path with the last component replaced by `filename`. If the path has no parent, `filename`
    /// will be appended.
    ///
    /// ## Remarks
    /// This ***will not*** validate with the filesystem or the existence of the file itself and respects
    /// platform-specific path seperators.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef path = "/home/user/docs/report.txt";
    /// Path newPath = path.WithFilename("summary.txt");
    /// ASSERT_EQUALS(newPath, "/home/user/docs/summary.txt");
    /// ```
    [[nodiscard]] auto WithFilename(Str filename) const noexcept -> Path;

    /// Returns a new path with the extension of the filename replaced by `ext`. If the filename has no extension,
    /// then `ext` is appended. A leading `.` in `ext` is optional; it will be added if missing.
    ///
    /// ## Remarks
    /// * Using a blank `ext` will remove the extension from this file.
    /// * The method preserves the filename stem and parent path.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef path = "/home/noeltowa/Documents/loveletter.txt";
    /// Path newPath = path.WithExtension("md");
    /// ASSERT_EQUALS(newPath, "/home/noeltowa/Documents/loveletter.md");
    /// ```
    [[nodiscard]] auto WithExtension(Str ext) const noexcept -> Path;

    /// Joins another path or path segment to the current path, producing a new [`Path`]. If the argument is an absolute
    /// path, it'll replace the current path.
    ///
    /// ## Example
    /// ```cpp
    /// Path base = "/home/user";
    /// Path joined = base.Join("documents/report.txt");
    /// ASSERT_EQUALS(joined, "/home/user/documents/report.txt");
    ///
    /// Path absolute = "/etc";
    /// joined = base.Join("/etc/passwd");
    /// ASSERT_EQUALS(joined, "/etc/password");
    /// ```
    [[nodiscard]] auto Join(Str rhs) const noexcept -> Path;

#ifdef VIOLET_WINDOWS
    /// Allows converting this UTF-8 path into a UTF-16 wide string for Win32 APIs.
    ///
    /// ## Example
    /// ```cpp
    /// Path path = "C:\\Users\\NoelTowa\\Workspaces\\Noelware\\secrets.txt";
    /// std::wstring w = path.AsWideStr();
    /// CreateFileW(w.c_str(), /* ... */);
    /// ```
    [[nodiscard]] constexpr auto AsWideStr() const noexcept -> std::wstring;
#endif

    constexpr VIOLET_EXPLICIT operator bool() const noexcept
    {
        return !this->Empty();
    }

    constexpr VIOLET_EXPLICIT operator StringType() const noexcept
    {
        return getThisObject().storage();
    }

    constexpr auto operator==(const Derived& rhs) const noexcept -> bool
    {
        return getThisObject().storage() == rhs.storage();
    }

    constexpr auto operator!=(const Derived& rhs) const noexcept -> bool
    {
        return getThisObject().storage() != rhs.storage();
    }

    constexpr auto operator==(CStr rhs) const noexcept -> bool
    {
        return getThisObject().storage() == rhs;
    }

    constexpr auto operator!=(CStr rhs) const noexcept -> bool
    {
        return getThisObject().storage() != rhs;
    }

    constexpr auto operator==(const String& rhs) const noexcept -> bool
    {
        return getThisObject().storage() == rhs;
    }

    constexpr auto operator!=(const String& rhs) const noexcept -> bool
    {
        return getThisObject().storage() != rhs;
    }

    constexpr auto operator==(Str rhs) const noexcept -> bool
    {
        return getThisObject().storage() == rhs;
    }

    constexpr auto operator!=(Str rhs) const noexcept -> bool
    {
        return getThisObject().storage() != rhs;
    }

    constexpr auto operator<<(std::ostream& os) const noexcept -> std::ostream&
    {
        return os << getThisObject().storage();
    }

private:
    friend Derived;

    constexpr auto getThisObject() noexcept -> Derived&
    {
        return static_cast<Derived&>(*this);
    }

    [[nodiscard]] constexpr auto getThisObject() const noexcept -> const Derived&
    {
        return static_cast<const Derived&>(*this);
    }
};

/// A non-owning, immutable view of a filesystem path.
struct VIOLET_API PathRef final: public BasePath<PathRef, Str> {
    constexpr VIOLET_IMPLICIT PathRef() = delete;

    /// Constructs a path from a C-style string.
    /// @param data the path to register
    constexpr VIOLET_IMPLICIT PathRef(CStr data)
        : n_path(data)
    {
    }

    /// Disallow constructing a [`PathRef`] from a string.
    constexpr VIOLET_IMPLICIT PathRef(String) = delete;

    /// Disallow constructing a [`PathRef`] from a string.
    constexpr VIOLET_IMPLICIT PathRef(const String&) = delete;

    /// Constructs a path from a copied [`std::string_view`].
    /// @param data the path to register
    constexpr VIOLET_IMPLICIT PathRef(Str data)
        : n_path(data)
    {
    }

    /// Constructs a path from a C-style string (with length).
    /// @param data the path to register
    /// @param length length of the cstr
    constexpr VIOLET_EXPLICIT PathRef(CStr data, UInt length)
        : n_path(data, length)
    {
    }

    /// Constructs a [`Path`] from a file descriptor.
    /// @param descriptor file descriptor from a file that was passed down from the OS.
    /// @returns I/O result of the found path from the descriptor, or an error.
    static auto FromFileDescriptor(io::FileDescriptor descriptor) -> io::Result<Path>;

    /// Canonicalizes this path by removing redundant `.` and `..` components, as well as consecutive path separators.
    ///
    /// ## Remarks
    /// This ***will not** check the filesystem or resolve symlinks; it operates purely on the string representation.
    /// Use the [`violet::filesystem::Canonicalize`] method to check the filesystem or resolve symlinks instead.
    ///
    /// ## Example
    /// ```cpp
    /// PathRef path = "/usr/./local/../bin/clang";
    /// Path canon = path.Canonicalize();
    /// ASSERT_EQUALS(canon, "/usr/bin/clang");
    /// ```
    ///
    /// ## Behaviour
    /// * `/usr/.local/../bin` -> `/usr/bin`
    /// * `/home/user/../docs/` -> `/home/docs`
    /// * `C:\Users\.\NoelTowa\..\Documents` -> `C:\Users\Documents`
    ///
    /// ## Notes
    /// * Consecutive slashes (//) are reduced to a single separator.
    /// * The method preserves the root directory and drive prefixes.
    /// * If the path is empty, it remains empty.
    [[nodiscard]] auto Canonicalize() const noexcept -> Path;

    constexpr VIOLET_EXPLICIT operator Str()
    {
        return this->storage();
    }

    constexpr VIOLET_EXPLICIT operator CStr()
    {
        return this->storage().data();
    }

private:
    friend struct BasePath<PathRef, Str>;
    friend struct Path;

    Str n_path;

    [[nodiscard]] constexpr auto storage() const noexcept -> Str
    {
        return this->n_path;
    }
};

/// A owning, mutable filesystem path.
struct VIOLET_API Path final: public BasePath<Path, String> {
    constexpr VIOLET_IMPLICIT Path() = default;

    /// Constructs a path from a C-style string.
    /// @param data the path to register
    constexpr VIOLET_IMPLICIT Path(CStr data)
        : n_path(data)
    {
    }

    /// Constructs a path from a [`std::string`].
    /// @param data the path to register
    constexpr VIOLET_IMPLICIT Path(String data)
        : n_path(VIOLET_MOVE(data))
    {
    }

    /// Constructs a path from a copied [`std::string_view`].
    /// @param data the path to register
    constexpr VIOLET_IMPLICIT Path(const Str& data)
        : n_path(data)
    {
    }

    /// Constructs a path from a C-style string (with length).
    /// @param data the path to register
    /// @param length length of the cstr
    constexpr VIOLET_EXPLICIT Path(CStr data, UInt length)
        : n_path(data, length)
    {
    }

    /// Constructs a [`Path`] from a file descriptor.
    /// @param descriptor file descriptor from a file that was passed down from the OS.
    /// @returns I/O result of the found path from the descriptor, or an error.
    static auto FromFileDescriptor(io::FileDescriptor descriptor) -> io::Result<Path>;

    /// Canonicalizes this path by removing redundant `.` and `..` components, as well as consecutive path separators.
    ///
    /// ## Remarks
    /// This method will update this path's string instance to the new canonicalized path.
    ///
    /// This ***will not** check the filesystem or resolve symlinks; it operates purely on the string representation.
    /// Use the [`violet::filesystem::Canonicalize`] method to check the filesystem or resolve symlinks instead.
    ///
    /// ## Example
    /// ```cpp
    /// Path path = "/usr/./local/../bin/clang";
    /// path.Canonicalize();
    ///
    /// ASSERT_EQUALS(path, "/usr/bin/clang");
    /// ```
    ///
    /// ## Behaviour
    /// * `/usr/.local/../bin` -> `/usr/bin`
    /// * `/home/user/../docs/` -> `/home/docs`
    /// * `C:\Users\.\NoelTowa\..\Documents` -> `C:\Users\Documents`
    ///
    /// ## Notes
    /// * Consecutive slashes (//) are reduced to a single separator.
    /// * The method preserves the root directory and drive prefixes.
    /// * If the path is empty, it remains empty.
    void Canonicalize() noexcept
    {
        this->n_path = detail::canonicalizeImpl(this->n_path);
    }

    constexpr VIOLET_EXPLICIT operator String() const noexcept
    {
        return this->storage();
    }

    VIOLET_EXPLICIT operator violet::filesystem::PathRef() const noexcept;

private:
    friend struct BasePath<Path, String>;
    friend struct PathRef;

    friend auto operator==(const Path& lhs, const PathRef& rhs) noexcept -> bool
    {
        return lhs.n_path == rhs.n_path;
    }

    friend auto operator!=(const Path& lhs, const PathRef& rhs) noexcept -> bool
    {
        return lhs.n_path != rhs.n_path;
    }

    String n_path;

    [[nodiscard]] auto storage() const noexcept -> String
    {
        return this->n_path;
    }

    [[nodiscard]] auto storage() noexcept -> String
    {
        return this->n_path;
    }
};

template<typename Derived, typename StringType>
auto BasePath<Derived, StringType>::Filename() const noexcept -> String
{
    if (Empty()) {
        return {};
    }

    auto [pos, val] = detail::computeTrailingSlashPosition(getThisObject().storage());
    if (pos == -1) {
        return String(val);
    }

    return String(val.substr(pos + 1));
}

template<typename Derived, typename StringType>
auto BasePath<Derived, StringType>::Extension() const noexcept -> Optional<String>
{
    if (Empty()) {
        return Nothing;
    }

    String filename = this->Filename();
    if (filename.empty()) {
        return Nothing;
    }

    Int64 pos = -1;
    for (auto i = static_cast<Int64>(filename.length()) - 1; i >= 0; i--) {
        if (filename[i] == '.') {
            pos = i;
            break;
        }
    }

    return pos == -1 || pos == 0 ? Nothing : Some<String>(filename.substr(pos + 1));
}

template<typename Derived, typename StringType>
auto BasePath<Derived, StringType>::Stem() const noexcept -> String
{
    String filename = this->Filename();
    if (filename.empty()) {
        return { filename };
    }

    UInt dotpos = filename.rfind('.');
    if (dotpos == String::npos) {
        return { filename };
    }

    // edge case: '.gitignore', .bazelignore, etc.
    //
    // if pos == 0 (only finds one `.`) and the first character
    // in `filename` == `.`
    if (dotpos == 0 && filename[0] == '.') {
        return { filename };
    }

    return String(filename.substr(0, dotpos));
}

template<typename Derived, typename StringType>
auto BasePath<Derived, StringType>::Parent() const noexcept -> Optional<Path>
{
    if (Empty()) {
        return Nothing;
    }

    auto [pos, val] = detail::computeTrailingSlashPosition(getThisObject().storage());
    if (pos == -1) {
        return Nothing;
    }

#ifdef VIOLET_WINDOWS
    if (pos == 2 && val[1] == ':') {
        return Some<Path>(val.substr(0, pos + 1));
    }
#else
    if (pos == 0) {
        return Some<Path>("/");
    }
#endif

    return Some<Path>(val.substr(0, pos));
}

template<typename Derived, typename StringType>
auto BasePath<Derived, StringType>::WithFilename(Str filename) const noexcept -> Path
{
    if (Empty()) {
        return filename;
    }

    if (auto parent = this->Parent()) {
        return parent->Join(filename);
    }

#ifdef VIOLET_UNIX
    // edge case: `n_value` == '/' (indicating that we are root on Unix) and
    // intentionally, `this->Parent()` will return `Nothing` if we are root
    // regardless.
    if (getThisObject().storage() == "/") {
        return { std::format("/{}", Str(filename)) };
    }
#endif

    return filename;
}

template<typename Derived, typename StringType>
auto BasePath<Derived, StringType>::WithExtension(Str ext) const noexcept -> Path
{
    String stem = this->Stem();
    if (stem.empty()) {
        return {};
    }

    String final = String(ext);
    if (!ext.empty() && ext[0] != '.') {
        final = std::format(".{}", ext);
    }

    return this->WithFilename(std::format("{}{}", stem, final));
}

template<typename Derived, typename StringType>
auto BasePath<Derived, StringType>::Join(Str rhs) const noexcept -> Path
{
    if (rhs.empty()) {
        return {};
    }

    Path newPath(rhs);
    if (newPath.Absolute()) {
        return newPath;
    }

    String result = String(getThisObject().storage());
    if (!result.empty() && result.back() != PathSeparator) {
        result.push_back(PathSeparator);
    }

    UInt start = 0;
    while (start < rhs.length() && (rhs[start] == '/' || rhs[start] == '\\')) {
        start++;
    }

    result.append(rhs.substr(start));
    return { VIOLET_MOVE(result) };
}

inline auto PathRef::Canonicalize() const noexcept -> Path
{
    return { detail::canonicalizeImpl(this->n_path) };
}

inline Path::operator PathRef() const noexcept
{
    return { Str(this->n_path) };
}

} // namespace violet::filesystem

VIOLET_FORMATTER(violet::filesystem::Path);
VIOLET_FORMATTER(violet::filesystem::PathRef);
