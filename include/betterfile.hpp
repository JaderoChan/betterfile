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

// Usage:
// 1. The betterfile is a header-only library, so you don't need to build it.
// 2. You can use this library by only including the betterfile.hpp file in your project.
// 3. You can also make a new .cpp file to your project and add the code below.
// (This method's aim is detach the implementation from the header file, can be avoid namespace pollution)
//
// #define BTF_IMPL
// #include <betterfile.hpp>
// 
// Last you need add the code below to your other location which need use this library.
//
// #define BTF_FWD
// #include <betterfile.hpp>
//

#ifndef BETTERFILE_HPP
#define BETTERFILE_HPP

#include <cstddef>  // size_t
#include <string>
#include <vector>
#include <iostream>
#include <fstream>
#include <stdexcept>

// Compiler version.
#ifdef _MSVC_LANG
#define _BETTERFILE_CPPVERS     _MSVC_LANG
#else
#define _BETTERFILE_CPPVERS     __cplusplus
#endif // _MSVC_LANG

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
#endif // BTF_IMPL

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

template<typename T>
using Vec = std::vector<T>;
using String = std::string;
using Strings = Vec<String>;

enum WritePolicy : uchar
{
    // Skip write process when the destination file exists.
    SKIP,
    // Override the destination file.
    OVERRIDE
};

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

}

// Utility functions with not filesystem.
namespace btf
{

// @brief Concatenate two paths.
inline String pathcat(const String& path1, const String& path2)
{
    return path1 + PREFERRED_PATH_SEPARATOR + path2;
}

// @brief Concatenate multiple paths.
template<typename... Args>
String pathcat(const String& path1, const String& path2, const Args&&... paths)
{
    if (sizeof...(paths) == 0)
        return pathcat(path1, path2);
    else
        return pathcat(pathcat(path1, path2), paths...);
}

// @brief Check if the filename is valid.
bool isValidFilename(const String& filename)
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

}

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
#endif // _BETTERFILE_CPP17

// Declaration of utility functions with filesystem.
namespace btf
{

#ifndef BTF_IMPL

BTF_API String normalize(const String& path);

BTF_API String currentPath();

BTF_API String parentPath(const String& path);

BTF_API String parentName(const String& path);

BTF_API String filenameEx(const String& path);

BTF_API String filename(const String& path);

BTF_API String extension(const String& path);

BTF_API bool isExists(const String& path);

BTF_API bool isFile(const String& path);

BTF_API bool isDirectory(const String& path);

BTF_API bool isSymlink(const String& path);

BTF_API bool isEmpty(const String& path);

BTF_API bool isRelative(const String& path);

BTF_API bool isAbsolute(const String& path);

BTF_API String relative(const String& path, const String& base = currentPath());

BTF_API String absolute(const String& path);

BTF_API bool equivalent(const String& path1, const String& path2);

BTF_API size_t sizes(const String& path);

BTF_API bool createDirectory(const String& path);

BTF_API bool createDirectorys(const String& path);

BTF_API size_t deletes(const String& path);

BTF_API void copy(const String& src, const String& dst, WritePolicy wp = SKIP);

BTF_API void move(const String& src, const String& dst, WritePolicy wp = SKIP);

BTF_API void reFilename(const String& path, const String& newFilename, WritePolicy wp = SKIP);

BTF_API void reFilenameEx(const String& path, const String& newFilenameEx, WritePolicy wp = SKIP);

BTF_API void reExtension(const String& path, const String& newExtension, WritePolicy wp = SKIP);

BTF_API void createSymlink(const String& src, const String& dst, WritePolicy wp = SKIP);

BTF_API String symlinkTarget(const String& path);

BTF_API void createHardlink(const String& src, const String& dst, WritePolicy wp = SKIP);

BTF_API void createHardlinkDirectory(const String& src, const String& dst,
                                     bool isRecursive = true, WritePolicy wp = SKIP);

BTF_API size_t hardlinkCount(const String& path);

BTF_API String tempDirectory();

BTF_API std::pair<Strings, Strings> getAlls(const String& path, bool isRecursive = true,
                                            bool (*filter) (const String&) = nullptr);

BTF_API Strings getAllFiles(const String& path, bool isRecursive = true,
                            bool (*filter) (const String&) = nullptr);

BTF_API Strings getAllDirectorys(const String& path, bool isRecursive = true,
                                 bool (*filter) (const String&) = nullptr);

BTF_API void permission(const String& path)
{
    fs::permissions(path, fs::perms::all);
}

#endif // !BTF_IMPL

}

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

BTF_API bool isSymlink(const String &path)
{
    return fs::is_symlink(path);
}

BTF_API bool isEmpty(const String& path)
{
    return fs::is_empty(path);
}

BTF_API bool isRelative(const String &path)
{
    return _pth(path).is_relative();
}

BTF_API bool isAbsolute(const String &path)
{
    return _pth(path).is_absolute();
}

BTF_API String relative(const String& path, const String& base)
{
    return fs::relative(path, base).string();
}

BTF_API String absolute(const String &path)
{
    return fs::absolute(path).string();
}

BTF_API bool equivalent(const String& path1, const String& path2)
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
        throw std::runtime_error(String("#") + __FUNCTION__ + "() The specify path not exists.");
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

BTF_API size_t deletes(const String& path)
{
    return fs::remove_all(path);
}

BTF_API void copy(const String& src, const String& dst, WritePolicy wp)
{
    bool dstIsFile = isFile(dst);
    fs::copy_options op;

    switch (wp) {
        case SKIP:
            // Fallthrough.
        default:
            op = fs::copy_options::skip_existing;
            if (dstIsFile)
                op |= fs::copy_options::skip_symlinks;
            break;
        case OVERRIDE:
            op = fs::copy_options::overwrite_existing;
            if (dstIsFile)
                op |= fs::copy_options::copy_symlinks;
            break;
    }

    fs::copy(src, dst, op);
}

BTF_API void move(const String& src, const String& dst, WritePolicy wp)
{
    if (wp == SKIP && isExists(dst))
        return;
    else
        fs::rename(src, dst);
}

BTF_API void reFilename(const String& path, const String& newFilename, WritePolicy wp)
{
    auto dst = pathcat(parentPath(path), newFilename + extension(path));
    move(path, dst, wp);
}

BTF_API void reFilenameEx(const String& path, const String& newFilenameEx, WritePolicy wp)
{
    auto dst = pathcat(parentPath(path), newFilenameEx);
    move(path, dst, wp);
}

BTF_API void reExtension(const String& path, const String& newExtension, WritePolicy wp)
{
    auto dst = pathcat(parentPath(path), filename(path) + newExtension);
    move(path, dst, wp);
}

BTF_API void createSymlink(const String& src, const String& dst, WritePolicy wp)
{
    if (isExists(dst) && wp == SKIP)
        return;

    if (isFile(src))
        fs::create_symlink(src, dst);
    else if (isDirectory(src))
        fs::create_directory_symlink(src, dst);
    else
        throw std::runtime_error(String("#") + __FUNCTION__ + "() The specify path not exists.");
}

BTF_API String symlinkTarget(const String &path)
{
    return fs::read_symlink(path).string();
}

BTF_API void createHardlink(const String& src, const String& dst, WritePolicy wp)
{
    if (isExists(dst) && wp == SKIP)
        return;

    fs::create_hard_link(src, dst);
}

BTF_API void
createHardlinkDirectory(const String& src, const String& dst, bool isRecursive, WritePolicy wp)
{
    if (!isDirectory(src))
        throw std::runtime_error(String("#") + __FUNCTION__ + "() The specify path is not directory or not exists");

    createDirectorys(dst);

    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(src)) {
            if (var.is_regular_file())
                createHardlink(var.path().string(),
                               pathcat(dst, String(var.path().string()).substr(src.size())),
                               wp);

            if (var.is_directory())
                createDirectory(pathcat(dst, String(var.path().string()).substr(src.size())));
        }
    } else {
        for (const auto& var : fs::directory_iterator(src)) {
            if (var.is_regular_file())
                createHardlink(var.path().string(),
                               pathcat(dst, String(var.path().string()).substr(src.size())),
                               wp);

            if (var.is_directory())
                createDirectory(pathcat(dst, String(var.path().string()).substr(src.size())));
        }
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

BTF_API std::pair<Strings, Strings>
getAlls(const String& path, bool isRecursive, bool (*filter) (const String&))
{
    if (!isDirectory(path))
        throw std::runtime_error(String("#") + __FUNCTION__ + "() The specify path is not directory or not exists.");

    Strings files;
    Strings dirs;

    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String path = var.path().string();

            if (var.is_regular_file() && (filter == nullptr || filter(path)))
                files.push_back(path);

            if (var.is_directory() && (filter == nullptr || filter(path)))
                dirs.push_back(path);
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String path = var.path().string();

            if (var.is_regular_file() && (filter == nullptr || filter(path)))
                files.push_back(path);

            if (var.is_directory() && (filter == nullptr || filter(path)))
                dirs.push_back(path);
        }
    }

    return {files, dirs};
}

BTF_API Strings
getAllFiles(const String& path, bool isRecursive, bool (*filter) (const String&))
{
    if (!isDirectory(path))
        throw std::runtime_error(String("#") + __FUNCTION__ + "() The specify path is not directory or not exists.");

    Strings files;

    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String path = var.path().string();

            if (var.is_regular_file() && (filter == nullptr || filter(path)))
                files.push_back(path);
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String path = var.path().string();

            if (var.is_regular_file() && (filter == nullptr || filter(path)))
                files.push_back(path);
        }
    }

    return files;
}

BTF_API Strings
getAllDirectorys(const String& path, bool isRecursive, bool (*filter) (const String&))
{
    if (!isDirectory(path))
        throw std::runtime_error(String("#") + __FUNCTION__ + "() The specify path is not directory or not exists.");

    Strings dirs;

    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String path = var.path().string();

            if (var.is_directory() && (filter == nullptr || filter(path)))
                dirs.push_back(path);
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String path = var.path().string();

            if (var.is_directory() && (filter == nullptr || filter(path)))
                dirs.push_back(path);
        }
    }

    return dirs;
}

#endif // !BTF_FWD

}

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
            throw std::runtime_error("Failed to open the file: " + filename);

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

    bool empty() const { return size() == 0;}

    void setName(const String& name)
    {
        if (!isValidFilename(name))
            throw std::runtime_error("Invalid file name: " + name);

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

    void write(const String& path, WritePolicy wp = SKIP,
               std::ios_base::openmode openmode = std::ios_base::binary) const
    {
        String _path = path + PREFERRED_PATH_SEPARATOR + name_;

        if (isFile(_path) && wp == SKIP)
            return;

        std::ofstream ofs(_path.data(), openmode);

        if (!ofs.is_open())
            throw std::runtime_error("Failed to open the file: " + path);

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

    template<typename T>
    File& operator=(const std::vector<T>& data)
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

    template<typename T>
    File& operator<<(const std::vector<T>& data)
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

    bool empty() const {return size() == 0;}

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
            throw std::runtime_error("Invalid file name: " + name);

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

    void add(File& file, WritePolicy wp = SKIP)
    {
        if (subFiles_ == nullptr)
            subFiles_ = new Vec<File>();

        size_t pos = hasFile_(file.name());

        if (pos != NOF_) {
            if (wp == OVERRIDE)
                (*subFiles_)[pos] = std::move(file);

            return;
        }

        subFiles_->emplace_back(std::move(file));
    }

    void add(Dir& dir, WritePolicy wp = SKIP)
    {
        if (subDirs_ == nullptr)
            subDirs_ = new Vec<Dir>();

        size_t pos = hasDir_(dir.name());

        if (pos != NOF_) {
            if (wp == OVERRIDE)
                (*subDirs_)[pos] = std::move(dir);

            return;
        }

        subDirs_->emplace_back(std::move(dir));
    }

    void add(File&& file, WritePolicy wp = SKIP) { add(file, wp); }

    void add(Dir&& dir, WritePolicy wp = SKIP) { add(dir, wp); }

    void write(const String& path, WritePolicy wp = SKIP,
               std::ios_base::openmode openmode = std::ios_base::binary) const
    {
        String root = String(path) + PREFERRED_PATH_SEPARATOR + name_;

        createDirectory(root);

        if (subFiles_)
            for (const auto& var : *subFiles_)
                var.write(root, wp, openmode);

        if (subDirs_)
            for (const auto& var : *subDirs_)
                var.write(root, wp);
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

}

#endif // !BETTERFILE_HPP
