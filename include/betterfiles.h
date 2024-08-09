// The "betterfiles" library written in c++.
//
// Web: https://github.com/JaderoChan/betterfiles
//
// MIT License
//
// Copyright (c) 2024 JaderoChan
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

#ifndef BETTERFILES_H
#define BETTERFILES_H

#include <cstdint>

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <exception>

#define BF_ERROR_UNDEFINED_ "The undefined error be occured."
#define BF_ERROR_FILE_OPEN_FAILED "The file open failed."
#define BF_ERROR_INVALID_PATH "The specify path is invalid."
#define BF_ERROR_INVALID_PARM "The specify parameter is invalid: "
#define BF_ERROR_HINT  "The error be occurred in the function \"" __FUNCTION__ "\". "

#ifdef _WIN32
#define BF_PATH_SEPARATOR '\\'
#else
BF_PATH_SEPARATOR '/'
#endif // _WIN32

#define BF_CHAR_END '\0'

#ifdef _MSVC_LANG
#define BF_CPPVERS _MSVC_LANG
#else
#define Bf_CPPVERS __cpluscplus
#endif // _MSVC_LANG

#if BF_CPPVERS >= 201703L
#define BF_CPP17
#endif // BF_CPPVERS >= 201703L

namespace Bf
{

constexpr int kBufferSize = 4096;

enum WritePolicy
{
    // Skip write operation when the file exist.
    Skip,
    // Override the old file.
    Override
};

}

#ifdef BF_CPP17

#include <string_view>
#include <filesystem>

namespace Bf
{

inline bool isExists(std::string_view path) {
    return std::filesystem::exists(path);
}

inline bool isExistsFile(std::string_view filepath) {
    return isExists(filepath) && std::filesystem::is_regular_file(filepath);
}

inline bool isExistsDirectory(std::string_view dirpath) {
    return isExists(dirpath) && std::filesystem::is_directory(dirpath);
}

inline bool isEmpty(std::string_view path) {
    return !std::filesystem::exists(path) || std::filesystem::is_empty(path);
}

inline bool isEmptyFile(std::string_view filepath) {
    return isExistsFile(filepath) && isEmpty(filepath);
}

inline bool isEmptyDirectory(std::string_view dirpath) {
    return isExistsDirectory(dirpath) && isEmpty(dirpath);
}

inline std::string getCurrentPath() {
    return std::filesystem::current_path().string();
}

inline std::string getParentName(std::string_view path) {
    if (!std::filesystem::path(path).has_parent_path()) {
        return std::string();
    }
    return std::filesystem::path(path).parent_path().filename().string();
}

inline std::pair< std::vector<std::string>, std::vector<std::string>> getAllSubs(
    std::string_view dirpath,
    bool isRecursive = true,
    bool (*filter) (std::string_view) = nullptr)
{
    using Paths = std::vector<std::string>;

    if (!isExistsDirectory(dirpath))
        return std::pair<Paths, Paths>();

    Paths dirs;
    Paths files;
    if (isRecursive) {
        for (auto &var : std::filesystem::recursive_directory_iterator(dirpath)) {
            if (var.is_regular_file() && (filter == nullptr || filter != nullptr && filter(var.path().string())))
                files.push_back(var.path().string());
            else if (var.is_directory() && (filter == nullptr || filter != nullptr && filter(var.path().string())))
                dirs.push_back(var.path().string());
            else
                continue;
        }
    } else {
        for (auto &var : std::filesystem::directory_iterator(dirpath)) {
            if (var.is_regular_file() && (filter == nullptr || filter != nullptr && filter(var.path().string())))
                files.push_back(var.path().string());
            else if (var.is_directory() && (filter == nullptr || filter != nullptr && filter(var.path().string())))
                dirs.push_back(var.path().string());
            else
                continue;
        }
    }

    return std::pair<Paths, Paths>(files, dirs);
}

inline std::vector<std::string> getAllFiles(std::string_view dirpath, bool isRecursive = true,
                                            bool (*filter) (std::string_view) = nullptr)
{
    using Paths = std::vector<std::string>;

    if (!isExistsDirectory(dirpath))
        return Paths();

    Paths files;
    if (isRecursive) {
        for (auto &var : std::filesystem::recursive_directory_iterator(dirpath)) {
            if (var.is_regular_file() && (filter == nullptr || filter != nullptr && filter(var.path().string())))
                files.push_back(var.path().string());
        }
    } else {
        for (auto &var : std::filesystem::directory_iterator(dirpath)) {
            if (var.is_regular_file() && (filter == nullptr || filter != nullptr && filter(var.path().string())))
                files.push_back(var.path().string());
        }
    }

    return files;
}

inline std::vector<std::string> getAllDirectorys(std::string_view dirpath, bool isRecursive = true,
                                                 bool (*filter) (std::string_view) = nullptr)
{
    using Paths = std::vector<std::string>;

    if (!isExistsDirectory(dirpath))
        return Paths();

    Paths dirs;
    if (isRecursive) {
        for (auto &var : std::filesystem::recursive_directory_iterator(dirpath)) {
            if (var.is_directory() && (filter == nullptr || filter != nullptr && filter(var.path().string())))
                dirs.push_back(var.path().string());
        }
    } else {
        for (auto &var : std::filesystem::directory_iterator(dirpath)) {
            if (var.is_directory() && (filter == nullptr || filter != nullptr && filter(var.path().string())))
                dirs.push_back(var.path().string());
        }
    }

    return dirs;
}

inline uintmax_t getSize(std::string_view path) {
    uintmax_t result = 0;
    if (isExistsFile(path)) {
        result = std::filesystem::file_size(path);
    } else if (isExistsDirectory(path)) {
        std::vector<std::string> files = getAllFiles(path);
        for (auto &var : files)
            result += std::filesystem::file_size(var);
    }
    return result;
}

inline std::string pathNormalizationC(std::string_view path) {
    return std::filesystem::path(path).lexically_normal().string();
}

inline std::string &pathNormalization(std::string &path) {
    return path = std::filesystem::path(path).lexically_normal().string();
}

inline std::string getPathPrefix(std::string_view path) {
    return std::filesystem::path(path).parent_path().string();
}

inline std::string getPathSuffix(std::string_view path) {
    return std::filesystem::path(path).filename().string();
}

inline std::string getFileName(std::string_view filepath) {
    return std::filesystem::path(filepath).filename().replace_extension().string();
}

inline std::string getFileExtension(std::string_view filepath) {
    return std::filesystem::path(filepath).filename().extension().string();
}

inline void rename(std::string_view oldPath, std::string_view newPath) {
    std::filesystem::rename(oldPath, newPath);
}

inline bool createDirectory(std::string_view dirpath) {
    return std::filesystem::create_directories(dirpath);
}

inline bool deleteFile(std::string_view filepath) {
    return std::filesystem::remove(filepath);
}

inline uintmax_t deleteDirectory(std::string_view dirpath) {
    if (!isExistsDirectory(dirpath))
        return 0;
    return  std::filesystem::remove_all(dirpath);
}

inline uintmax_t deletes(std::string_view path) {
    if (isExistsFile(path))
        return deleteFile(path);
    if (isExistsDirectory(path))
        return deleteDirectory(path);
    return 0;
}

inline bool copyFile(std::string_view src, std::string_view dst, bool dstIsEnd = true,
                     WritePolicy wp = Skip)
{
    if (!isExistsFile(src))
        return false;

    std::filesystem::copy_options cop = wp == Skip ?
        std::filesystem::copy_options::skip_existing : std::filesystem::copy_options::overwrite_existing;
    if (dstIsEnd) {
        return std::filesystem::copy_file(src, dst, cop);
    } else {
        if (!isExistsDirectory(dst))
            createDirectory(dst);
        return std::filesystem::copy_file(src,
                                          std::string(dst) + BF_PATH_SEPARATOR + getPathSuffix(src),
                                          cop);
    }
}

inline uintmax_t copyDirectory(std::string_view src, std::string_view dst, WritePolicy wp = Skip) {
    uintmax_t result = 0;
    if (!isExistsDirectory(src))
        return result;

    std::vector<std::string> files = getAllFiles(src);
    for (auto &var : files) {
        std::string newPath = var;
        newPath.replace(0, src.size(), dst);
        if (copyFile(var, newPath, true, wp))
            ++result;
    }

    return result;
}

inline void createHardLink(std::string_view src, std::string_view dst, bool dstIsEnd = true) {
    if (dstIsEnd)
        std::filesystem::create_hard_link(src, dst);
    else {
        if (!isExistsDirectory(dst))
            createDirectory(dst);
        std::filesystem::create_hard_link(src,
                                          std::string(dst) + BF_PATH_SEPARATOR + getPathSuffix(src));
    }
}

}

#else

#include <sys/stat.h>

#ifdef _WIN32

#include <io.h>
#include <windows.h>

#define access _access
#define F_OK 0

inline std::wstring string2wstring(const std::string &string)
{
    std::wstring result;
    int len = MultiByteToWideChar(CP_ACP, 0, string.c_str(), (int) string.size(), NULL, 0);
    WCHAR *buffer = new WCHAR[len + 1];
    MultiByteToWideChar(CP_ACP, 0, string.c_str(), (int) string.size(), buffer, len);
    buffer[len] = BF_CHAR_END;
    result = buffer;
    delete[] buffer;
    return result;
}

inline std::string wstring2string(const std::wstring &wstring)
{
    std::string result;
    int len = WideCharToMultiByte(CP_ACP, 0, wstring.c_str(), (int) wstring.size(), NULL, 0, NULL, NULL);
    char *buffer = new char[len + 1];
    WideCharToMultiByte(CP_ACP, 0, wstring.c_str(), (int) wstring.size(), buffer, len, NULL, NULL);
    buffer[len] = BF_CHAR_END;
    result = buffer;
    delete[] buffer;
    return result;
}

#else
#include <unistd.h>
#include <dirent.h>
#include <limits.h> // PATH_MAX
#endif  // _WIN32

namespace Bf
{

inline std::string pathNormalizationC(const std::string &path) {
    if (path.empty())
        return path;
    std::string _path = path;
    while (_path.back() == '/' || _path.back() == '\\')
        _path.pop_back();
    std::size_t pos = 0;
    if (_path.empty())
        return _path;
    while ((pos = _path.find('\\')) != std::string::npos)
        _path.replace(pos, 1, "/");
    return _path;
}

inline std::string &pathNormalization(std::string &path) {
    if (path.empty())
        return path;
    while (path.back() == '/' || path.back() == '\\')
        path.pop_back();
    std::size_t pos = 0;
    if (path.empty())
        return path;
    while ((pos = path.find('\\')) != std::string::npos)
        path.replace(pos, 1, "/");
    return path;
}

inline bool isExists(const std::string &path) {
    return access(path.c_str(), F_OK) != -1;
}

inline bool isExistsFile(const std::string &path) {
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0)
        return false;
#ifdef _WIN32
    return path_stat.st_mode == _S_IFREG;
#else
    return S_ISREG(path_stat.st_mode);
#endif // _WIN32
}

inline bool isExistsDirectory(const std::string &path) {
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0)
        return false;
#ifdef _WIN32
    return path_stat.st_mode == _S_IFDIR;
#else
    return S_ISDIR(path_stat.st_mode);
#endif // _WIN32

}

inline bool isEmptyFile(const std::string &path) {
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0)
        return false;
    return path_stat.st_size == 0;
}

inline bool isEmptyDirectory(const std::string &path) {
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFileW(string2wstring(path + "\\*").c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE)
        return false;

    int n = 0;
    while (FindNextFileW(hFind, &findData) != 0)
        if (++n > 2)
            break;

    FindClose(hFind);
    // '.' and '..' are always present.
    return n <= 2;
#else
    struct dirent *d;
    DIR *dir = opendir(path.c_str());

    if (dir == nullptr)
        return false;

    int n = 0;
    while ((d = readdir(dir)) != nullptr)
        if (++n > 2)
            break;

    closedir(dir);
    // '.' and '..' are always present.
    return n <= 2;
#endif // _WIN32
}

inline bool isEmpty(const std::string &path) {
    return isEmptyFile(path) || isEmptyDirectory(path);
}

inline std::string getCurrentPath() {
    std::string result;
#ifdef _WIN32
    char path[MAX_PATH] {};
    if (GetCurrentDirectoryA(MAX_PATH, path)) {
        result = path;
    } else {
        // TODO
    }
#else
    char path[PATH_MAX];
    if (getcwd(path, PATH_MAX) != nullptr) {
        result = path;
    } else {
        // TODO
    }
#endif // _WIN32
    return result;
}

inline std::string getParentName(const std::string &path) {
    std::string _path = pathNormalizationC(path);
    if (_path.empty()) {
        // TODO
        return std::string();
    }
    std::size_t pos = _path.rfind('/');
    if (pos == std::string::npos) {
        // TODO
        return std::string();
    }
    _path = _path.substr(0, pos);
    pos = _path.rfind('/');
    if (pos == std::string::npos)
        return std::string();
    return _path.substr(pos + 1);
}

// TODO
inline std::vector<std::string> getAllDirectorys(const std::string &path, bool isRecursive = true) {

}

// TODO
inline std::vector<std::string> getAllFiles(const std::string &path, bool isRecursive = true) {

}

// TODO
inline uintmax_t getSize(const std::string &path) {

}

// TODO
inline std::string getPathPrefix(const std::string &path) {
}

// TODO
inline std::string getPathSuffix(const std::string &path) {
}

// TODO
inline std::string getFileName(const std::string &path) {
}

// TODO
inline std::string getFileExtension(const std::string &path) {
}

// TODO
inline bool rename(const std::string &oldPath, const std::string &newPath) {

}

// TODO
inline bool createDirectory(const std::string &path) {
}

// TODO
inline bool deleteFile(const std::string &path) {
}

// TODO
inline uintmax_t deleteDirectory(const std::string &path) {
}

// TODO
inline uintmax_t deletes(const std::string &path) {
}

// TODO
inline bool copyFile(const std::string &src, const std::string &dest, bool destIsFile = true) {
}

// TODO
inline uintmax_t copyDirectory(const std::string &src, const std::string &dest) {

}

}

#endif // BF_CPP17

namespace Bf
{

class File
{
public:
    explicit File(const std::string &name) : mName(name), mData(nullptr) {}

    explicit File(const File &other) : mData(nullptr) {
        mName = other.mName;
        if (other.mData == nullptr)
            return;
        mData = new std::string(*other.mData);
    }

    File(File &&other) noexcept {
        mName = other.mName;
        mData = other.mData;
        other.mData = nullptr;
    }

    ~File() {
        clear();
    }

    File copy() const {
        return File(*this);
    }

    static File fromPath(const std::string &filePath) {
        if (!isExistsFile(filePath))
            throw BF_ERROR_INVALID_PATH;
        std::ifstream ifs(filePath.data(), std::ios_base::binary);
        if (!ifs.is_open())
            throw BF_ERROR_FILE_OPEN_FAILED;
        File file(getPathSuffix(filePath));
        file << ifs;
        ifs.close();
        return file;
    }

    std::string name() const {
        return mName;
    }

    std::string data() const {
        if (mData == nullptr)
            return std::string();
        return *mData;
    }

    std::size_t size() const {
        if (mData == nullptr)
            return 0;
        return mData->size();
    }

    bool empty() const {
        if (mData == nullptr)
            return true;
        return mData->empty();
    }

    void setName(const std::string &name) {
        mName = name;
    }

    void clear() {
        if (mData == nullptr)
            return;
        delete mData;
        mData = nullptr;
    }

    void write(std::ostream &os) const {
        if (mData == nullptr)
            return;
        os << *mData;
    }

    void write(const std::string &path, WritePolicy policy = Skip,
               std::ios_base::openmode openmode = std::ios_base::binary) const
    {
        std::string _path = path.data();
        _path += "/" + mName;
        if (isExistsFile(_path) && policy == Skip)
            return;
        std::ofstream os(_path.data(), openmode);
        if (!os.is_open())
            throw BF_ERROR_FILE_OPEN_FAILED;
        write(os);
        os.close();
    }

    File &operator=(const File &other) {
        mName = other.mName;
        clear();
        if (other.mData != nullptr)
            mData = new std::string(*other.mData);
        return *this;
    }

    File &operator=(const std::string &data) {
        clear();
        mData = new std::string(data);
        return *this;
    }

    template<typename T>
    File &operator=(const std::vector<T> &data) {
        clear();
        mData = new std::string;
        std::size_t size = data.size();
        mData->reserve(mData->size() + size);
        for (auto &var : data)
            mData->push_back(var);
        return *this;
    }

    File &operator<<(const File &other) {
        if (mData == nullptr)
            mData = new std::string;
        mData->append(other.data());
        return *this;
    }

    File &operator<<(std::istream &is) {
        if (mData == nullptr)
            mData = new std::string();
        is.seekg(0, std::ios_base::end);
        std::size_t size = is.tellg();
        is.seekg(0, std::ios_base::beg);
        mData->reserve(mData->size() + size);
        char buffer[kBufferSize] {};
        while (is.read(buffer, kBufferSize))
            mData->append(buffer, is.gcount());
        mData->append(buffer, is.gcount());
        return *this;
    }

    File &operator<<(const std::string &data) {
        if (mData == nullptr)
            mData = new std::string();
        mData->append(data);
        return *this;
    }

    template<typename T>
    File &operator<<(const std::vector<T> &data) {
        if (mData == nullptr)
            mData = new std::string;
        std::size_t size = data.size();
        mData->reserve(mData->size() + size);
        for (auto &var : data)
            mData->push_back(var);
        return *this;
    }

    const File &operator>>(std::ostream &os) const {
        write(os);
        return *this;
    }

private:
    std::string mName;
    std::string *mData;
};

class Dir
{
public:
    explicit Dir(const std::string &name) : mName(name), mSubFiles(nullptr), mSubDirs(nullptr) {}

    explicit Dir(const Dir &other) : mSubFiles(nullptr), mSubDirs(nullptr) {
        mName = other.mName;
        if (other.mSubFiles != nullptr)
            mSubFiles = new std::vector<File>(*other.mSubFiles);
        if (other.mSubDirs != nullptr)
            mSubDirs = new std::vector<Dir>(*other.mSubDirs);
    }

    Dir(Dir &&other) noexcept {
        mName = other.mName;
        mSubFiles = other.mSubFiles;
        other.mSubFiles = nullptr;
        mSubDirs = other.mSubDirs;
        other.mSubDirs = nullptr;
    }

    ~Dir() {
        clearFiles();
        clearDirs();
    }

    Dir copy() const {
        return Dir(*this);
    }

    static Dir fromPath(const std::string &dirPath) {
        if (!isExistsDirectory(dirPath))
            throw BF_ERROR_INVALID_PATH;
        Dir root(getPathSuffix(dirPath));

        auto dirs = getAllDirectorys(dirPath, false);
        for (auto &var : dirs)
            root << Dir::fromPath(var);

        auto files = getAllFiles(dirPath, false);
        for (auto &var : files)
            root << File::fromPath(var);

        return root;
    }

    std::string name() const {
        return mName;
    }

    std::size_t size() const {
        std::size_t size = 0;
        if (mSubFiles != nullptr) {
            for (auto &var : *mSubFiles)
                size += var.size();
        }
        if (mSubDirs != nullptr) {
            for (auto &var : *mSubDirs)
                size += var.size();
        }
        return size;
    }

    std::size_t fileCount() const {
        if (mSubFiles == nullptr)
            return 0;
        return mSubFiles->size();
    }

    std::size_t dirCount() const {
        if (mSubDirs == nullptr)
            return 0;
        return mSubDirs->size();
    }

    std::size_t count() const {
        return fileCount() + dirCount();
    }

    std::vector<File> files() const {
        if (mSubFiles == nullptr)
            return std::vector<File>();
        return *mSubFiles;
    }

    std::vector<Dir> dirs() const {
        if (mSubFiles == nullptr)
            return std::vector<Dir>();
        return *mSubDirs;
    }

    std::vector<File> &files() {
        if (mSubFiles == nullptr)
            mSubFiles = new std::vector<File>();
        return *mSubFiles;
    }

    std::vector<Dir> &dirs() {
        if (mSubDirs == nullptr)
            mSubDirs = new std::vector<Dir>();
        return *mSubDirs;
    }

    bool empty() const {
        return (mSubFiles == nullptr || mSubFiles->empty()) &&
            (mSubDirs == nullptr || mSubDirs->empty());
    }

    bool hasFile(const std::string &name) const {
        if (mSubFiles == nullptr)
            return false;
        for (auto &var : *mSubFiles) {
            if (var.name() == name)
                return true;
        }
        return false;
    }

    bool hasDir(const std::string &name) const {
        if (mSubDirs == nullptr)
            return false;
        for (auto &var : *mSubDirs) {
            if (var.name() == name)
                return true;
        }
        return false;
    }

    void setName(const std::string &name) {
        mName = name;
    }

    File &file(const std::string &name) {
        if (mSubFiles == nullptr)
            mSubFiles = new std::vector<File>();
        for (auto &var : *mSubFiles) {
            if (var.name() == name)
                return var;
        }
        mSubFiles->push_back(File(name));
        return mSubFiles->back();
    }

    Dir &dir(const std::string &name) {
        if (mSubDirs == nullptr)
            mSubDirs = new std::vector<Dir>();
        for (auto &var : *mSubDirs) {
            if (var.name() == name)
                return var;
        }
        mSubDirs->push_back(Dir(name));
        return mSubDirs->back();
    }

    void clearFiles() {
        if (mSubFiles == nullptr)
            return;
        delete mSubFiles;
        mSubFiles = nullptr;
    }

    void clearDirs() {
        if (mSubDirs == nullptr)
            return;
        delete mSubDirs;
        mSubDirs = nullptr;
    }

    void add(File &file) {
        if (mSubFiles == nullptr)
            mSubFiles = new std::vector<File>();
        mSubFiles->emplace_back(std::move(file));
    }

    void add(Dir &dir) {
        if (mSubDirs == nullptr)
            mSubDirs = new std::vector<Dir>();
        mSubDirs->emplace_back(std::move(dir));
    }

    void add(File &&file) {
        add(file);
    }

    void add(Dir &&dir) {
        add(dir);
    }

    void addFile(const std::string &name) {
        add(File(name));
    }

    void addDir(const std::string &name) {
        add(Dir(name));
    }

    void write(const std::string &path, WritePolicy policy = Skip,
               std::ios_base::openmode openmode = std::ios_base::binary) const
    {
        std::string root = std::string(path) + '/' + mName;
        createDirectory(root);

        if (mSubFiles != nullptr) {
            for (auto &var : *mSubFiles) {
                var.write(root, policy, openmode);
            }
        }

        if (mSubDirs != nullptr) {
            for (auto &var : *mSubDirs)
                var.write(root, policy);
        }
    }

    Dir &operator=(const Dir &other) {
        mName = other.mName;
        clearFiles();
        clearDirs();
        if (other.mSubFiles != nullptr)
            mSubFiles = new std::vector<File>(*other.mSubFiles);
        if (other.mSubDirs != nullptr)
            mSubDirs = new std::vector<Dir>(*other.mSubDirs);
        return *this;
    }

    Dir &operator[](const std::string &name) {
        return dir(name);
    }

    File &operator()(const std::string &name) {
        return file(name);
    }

    Dir &operator<<(File &file) {
        add(file);
        return *this;
    }

    Dir &operator<<(Dir &dir) {
        add(dir);
        return *this;
    }

    Dir &operator<<(File &&file) {
        add(file);
        return *this;
    }

    Dir &operator<<(Dir &&dir) {
        add(dir);
        return *this;
    }

private:
    std::string mName;
    std::vector<File> *mSubFiles;
    std::vector<Dir> *mSubDirs;
};

}

#endif // !BETTERFILES_H
