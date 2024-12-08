// The "betterfile" library written in c++.
//
// Web: https://github.com/JaderoChan/betterfile
// You can contact me at: c_dl_cn@outlook.com
//
// MIT License
//
// Copyright (c) 2024 頔珞JaderoChan
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
// SOFTWARE.

// Usage:
// 1. The betterfile is a header-only library, so you don't need to build it.
// 2. You can use this library by only including the betterfile.hpp file in your
// project.
// 3. You can also make a new .cpp file to your project and add the code below.
// (This method's aim is detach the implementation from the header file, can be
// avoid namespace pollution)
//
// #define BTF_IMPL
// #include <betterfile.hpp>
//
// Last you need add the code below to your other location which need use this
// library.
//
// #define BTF_FWD
// #include <betterfile.hpp>
//

#ifndef BETTERFILE_HPP
#define BETTERFILE_HPP

#include <cstddef> // size_t
#include <fstream>
#include <iostream>
#include <sstream> // std::stringstream
#include <stdexcept>
#include <string>
#include <vector>

// Compiler version.
#ifdef _MSVC_LANG
    #define _BETTERFILE_CPPVERS _MSVC_LANG
#else
    #define _BETTERFILE_CPPVERS __cplusplus
#endif // _MSVC_LANG

#if _BETTERFILE_CPPVERS < 201103L
    #error "The betterfile library just useable in c++11 and above."
#endif // _BETTERFILE_CPPVERS < 201103L

// Check C++17 support.
#if _BETTERFILE_CPPVERS >= 201703L
    #define _BETTERFILE_CPP17
#endif // BETTERFILE_CPPVERS >= 201703L

#ifdef BTF_IMPL
    #define BTF_API
#else
    #ifdef BTF_FWD
        #define BTF_API extern
    #else
        #define BTF_API inline
    #endif // BTF_FWD
#endif     // BTF_IMPL

// Betterfile namespace.
namespace btf
{

// Just for the intellisense better show "tip about namespace". :)

}

// Type alias, enum and constants.
namespace btf
{

using uchar = unsigned char;
using uint = unsigned int;

template <typename T>
using Vec = std::vector<T>;
using String = std::string;
using Strings = Vec<String>;
using Exception = std::runtime_error;

constexpr uint _BUFFER_SIZE = 4096;

// Preferred path separator.
constexpr char WIN_PATH_SEPARATOR = '\\';
constexpr char LINUX_PATH_SEPARATOR = '/';
#ifdef _WIN32
constexpr char PREFERRED_PATH_SEPARATOR = WIN_PATH_SEPARATOR;
#else
constexpr char PREFERRED_PATH_SEPARATOR = LINUX_PATH_SEPARATOR;
#endif // _WIN32

// The invalid characters in filename.
constexpr const char* FILENAME_INVALID_CHARS = "\\/:*?\"<>|";

} // namespace btf

// Utility functions with not filesystem.
namespace btf
{

// @brief Concatenate two paths.
inline String pathcat(const String& path1, const String& path2)
{
    return path1 + PREFERRED_PATH_SEPARATOR + path2;
}

// @brief Concatenate multiple paths.
template <typename... Args>
String pathcat(const String& path1, const String& path2, Args&&... paths)
{
    if (sizeof...(paths) == 0)
        return pathcat(path1, path2);
    else
        return pathcat(pathcat(path1, path2), std::forward<Args>(paths)...);
}

// @brief Check if the filename is valid.
inline bool isValidFilename(const String& filename)
{
    // Filename can't be empty.
    if (filename.empty())
        return false;

    // Filename can't be "." or "..".
    if (filename == "." || filename == "..")
        return false;

    // Filename can't contain invalid characters.
    if (filename.find_first_of(FILENAME_INVALID_CHARS) != String::npos)
        return false;

    return true;
}

inline String quotePath(const String& path)
{
    return "\"" + path + "\"";
}

template <typename T>
String _fmt(const String& fmt, const T& arg)
{
    std::stringstream ss;

    if (fmt.size() < 4) {
        size_t pos = fmt.find("{}");
        if (pos == String::npos)
            return fmt;

        ss << fmt.substr(0, pos);
        ss << arg;

        return ss.str() + fmt.substr(pos + 2);
    }

    String window(4, '\0');
    for (size_t i = 0; i < fmt.size();) {
        window[0] = fmt[i];
        window[1] = i < fmt.size() - 1 ? fmt[i + 1] : '\0';
        window[2] = i < fmt.size() - 2 ? fmt[i + 2] : '\0';
        window[3] = i < fmt.size() - 3 ? fmt[i + 3] : '\0';

        if (window == "{{}}") {
            ss << "{}";
            i += 4;
            continue;
        }

        if (window[0] == '{' && window[1] == '}') {
            ss << arg;
            return ss.str() + fmt.substr(i + 2);
        } else {
            ss << window[0];
            i += 1;
            continue;
        }
    }

    return ss.str();
}

template <typename T, typename... Args>
String _fmt(const String& fmt, const T& arg, Args&&... args)
{
    std::stringstream ss;

    if (fmt.size() < 4) {
        size_t pos = fmt.find("{}");
        if (pos == String::npos)
            return fmt;

        ss << fmt.substr(0, pos);
        ss << arg;

        return ss.str() + fmt.substr(pos + 2);
    }

    String window(4, '\0');
    for (size_t i = 0; i < fmt.size();) {
        window[0] = fmt[i];
        window[1] = i < fmt.size() - 1 ? fmt[i + 1] : '\0';
        window[2] = i < fmt.size() - 2 ? fmt[i + 2] : '\0';
        window[3] = i < fmt.size() - 3 ? fmt[i + 3] : '\0';

        if (window == "{{}}") {
            ss << "{}";
            i += 4;
            continue;
        }

        if (window[0] == '{' && window[1] == '}') {
            ss << arg;
            return ss.str() + _fmt(fmt.substr(i + 2), std::forward<Args>(args)...);
        } else {
            ss << window[0];
            i += 1;
            continue;
        }
    }

    return ss.str();
}

} // namespace btf

#ifdef _BETTERFILE_CPP17
    #ifndef BTF_FWD
        #include <filesystem>

namespace btf
{

namespace fs = std::filesystem;

}
    #endif // !BTF_FWD
#else
    #ifndef BTF_FWD
        #include <ghc/filesystem.hpp>

namespace btf
{

namespace fs = ghc::filesystem;

}
    #endif // !BTF_FWD
#endif     // _BETTERFILE_CPP17

// Declaration of utility functions with filesystem.
namespace btf
{

#ifndef BTF_IMPL

BTF_API String normalize(const String& path);

BTF_API String currentPath();

// @example "C:/path/to/file.txt" -> "C:/path/to"
BTF_API String parentPath(const String& path);

// @example "C:/path/to/file.txt" -> "to"
BTF_API String parentName(const String& path);

// @example "C:/path/to/file.txt" -> "file.txt"
BTF_API String filenameEx(const String& path);

// @example "C:/path/to/file.txt" -> "file"
BTF_API String filename(const String& path);

// @example "C:/path/to/file.txt" -> ".txt"
BTF_API String extension(const String& path);

BTF_API bool isExists(const String& path);

// @return If the path is exists and path target is a regular file return true,
// else return false.
BTF_API bool isFile(const String& path);

// @return If the path is exists and path target is a directory return true,
// else return false.
BTF_API bool isDirectory(const String& path);

// @return If the path is exists and path target is a symlink return true, else
// return false.
BTF_API bool isSymlink(const String& path);

// @return If the path is a empty file or directory return true, else return
// false.
// @note If the path is not exists, throw exception.
BTF_API bool isEmpty(const String& path);

BTF_API bool isSubPath(const String& path, const String& base);

BTF_API bool isRelative(const String& path);

BTF_API bool isAbsolute(const String& path);

// @brief Get the relative path from the base path.
BTF_API String relative(const String& path, const String& base = currentPath());

BTF_API String absolute(const String& path);

BTF_API bool isEqualPath(const String& path1, const String& path2);

// @brief Check if filesystem entity (file, directory, symlink, hardlink) of two
// paths is equivalent.
BTF_API bool isEqualFileSystemEntity(const String& path1, const String& path2);

// @return The size of the file or directory.
BTF_API size_t sizes(const String& path);

// @brief Create a directory.
// @return If the directory is existed return false.
// @note The parent directory must exists.
BTF_API bool createDirectory(const String& path);

// @brief Create a directory tree.
// @return If the directory is existed return false.
// @note The parent directory can be not exists (create it automatically).
BTF_API bool createDirectorys(const String& path);

// @return The number of files and directories deleted.
BTF_API size_t deletes(const String& path);

BTF_API void copy(const String& src, const String& dst, bool isOverwrite = false);

BTF_API void copySymlink(const String& src, const String& dst, bool isOverwrite = false);

BTF_API void move(const String& src, const String& dst, bool isOverwrite = false);

BTF_API void reFilename(const String& path, const String& newFilename, bool isOverwrite = false);

BTF_API void reFilenameEx(const String& path, const String& newFilenameEx, bool isOverwrite = false);

BTF_API void reExtension(const String& path, const String& newExtension, bool isOverwrite = false);

BTF_API void createSymlink(const String& src, const String& dst, bool isOverwrite = false);

BTF_API String symlinkTarget(const String& path);

BTF_API void createHardlink(const String& src, const String& dst, bool isOverwrite = false);

BTF_API size_t hardlinkCount(const String& path);

BTF_API String tempDirectory();

BTF_API std::pair<Strings, Strings> getAlls(const String& path, bool isRecursive = true,
                                            bool (*filter)(const String&) = nullptr);

BTF_API Strings getAllFiles(const String& path, bool isRecursive = true, bool (*filter)(const String&) = nullptr);

BTF_API Strings getAllDirectorys(const String& path, bool isRecursive = true, bool (*filter)(const String&) = nullptr);

#endif // !BTF_IMPL

} // namespace btf

// Implementation of utility functions with filesystem.
namespace btf
{

#ifndef BTF_FWD

using _pth = fs::path;

BTF_API String normalize(const String& path)
{
    return _pth(path).lexically_normal().generic_string();
}

BTF_API String currentPath()
{
    return fs::current_path().string();
}

BTF_API String parentPath(const String& path)
{
    return _pth(path).parent_path().string();
}

BTF_API String parentName(const String& path)
{
    return _pth(path).parent_path().filename().string();
}

BTF_API String filenameEx(const String& path)
{
    return _pth(path).filename().string();
}

BTF_API String filename(const String& path)
{
    return _pth(path).filename().replace_extension().string();
}

BTF_API String extension(const String& path)
{
    return _pth(path).extension().string();
}

BTF_API bool isExists(const String& path)
{
    return fs::exists(path);
}

BTF_API bool isFile(const String& path)
{
    return fs::exists(path) && fs::is_regular_file(path);
}

BTF_API bool isDirectory(const String& path)
{
    return fs::exists(path) && fs::is_directory(path);
}

BTF_API bool isSymlink(const String& path)
{
    return fs::is_symlink(path);
}

BTF_API bool isEmpty(const String& path)
{
    return fs::is_empty(path);
}

BTF_API bool isSubPath(const String& path, const String& base)
{
    String path_ = normalize(absolute(path));
    String base_ = normalize(absolute(base));

    return path_.substr(0, base_.size()) == base_;
}

BTF_API bool isRelative(const String& path)
{
    return _pth(path).is_relative();
}

BTF_API bool isAbsolute(const String& path)
{
    return _pth(path).is_absolute();
}

BTF_API String relative(const String& path, const String& base)
{
    return fs::relative(path, base).string();
}

BTF_API String absolute(const String& path)
{
    return fs::absolute(path).string();
}

BTF_API bool isEqualPath(const String& path1, const String& path2)
{
    String path1_ = normalize(absolute(path1));
    String path2_ = normalize(absolute(path2));

    return path1_ == path2_;
}

BTF_API bool isEqualFileSystemEntity(const String& path1, const String& path2)
{
    return fs::equivalent(path1, path2);
}

BTF_API size_t sizes(const String& path)
{
    if (isFile(path)) {
        return fs::file_size(path);
    } else if (isDirectory(path)) {
        size_t rslt = 0;

        for (const auto& var : fs::recursive_directory_iterator(path))
            rslt += (var.is_regular_file() ? var.file_size() : 0);

        return rslt;
    } else {
        throw Exception(_fmt("The specify path not exists. \"{}\"", path));
    }
}

BTF_API bool createDirectory(const String& path)
{
    return fs::create_directory(path);
}

BTF_API bool createDirectorys(const String& path)
{
    return fs::create_directories(path);
}

// @return If the path not exists, return 0.
// @note Even if the path not exists not throw exception.
BTF_API size_t deletes(const String& path)
{
    return fs::remove_all(path);
}

BTF_API void copy(const String& src, const String& dst, bool isOverwrite)
{
    // If the source path equals the destination path, do nothing.
    if (isEqualPath(src, dst))
        return;

    if (isFile(src)) {
        // If the destination path exists same name file or directory and
        // specify not overwrite, do nothing.
        if (!isOverwrite && isExists(dst))
            return;

        // If the destination path has a same name directory (not file), throw
        // exception.
        if (isDirectory(dst))
            throw Exception(_fmt("The destination path contains same name "
                                 "directory. \"{}\" -> \"{}\"",
                                 src, dst));

        // Create the parent directory of the destination path first if not
        // exists.
        if (!isDirectory(parentPath(dst)))
            createDirectorys(parentPath(dst));

        // If the destination path is a file, delete it first.
        // #deletes can automatically handle the case of not exists deleted
        // file.
        deletes(dst);
        fs::copy_file(src, dst);
    } else if (isDirectory(src)) {
        // If the destination path has a same name file (not directory), throw
        // exception.
        if (isFile(dst))
            throw Exception(_fmt("The destination path contains same name "
                                 "file. \"{}\" -> \"{}\"",
                                 src, dst));

        // If attempt to copy a directory to a subdirectory, throw exception.
        if (isSubPath(dst, src))
            throw Exception(_fmt("Can't copy directory to a subdirectory. \"{}\" -> \"{}\"", src, dst));

        // For each file in the source directory, copy it to the destination
        // directory.
        for (const auto& var : fs::recursive_directory_iterator(src)) {
            if (var.is_regular_file())
                copy(var.path().string(), pathcat(dst, var.path().string().substr(src.size())), isOverwrite);
            else if (var.is_directory())
                createDirectorys(pathcat(dst, var.path().string().substr(src.size())));
        }
    } else {
        throw Exception(_fmt("The specify source path not exists. \"{}\"", src));
    }
}

BTF_API void copySymlink(const String& src, const String& dst, bool isOverwrite)
{
    if (!isOverwrite && isExists(dst))
        return;

    deletes(dst);
    fs::copy_symlink(src, dst);
}

BTF_API void move(const String& src, const String& dst, bool isOverwrite)
{
    // If the source path equals the destination path, do nothing.
    if (isEqualPath(src, dst))
        return;

    if (isFile(src)) {
        // If the destination path exists same name file or directory and
        // specify not overwrite, do nothing.
        if (!isOverwrite && isExists(dst))
            return;

        // If the destination path has a same name directory (not file), throw
        // exception.
        if (isDirectory(dst))
            throw Exception(_fmt("The destination path contains same name "
                                 "directory. \"{}\" -> \"{}\"",
                                 src, dst));

        // Create the parent directory of the destination path first if not
        // exists.
        if (!isDirectory(parentPath(dst)))
            createDirectorys(parentPath(dst));

        fs::rename(src, dst);
    } else if (isDirectory(src)) {
        // If the destination path has a same name file (not directory), throw
        // exception.
        if (isFile(dst))
            throw Exception(_fmt("The destination path contains same name "
                                 "file. \"{}\" -> \"{}\"",
                                 src, dst));

        // If attempt to move a directory to a subdirectory, throw exception.
        if (isSubPath(dst, src))
            throw Exception(_fmt("Can't move directory to a subdirectory. \"{}\" -> \"{}\"", src, dst));

        createDirectorys(dst);

        for (const auto& var : fs::directory_iterator(src)) {
            if (var.is_regular_file()) {
                move(var.path().string(), pathcat(dst, var.path().string().substr(src.size())), isOverwrite);
            } else if (var.is_directory()) {
                move(var.path().string(), pathcat(dst, var.path().string().substr(src.size())), isOverwrite);
                if (fs::is_empty(var.path()))
                    fs::remove(var.path());
            }
        }

        if (fs::is_empty(src))
            fs::remove(src);
    } else {
        throw Exception(_fmt("The specify source path not exists. \"{}\"", src));
    }
}

BTF_API void reFilename(const String& path, const String& newFilename, bool isOverwrite)
{
    auto dst = pathcat(parentPath(path), newFilename + extension(path));
    move(path, dst, isOverwrite);
}

BTF_API void reFilenameEx(const String& path, const String& newFilenameEx, bool isOverwrite)
{
    auto dst = pathcat(parentPath(path), newFilenameEx);
    move(path, dst, isOverwrite);
}

BTF_API void reExtension(const String& path, const String& newExtension, bool isOverwrite)
{
    auto dst = pathcat(parentPath(path), filename(path) + newExtension);
    move(path, dst, isOverwrite);
}

BTF_API void createSymlink(const String& src, const String& dst, bool isOverwrite)
{
    // If the source path equals the destination path, do nothing.
    if (isEqualPath(src, dst))
        return;

    // If the destination path exists same name file or directory and specify
    // not overwrite, do nothing.
    if (!isOverwrite && isExists(dst))
        return;

    if (isFile(src)) {
        // If the destination path has a same name directory (not file), throw
        // exception.
        if (isDirectory(dst))
            throw Exception(_fmt("The destination path contains same name "
                                 "directory. \"{}\" -> \"{}\"",
                                 src, dst));
    } else if (isDirectory(src)) {
        // If the destination path has a same name file (not directory), throw
        // exception.
        if (isFile(dst))
            throw Exception(_fmt("The destination path contains same name "
                                 "file. \"{}\" -> \"{}\"",
                                 src, dst));
    } else {
        throw Exception(_fmt("The specify path not exists. \"{}\"", src));
    }

    deletes(dst);

    // Create the parent directory of the destination path first if not exists.
    if (!isDirectory(parentPath(dst)))
        createDirectorys(parentPath(dst));

    if (isFile(src))
        fs::create_symlink(src, dst);
    else if (isDirectory(src))
        fs::create_directory_symlink(src, dst);
}

BTF_API String symlinkTarget(const String& path)
{
    return fs::read_symlink(path).string();
}

BTF_API void createHardlink(const String& src, const String& dst, bool isOverwrite)
{
    // If the source path equals the destination path, do nothing.
    if (isEqualPath(src, dst))
        return;

    if (isFile(src)) {
        if (!isOverwrite && isExists(dst))
            return;

        // If the destination path has a same name directory (not file), throw
        // exception.
        if (isDirectory(dst))
            throw Exception(_fmt("The destination path contains same name "
                                 "directory. \"{}\" -> \"{}\"",
                                 src, dst));

        deletes(dst);

        // Create the parent directory of the destination path first if not
        // exists.
        if (!isDirectory(parentPath(dst)))
            createDirectorys(parentPath(dst));

        fs::create_hard_link(src, dst);
    } else if (isDirectory(src)) {
        throw Exception(_fmt("Can't hardlink for directory. \"{}\"", src));
    } else {
        throw Exception(_fmt("The specify path not exists. \"{}\"", src));
    }
}

BTF_API size_t hardlinkCount(const String& path)
{
    return fs::hard_link_count(path);
}

BTF_API String tempDirectory()
{
    return fs::temp_directory_path().string();
}

BTF_API std::pair<Strings, Strings> getAlls(const String& path, bool isRecursive, bool (*filter)(const String&))
{
    if (!isDirectory(path))
        throw Exception(_fmt("The specify path is not directory or not exists. \"{}\"", path));

    Strings files;
    Strings dirs;

    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_regular_file() && (filter == nullptr || filter(_path)))
                files.push_back(_path);

            if (var.is_directory() && (filter == nullptr || filter(_path)))
                dirs.push_back(_path);
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_regular_file() && (filter == nullptr || filter(_path)))
                files.push_back(_path);

            if (var.is_directory() && (filter == nullptr || filter(_path)))
                dirs.push_back(_path);
        }
    }

    return { files, dirs };
}

BTF_API Strings getAllFiles(const String& path, bool isRecursive, bool (*filter)(const String&))
{
    if (!isDirectory(path))
        throw Exception(_fmt("The specify path is not directory or not exists. \"{}\"", path));

    Strings files;

    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_regular_file() && (filter == nullptr || filter(_path)))
                files.push_back(_path);
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_regular_file() && (filter == nullptr || filter(_path)))
                files.push_back(_path);
        }
    }

    return files;
}

BTF_API Strings getAllDirectorys(const String& path, bool isRecursive, bool (*filter)(const String&))
{
    if (!isDirectory(path))
        throw Exception(_fmt("The specify path is not directory or not exists. \"{}\"", path));

    Strings dirs;

    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_directory() && (filter == nullptr || filter(_path)))
                dirs.push_back(_path);
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String _path = var.path().string();

            if (var.is_directory() && (filter == nullptr || filter(_path)))
                dirs.push_back(_path);
        }
    }

    return dirs;
}

#endif // !BTF_FWD

} // namespace btf

// Classes.
namespace btf
{

#ifndef BTF_IMPL

class File
{
public:
    File() = default;

    explicit File(const String& name) { setName(name); }

    File(const File& other)
    {
        name_ = other.name_;

        if (other.data_)
            data_ = new String(*other.data_);
    }

    File(File&& other) noexcept
    {
        name_ = other.name_;

        data_ = other.data_;
        other.data_ = nullptr;
    }

    ~File() { releaseData(); }

    static File fromDiskPath(const String& filename)
    {
        std::ifstream ifs(filename, std::ios_base::binary);

        if (!ifs.is_open())
            throw Exception(_fmt("Failed to open the file: \"{}\"", filename));

        File file(filenameEx(filename));
        file << ifs;

        ifs.close();

        return file;
    }

    String name() const { return name_; }

    String data() const
    {
        if (data_ == nullptr)
            return "";
        return *data_;
    }

    size_t size() const
    {
        if (data_ == nullptr)
            return 0;
        return data_->size();
    }

    bool empty() const { return size() == 0; }

    void setName(const String& name)
    {
        if (!isValidFilename(name))
            throw Exception(_fmt("Invalid file name: \"{}\"", name));

        name_ = name;
    }

    void releaseData()
    {
        if (data_ == nullptr)
            return;

        delete data_;
        data_ = nullptr;
    }

    void write(std::ostream& os) const
    {
        if (data_ == nullptr)
            return;

        os << *data_;
    }

    void write(const String& path, bool isOverwrite = false,
               std::ios_base::openmode openmode = std::ios_base::binary) const
    {
        String _path = path + PREFERRED_PATH_SEPARATOR + name_;

        if (!isOverwrite && isFile(_path))
            return;

        std::ofstream ofs(_path.data(), openmode);

        if (!ofs.is_open())
            throw Exception(_fmt("Failed to open the file: \"{}\"", _path));

        write(ofs);

        ofs.close();
    }

    File copy() const { return File(*this); }

    File& operator=(const File& other)
    {
        name_ = other.name_;

        releaseData();

        if (other.data_)
            data_ = new String(*other.data_);

        return *this;
    }

    File& operator=(const String& data)
    {
        releaseData();

        data_ = new String(data);

        return *this;
    }

    template <typename T>
    File& operator=(const Vec<T>& data)
    {
        releaseData();

        data_ = new String;
        data_->reserve(data_->size() + data.size());

        for (const auto& var : data)
            data_->push_back(var);

        return *this;
    }

    File& operator<<(const File& other)
    {
        if (data_ == nullptr)
            data_ = new String;
        data_->append(other.data());

        return *this;
    }

    File& operator<<(std::istream& is)
    {
        is.seekg(0, std::ios_base::end);
        size_t size = is.tellg();
        is.seekg(0, std::ios_base::beg);

        if (data_ == nullptr)
            data_ = new String();
        data_->reserve(data_->size() + size);

        char buffer[_BUFFER_SIZE] = {};
        while (is.read(buffer, _BUFFER_SIZE))
            data_->append(String(buffer, is.gcount()));

        data_->append(String(buffer, is.gcount()));

        return *this;
    }

    File& operator<<(const String& data)
    {
        if (data_ == nullptr)
            data_ = new String();
        data_->append(data);

        return *this;
    }

    template <typename T>
    File& operator<<(const Vec<T>& data)
    {
        size_t size = data.size();

        if (data_ == nullptr)
            data_ = new String;
        data_->reserve(data_->size() + size);

        for (const auto& var : data)
            data_->push_back(var);

        return *this;
    }

    const File& operator>>(std::ostream& os) const
    {
        write(os);
        return *this;
    }

private:
    String name_;
    String* data_ = nullptr;
};

class Dir
{
public:
    Dir() = default;

    explicit Dir(const String& name) { setName(name); }

    Dir(const Dir& other)
    {
        name_ = other.name_;

        if (other.subFiles_)
            subFiles_ = new Vec<File>(*other.subFiles_);

        if (other.subDirs_)
            subDirs_ = new Vec<Dir>(*other.subDirs_);
    }

    Dir(Dir&& other) noexcept
    {
        name_ = other.name_;

        subFiles_ = other.subFiles_;
        other.subFiles_ = nullptr;

        subDirs_ = other.subDirs_;
        other.subDirs_ = nullptr;
    }

    ~Dir() { clear(); }

    static Dir fromDiskPath(const String& dirpath)
    {
        Dir root(filenameEx(dirpath));

        auto dirs = getAllDirectorys(dirpath, false);
        for (const auto& var : dirs)
            root << Dir::fromDiskPath(var);

        auto files = getAllFiles(dirpath, false);
        for (const auto& var : files)
            root << File::fromDiskPath(var);

        return root;
    }

    String name() const { return name_; }

    size_t size() const
    {
        size_t size = 0;

        if (subFiles_)
            for (const auto& var : *subFiles_)
                size += var.size();

        if (subDirs_)
            for (const auto& var : *subDirs_)
                size += var.size();

        return size;
    }

    size_t fileCount(bool isRecursive = true) const
    {
        size_t cnt = 0;

        if (subFiles_)
            cnt += subFiles_->size();

        if (isRecursive && subDirs_)
            for (const auto& var : *subDirs_)
                cnt += var.fileCount();

        return cnt;
    }

    size_t dirCount(bool isRecursive = true) const
    {
        size_t cnt = 0;

        if (subDirs_)
            cnt += subDirs_->size();

        if (isRecursive && subDirs_)
            for (const auto& var : *subDirs_)
                cnt += var.dirCount();

        return cnt;
    }

    size_t count(bool isRecursive = true) const { return fileCount(isRecursive) + dirCount(isRecursive); }

    bool empty() const { return size() == 0; }

    bool hasFile(const String& name, bool isRecursive = false) const
    {
        if (hasFile_(name) != NOF_)
            return true;

        if (isRecursive && subDirs_) {
            for (const auto& var : *subDirs_) {
                if (var.hasFile(name, true))
                    return true;
            }
        }

        return false;
    }

    bool hasDir(const String& name, bool isRecursive = false) const
    {
        if (hasDir_(name) != NOF_)
            return true;

        if (isRecursive && subDirs_) {
            for (const auto& var : *subDirs_) {
                if (var.hasDir(name, true))
                    return true;
            }
        }

        return false;
    }

    void setName(const String& name)
    {
        if (!isValidFilename(name))
            throw Exception(_fmt("Invalid file name: \"{}\"", name));

        name_ = name;
    }

    const Vec<File>& files() const { return *subFiles_; }

    const Vec<Dir>& dirs() const { return *subDirs_; }

    Vec<File>& files() { return *subFiles_; }

    Vec<Dir>& dirs() { return *subDirs_; }

    File& file(const String& name)
    {
        size_t pos = hasFile_(name);

        if (pos == NOF_) {
            add(File(name));
            return subFiles_->back();
        }

        return (*subFiles_)[pos];
    }

    Dir& dir(const String& name)
    {
        size_t pos = hasDir_(name);

        if (pos == NOF_) {
            add(Dir(name));
            return subDirs_->back();
        }

        return (*subDirs_)[pos];
    }

    void removeFile(const String& name)
    {
        size_t pos = hasFile_(name);

        if (pos == NOF_)
            return;

        subFiles_->erase(subFiles_->begin() + pos);
    }

    void removeDir(const String& name)
    {
        size_t pos = hasDir_(name);

        if (pos == NOF_)
            return;

        subDirs_->erase(subDirs_->begin() + pos);
    }

    void releaseAllFilesData()
    {
        if (subFiles_)
            for (auto& var : *subFiles_)
                var.releaseData();

        if (subDirs_)
            for (auto& var : *subDirs_)
                var.releaseAllFilesData();
    }

    void clearFiles()
    {
        if (subFiles_) {
            delete subFiles_;
            subFiles_ = nullptr;
        }
    }

    void clearDirs()
    {
        if (subDirs_) {
            delete subDirs_;
            subDirs_ = nullptr;
        }
    }

    void clear()
    {
        clearFiles();
        clearDirs();
    }

    void add(File& file, bool isOverwrite = false)
    {
        if (subFiles_ == nullptr)
            subFiles_ = new Vec<File>();

        size_t pos = hasFile_(file.name());

        if (pos != NOF_) {
            if (isOverwrite)
                (*subFiles_)[pos] = std::move(file);

            return;
        }

        subFiles_->emplace_back(std::move(file));
    }

    void add(Dir& dir, bool isOverwrite = false)
    {
        if (subDirs_ == nullptr)
            subDirs_ = new Vec<Dir>();

        size_t pos = hasDir_(dir.name());

        if (pos != NOF_) {
            if (isOverwrite)
                (*subDirs_)[pos] = std::move(dir);

            return;
        }

        subDirs_->emplace_back(std::move(dir));
    }

    void add(File&& file, bool isOverwrite = false) { add(file, isOverwrite); }

    void add(Dir&& dir, bool isOverwrite = false) { add(dir, isOverwrite); }

    void write(const String& path, bool isOverwrite = false,
               std::ios_base::openmode openmode = std::ios_base::binary) const
    {
        String root = String(path) + PREFERRED_PATH_SEPARATOR + name_;

        createDirectory(root);

        if (subFiles_)
            for (const auto& var : *subFiles_)
                var.write(root, isOverwrite, openmode);

        if (subDirs_)
            for (const auto& var : *subDirs_)
                var.write(root, isOverwrite);
    }

    Dir copy() const { return Dir(*this); }

    Dir& operator=(const Dir& other)
    {
        name_ = other.name_;

        clear();

        if (other.subFiles_)
            subFiles_ = new Vec<File>(*other.subFiles_);

        if (other.subDirs_)
            subDirs_ = new Vec<Dir>(*other.subDirs_);

        return *this;
    }

    Dir& operator[](const String& name) { return dir(name); }

    File& operator()(const String& name) { return file(name); }

    Dir& operator<<(File& file)
    {
        add(file);
        return *this;
    }

    Dir& operator<<(Dir& dir)
    {
        add(dir);
        return *this;
    }

    Dir& operator<<(File&& file)
    {
        add(file);
        return *this;
    }

    Dir& operator<<(Dir&& dir)
    {
        add(dir);
        return *this;
    }

private:
    static constexpr size_t NOF_ = size_t(-1);

    size_t hasFile_(const String& name) const
    {
        if (subFiles_) {
            for (size_t i = 0; i < subFiles_->size(); ++i) {
                if ((*subFiles_)[i].name() == name)
                    return i;
            }
        }

        return NOF_;
    }

    size_t hasDir_(const String& name) const
    {
        if (subDirs_) {
            for (size_t i = 0; i < subDirs_->size(); ++i) {
                if ((*subDirs_)[i].name() == name)
                    return i;
            }
        }

        return NOF_;
    }

    String name_;
    Vec<File>* subFiles_ = nullptr;
    Vec<Dir>* subDirs_ = nullptr;
};

#endif // !BTF_IMPL

} // namespace btf

#endif // !BETTERFILE_HPP
