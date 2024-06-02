// The "betterfiles" library written in c++.
//
// Web: https://github.com/JaderoChan/betterfiles
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

#ifndef BETTERFILES_HPP
#define BETTERFILES_HPP

#include <cstdint>

#include <string>
#include <fstream>
#include <iostream>
#include <vector>

#define BF_ERROR_UNDEFINED_ "The undefined error be occured."
#define BF_ERROR_FILE_OPEN_FAILED "The file open failed."
#define BF_ERROR_INVALID_PATH "The specify path is invalid."
#define BF_ERROR_INVALID_PARM "The specify parameter is invalid: "
#define BF_ERROR_HINT  "The error be occurred in the function \"" __FUNCTION__ "\". "

#ifdef _MSVC_LANG
#define BETTERFILES_CPPVERS _MSVC_LANG
#else
#define BETTERFILES_CPPVERS __cpluscplus
#endif // _MSVC_LANG

#if BETTERFILES_CPPVERS >= 201703L

#include <string_view>
#include <filesystem>

namespace Bf
{

inline bool isExists(std::string_view path) {
    bool result = false;
    try {
        result = std::filesystem::exists(path);
    } catch (std::filesystem::filesystem_error &) {}
    return result;
}

inline bool isExistsFile(std::string_view path) {
    return isExists(path) && std::filesystem::is_regular_file(path);
}

inline bool isExistsDirectory(std::string_view path) {
    return isExists(path) && std::filesystem::is_directory(path);
}

inline bool isEmpty(std::string_view path) {
    bool result = false;
    try {
        result = !std::filesystem::exists(path) || std::filesystem::is_empty(path);
    } catch (std::filesystem::filesystem_error &) {}
    return result;
}

inline bool isEmptyFile(std::string_view path) {
    return isExistsFile(path) && isEmpty(path);
}

inline bool isEmptyDirectory(std::string_view path) {
    return isExistsDirectory(path) && isEmpty(path);
}

inline std::string getCurrentPath() {
    std::string result;
    try {
        result = std::filesystem::current_path().string();
    } catch (std::filesystem::filesystem_error &e) {
        std::cerr << BF_ERROR_HINT << e.what() << std::endl;
    }
    return result;
}

inline std::string getParentName(std::string_view path) {
    if (!std::filesystem::path(path).has_parent_path()) {
        return std::string();
    }
    return std::filesystem::path(path).parent_path().filename().string();
}

inline std::vector<std::string> getAllDirectorys(std::string_view path, bool isRecursive = true)
{
    if (!isExistsDirectory(path))
        return std::vector<std::string>();
    std::vector<std::string> dirs;
    if (isRecursive) {
        for (auto &var : std::filesystem::recursive_directory_iterator(path)) {
            if (!var.is_directory())
                continue;
            dirs.push_back(var.path().string());
        }
    } else {
        for (auto &var : std::filesystem::directory_iterator(path)) {
            if (!var.is_directory())
                continue;
            dirs.push_back(var.path().string());
        }
    }
    return dirs;
}

inline std::vector<std::string> getAllFiles(std::string_view path, bool isRecursive = true)
{
    if (!isExistsDirectory(path))
        return std::vector<std::string>();
    std::vector<std::string> files;
    if (isRecursive) {
        for (auto &var : std::filesystem::recursive_directory_iterator(path)) {
            if (!var.is_regular_file())
                continue;
            files.push_back(var.path().string());
        }
    } else {
        for (auto &var : std::filesystem::directory_iterator(path)) {
            if (!var.is_regular_file())
                continue;
            files.push_back(var.path().string());
        }
    }
    return files;
}

inline uintmax_t getSize(std::string_view path) {
    uintmax_t result = 0;
    if (isExistsFile(path)) {
        try {
            result = std::filesystem::file_size(path);
        } catch (std::filesystem::filesystem_error &e) {
            std::cerr << BF_ERROR_HINT << e.what() << std::endl;
        }
    } else if (isExistsDirectory(path)) {
        std::vector<std::string> files = getAllFiles(path);
        for (const auto &var : files) {
            try {
                result += std::filesystem::file_size(var);
            } catch (std::filesystem::filesystem_error &e) {
                std::cerr << BF_ERROR_HINT << e.what() << std::endl;
                continue;
            }
        }
    } else {
        std::cerr << BF_ERROR_HINT << BF_ERROR_INVALID_PARM << "\"" << path << "\"" << std::endl;
    }
    return result;
}

inline std::string getPathPrefix(std::string_view path) {
    return std::filesystem::path(path).parent_path().string();
}

inline std::string getPathSuffix(std::string_view path) {
    return std::filesystem::path(path).filename().string();
}

inline std::string getFileName(std::string_view path) {
    return std::filesystem::path(path).filename().replace_extension().string();
}

inline std::string getFileExtension(std::string_view path) {
    return std::filesystem::path(path).filename().extension().string();
}

inline std::string pathNormalizationC(std::string_view path) {
    return std::filesystem::path(path).lexically_normal().string();
}

inline std::string &pathNormalization(std::string &path) {
    path = std::filesystem::path(path).lexically_normal().string();
    return path;
}

inline bool rename(std::string_view oldPath, std::string_view newPath) {
    try {
        std::filesystem::rename(oldPath, newPath);
        return true;
    } catch (std::filesystem::filesystem_error &e) {
        std::cerr << BF_ERROR_HINT << e.what() << std::endl;
    }
    return false;
}

inline bool createDirectory(std::string_view path) {
    bool result = false;
    try {
        result = std::filesystem::create_directories(path);
    } catch (std::filesystem::filesystem_error &e) {
        std::cerr << BF_ERROR_HINT << e.what() << std::endl;
    }
    return result;
}

inline bool deleteFile(std::string_view path) {
    bool result = false;
    if (!isExistsFile(path))
        return result;
    try {
        result = std::filesystem::remove(path);
    } catch (std::filesystem::filesystem_error &e) {
        std::cerr << BF_ERROR_HINT << e.what() << std::endl;
    }
    return result;
}

inline uintmax_t deleteDirectory(std::string_view path) {
    uintmax_t result = 0;
    if (!isExistsDirectory(path))
        return result;
    try {
        result = std::filesystem::remove_all(path);
    } catch (std::filesystem::filesystem_error &e) {
        std::cerr << BF_ERROR_HINT << e.what() << std::endl;
    }
    return result;
}

inline uintmax_t deletes(std::string_view path) {
    uintmax_t result = 0;
    if (isExistsFile(path))
        if (deleteFile(path))
            ++result;
    if (isExistsDirectory(path))
        result += deleteDirectory(path);
    return result;
}

inline bool copyFile(std::string_view src, std::string_view dest, bool destIsFile = true) {
    bool result = false;
    if (!isExistsFile(src))
        return result;
    try {
        if (destIsFile) {
            result = std::filesystem::copy_file(src, dest);
        } else {
            createDirectory(dest);
            result = std::filesystem::copy_file(src, std::string(dest) + "/" + getPathSuffix(src));
        }
    } catch (std::filesystem::filesystem_error &e) {
        std::cerr << BF_ERROR_HINT << e.what() << std::endl;
    }
    return result;
}

inline uintmax_t copyDirectory(std::string_view src, std::string_view dest) {
    uintmax_t result = 0;
    if (!isExistsDirectory(src))
        return result;
    std::vector<std::string> files = getAllFiles(src);
    for (const auto &var : files) {
        std::string newPath = var;
        newPath.replace(0, src.size(), dest);
        if (copyFile(var, newPath))
            ++result;
    }
    return result;
}

}

#else

#include <sys/stat.h>

#ifdef _WIN32

#include <io.h>
#include <windows.h>

#define access _access
#define F_OK 0

std::wstring stringToWString(const std::string &str) {
    // Calculate the size of the buffer needed.
    int size = MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int) str.size(), NULL, 0);
    // Allocate the buffer.
    std::wstring wstr(size, 0);
    // Perform the conversion.
    MultiByteToWideChar(CP_UTF8, 0, str.c_str(), (int) str.size(), &wstr[0], size);
    return wstr;
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
    HANDLE hFind = FindFirstFileW(stringToWString(path + "\\*").c_str(), &findData);

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

inline std::vector<std::string> getAllDirectorys(const std::string &path, bool isRecursive = true) {

}

inline std::vector<std::string> getAllFiles(const std::string &path, bool isRecursive = true) {

}

inline uintmax_t getSize(const std::string &path) {

}

inline std::string getPathPrefix(const std::string &path) {
}

inline std::string getPathSuffix(const std::string &path) {
}

inline std::string getFileName(const std::string &path) {
}

inline std::string getFileExtension(const std::string &path) {
}

inline bool rename(const std::string &oldPath, const std::string &newPath) {

}

inline bool createDirectory(const std::string &path) {
}

inline bool deleteFile(const std::string &path) {
}

inline uintmax_t deleteDirectory(const std::string &path) {
}

inline uintmax_t deletes(const std::string &path) {
}

inline bool copyFile(const std::string &src, const std::string &dest, bool destIsFile = true) {
}

inline uintmax_t copyDirectory(const std::string &src, const std::string &dest) {

}

}

#endif

namespace Bf
{

constexpr const int BufferSize = 1024;

enum WritePolicy
{
    // Skip write operation when the file exist.
    Skip,
    // Override the old file.
    Override
};

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
        char buffer[BufferSize] {};
        while (is.read(buffer, BufferSize))
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

#endif // !BETTERFILES_HPP
