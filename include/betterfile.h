// The "betterfile" library written in c++.
//
// Web: https://github.com/JaderoChan/betterfile
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

#ifndef BETTERFILE_H
#define BETTERFILE_H

#include <cstdint>  // uintmax_t
#include <cstddef>  // size_t
#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <stdexcept>

#ifdef _MSVC_LANG
#define BTF_CPPVERS     _MSVC_LANG
#else
#define BTF_CPPVERS      __cplusplus
#endif // _MSVC_LANG

#if BTF_CPPVERS >= 201703L
#define BTF_CPP17
#endif // BTF_CPPVERS >= 201703L

#define BTF_PATH_SEPARATOR_WIN          '\\'
#define BTF_PATH_SEPARATOR_LINUX        '/'

#ifdef _WIN32
#define BTF_PATH_SEPARATOR      BTF_PATH_SEPARATOR_WIN
#else
#define BTF_PATH_SEPARATOR      BTF_PATH_SEPARATOR_LINUX
#endif // _WIN32

// Exception infos.
#ifndef BTF_ERROR_TYPE // Just for the code block to be foldable.
#define BTF_ERROR_INFO
#define BTF_ERR_UNDEFINED             "The undefined error."
#define BTF_ERR_FAILED_OSAPI          "Failed to process in OS API."
#define BTF_ERR_FILE_OPEN_FAILED      "Failed to open file."
#define BTF_ERR_INVALID_PATH          "The invalid path."
#define BTF_ERR_INVALID_PARM          "The invalid parameter."
#define BTF_ERR_UNEXISTS_PATH         "The path is not exists."
#define BTF_ERR_CP                    "The unsupported character set."
#endif // !BTF_ERROR_INFO

namespace btf
{

// type alias.

typedef unsigned char uchar;
typedef unsigned int uint;

template<typename T>
using Vec = std::vector<T>;
using String = std::string;
using WString = std::wstring;
using Strings = Vec<String>;

}

#define BTF_ERROR_HINT          "[BetterFile Error] "
#define BTF_MKERR(et,added)     std::runtime_error(std::string(BTF_ERROR_HINT) + et + " " + added)

namespace btf
{

// constexprs and enums.

constexpr uint BUFFER_SIZE = 4096;

enum WritePolicy : uchar
{
    // Skip write process when the file exists.
    SKIP = 1,
    // Override the old file.
    OVERRIDE
};

}

#ifdef BTF_CPP17
#include <filesystem>
namespace btf
{

namespace fs = std::filesystem;

}
#else
#include <sys/stat.h>
#include <algorithm>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <ShlObj.h>

#undef min
#undef max

#define BTF_ACCESS ::_access
#define BTF_F_OK 0

#else
#include <unistd.h>
#include <dirent.h>
#endif  // _WIN32

namespace btf
{

// aux functions.

inline WString string2wstring(const String& str)
{
    WString result;
#ifdef _WIN32
    int len = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int) str.size(), NULL, 0);

    if (len == 0)
        return result;

    wchar_t* buffer = new wchar_t[len + 1];
    int rtn = ::MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int) str.size(), buffer, len);

    if (rtn == 0)
        return result;

    result = buffer;

    delete[] buffer;
#else
    // TODO for linux.
#endif // _WIN32
    return result;
}

inline String wstring2string(const WString& wstr)
{
    String result;
#ifdef _WIN32
    int len = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int) wstr.size(),
                                  NULL, 0, NULL, NULL);

    if (len == 0)
        return result;

    char* buffer = new char[len + 1];
    int rtn = ::WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int) wstr.size(),
                                  buffer, len, NULL, NULL);

    if (rtn == 0)
        return result;

    result = buffer;

    delete[] buffer;
#else
    // TODO for linux.
#endif // _WIN32
    return result;
}

#ifdef _WIN32
HANDLE getFileHandle(const String& path)
{
    WIN32_FIND_DATAA fd;
    return ::FindFirstFileA(path.c_str(), &fd);
}
#endif // _WIN32

}

#endif // BTF_CPP17

namespace btf
{

// utility functions.

// @brief Normalize path following:
// 1. discard the path separator char at end.
// 2. merge the sequential path separator.
// 3. convert the path separator to suit OS.
// @param removeDQuot If it is true, discard the double quotation mark '"'.
// @note Do not change the paramter.
// @example "C://path_to/\/file\" => "C:\path_to\file"
// @example ""C:/path_to/file name . ext"" => "C:\path_to\file name . ext" (removeDQuot == true)
inline String normalizePathC(const String& path, bool removeDQuot = true)
{
#ifdef BTF_CPP17
    String _path = fs::path(path).lexically_normal().string();
#else
    String _path = path;

    // Remove the separator characters at the end.
    while (_path.back() == BTF_PATH_SEPARATOR_WIN || _path.back() == BTF_PATH_SEPARATOR_LINUX)
        _path.pop_back();

    size_t len = 0;
    for (size_t pos = 0; pos < _path.size(); ++pos) {
        if (removeDQuot && _path[pos] == '"') {
            _path.erase(pos, 1);
            continue;
        }
    #ifdef _WIN32
        if (_path[pos] == BTF_PATH_SEPARATOR_LINUX)
            _path.replace(pos, 1, 1, BTF_PATH_SEPARATOR_WIN);
    #else
        if (_path[pos] == BTF_PATH_SEPARATOR_WIN)
            _path.replace(pos, 1, 1, BTF_PATH_SEPARATOR_LINUX);
    #endif // _WIN32
        if (_path[pos] == BTF_PATH_SEPARATOR)
            ++len;
        else {
            if (len > 1)
                _path.replace(pos - len, len, 1, BTF_PATH_SEPARATOR);
            len = 0;
        }
    }
#endif // BTF_CPP17
    return _path;
}

// @brief Normalize path following:
// 1. discard the path separator char at end.
// 2. merge the sequential path separator.
// 3. convert the path separator to suit OS.
// @param removeDQuot If it is true, discard the double quotation mark '"'.
// @note Do change the paramter.
// @example "C://path_to/\/file\" => "C:\path_to\file"
// @example ""C:/path_to/file name . ext"" => "C:\path_to\file name . ext" (removeDQuot == true)
inline String& normalizePath(String& path, bool removeDQuot = true)
{
#ifdef BTF_CPP17
    path = fs::path(path).lexically_normal().string();
#else
    // Remove the separator characters at the end.
    while (path.back() == BTF_PATH_SEPARATOR_WIN || path.back() == BTF_PATH_SEPARATOR_LINUX)
        path.pop_back();

    size_t len = 0;
    for (size_t pos = 0; pos < path.size(); ++pos) {
        if (removeDQuot && path[pos] == '"') {
            path.erase(pos, 1);
            continue;
        }
    #ifdef _WIN32
        if (path[pos] == BTF_PATH_SEPARATOR_LINUX)
            path.replace(pos, 1, 1, BTF_PATH_SEPARATOR_WIN);
    #else
        if (path[pos] == BTF_PATH_SEPARATOR_WIN)
            path.replace(pos, 1, 1, BTF_PATH_SEPARATOR_LINUX);
    #endif // _WIN32
        if (path[pos] == BTF_PATH_SEPARATOR) {
            ++len;
        } else {
            if (len > 1)
                path.replace(pos - len, len, 1, BTF_PATH_SEPARATOR);
            len = 0;
        }
    }
#endif // BTF_CPP17
    return path;
}

// @brief Get the normal path which discard the filename and extension.
// @example "C:/path_to/file.ext" => "C:\path_to"
inline String getPathPrefix(const String& path)
{
#ifdef BTF_CPP17
    return fs::path(path).parent_path().string();
#else
    size_t posw = path.rfind(BTF_PATH_SEPARATOR_WIN);
    size_t posl = path.rfind(BTF_PATH_SEPARATOR_LINUX);
    size_t pos = 0;

    if (posw == path.npos && posl == path.npos)
        return path;

    if (posw == path.npos)
        pos = posl;
    else if (posl == path.npos)
        pos = posw;
    else
        pos = std::max({ posw, posl });

    String _path = path.substr(0, pos);

    // Remove the separator characters at the end.
    while (_path.back() == BTF_PATH_SEPARATOR_WIN || _path.back() == BTF_PATH_SEPARATOR_LINUX)
        _path.pop_back();

    return _path;
#endif // BTF_CPP17
}

// @brief Get the normal path which just reserve the filename and extension.
// @example "C:/path_to/file.ext" => "file.ext"
inline String getPathSuffix(const String& path)
{
#ifdef BTF_CPP17
    return fs::path(path).filename().string();
#else
    size_t posw = path.rfind(BTF_PATH_SEPARATOR_WIN);
    size_t posl = path.rfind(BTF_PATH_SEPARATOR_LINUX);
    size_t pos = 0;

    if (posw == path.npos && posl == path.npos)
        return path;

    if (posw == path.npos)
        pos = posl;
    else if (posl == path.npos)
        pos = posw;
    else
        pos = std::max({ posw, posl });

    if (pos == path.size() - 1)
        return "";

    return path.substr(pos + 1);
#endif // BTF_CPP17
}

// @brief Get the file's name, and not include extension.
// @example "C:/path_to/file.ext" => "file"
inline String getFileName(const String& path)
{
#ifdef BTF_CPP17
    return fs::path(path).filename().replace_extension().string();
#else
    String _path = getPathSuffix(path);

    size_t pos = _path.rfind('.');

    if (pos == _path.npos)
        return _path;
    else
        return _path.substr(0, pos);
#endif // BTF_CPP17
}

// @brief Get the file's extension.
// @return Include the symbol dit '.'.
// @example "C:/path_to/file.ext" => ".ext"
inline String getFileExtension(const String& path)
{
#ifdef BTF_CPP17
    return fs::path(path).filename().extension().string();
#else
    String _path = getPathSuffix(path);

    size_t pos = _path.rfind('.');

    if (pos == _path.npos)
        return "";

    return _path.substr(pos);
#endif // BTF_CPP17
}

// @brief Get the specified path's previously directory path's name.
// @example "C:/path_to/file.ext" => "path_to"
inline String getParentName(const String& path)
{
#ifdef BTF_CPP17
    return fs::path(path).parent_path().filename().string();
#else
    return getPathSuffix(getPathPrefix(path));
#endif // BTF_CPP17
}

// @brief Concatenate the path with prefered separator.
// @example "C:/path_to" "file.ext" => "C:/path_to\file.ext"
inline String pathcat(const String& path1, const String& path2)
{
    return path1 + BTF_PATH_SEPARATOR + path2;
}

// @brief Get the filename of the path (no include file extension)
// and chang the filename to new filename.
// @param path The full path.
// @param newname The new filename.
// @return Return the full path after change the filename.
// @example path = "C:/path_to/oldfile.dat" newname = "newfile" => "C:/path_to/newfile.dat"
// @attention Just base string, not check the actually path.
inline String changeFileName(const String& path, const String& newname)
{
    String prefix = getPathPrefix(path);
    if(normalizePathC(path) == prefix)
        return path;
    return pathcat(prefix, newname + getFileExtension(path));
}

// @brief Whether file or directory exists.
inline bool isExists(const String& path)
{
#ifdef BTF_CPP17
    return fs::exists(path);
#else
    return BTF_ACCESS(path.c_str(), BTF_F_OK) != -1;
#endif // BTF_CPP17
}

// @brief Whether the file exists.
inline bool isExistsFile(const String& path)
{
#ifdef BTF_CPP17
    return isExists(path) && fs::is_regular_file(path);
#else
    struct stat path_stat;

    if (::stat(path.c_str(), &path_stat) != 0)
        return false;
#ifdef _WIN32
    return path_stat.st_mode == _S_IFREG;
#else
    return ::S_ISREG(path_stat.st_mode);
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief Whether the directory exists.
inline bool isExistsDirectory(const String& path)
{
#ifdef BTF_CPP17
    return isExists(path) && fs::is_directory(path);
#else
    struct stat path_stat;

    if (::stat(path.c_str(), &path_stat) != 0)
        return false;
#ifdef _WIN32
    return path_stat.st_mode == _S_IFDIR;
#else
    return ::S_ISDIR(path_stat.st_mode);
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief Whether file is empty. If it is empty indicates it's size is 0.
// @note If the file is not exists, throw exception.
inline bool isEmptyFile(const String& path)
{
    if (!isExistsFile(path)) {
        throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path);
    }
#ifdef BTF_CPP17
    return fs::is_empty(path);
#else
    struct stat path_stat;

    if (::stat(path.c_str(), &path_stat) != 0)
        return true;

    return path_stat.st_size == 0;
#endif // BTF_CPP17
}

// @brief Whether directory is empty. If it is empty indicates it's not contains anything.
// @note If the directory is not exists, throw exception.
inline bool isEmptyDirectory(const String& path)
{
    if (!isExistsFile(path))
        throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path);
#ifdef BTF_CPP17
    return fs::is_empty(path);
#else
#ifdef _WIN32
    WIN32_FIND_DATAA findData;
    HANDLE hFind = ::FindFirstFileA((path + "\\*").c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE)
        return true;

    int n = 0;
    while (::FindNextFileA(hFind, &findData) != 0)
        if (++n > 2)
            break;

    ::FindClose(hFind);

    // '.' and '..' are always present.
    return n <= 2;
#else
    struct dirent* d;
    DIR* dir = ::opendir(path.c_str());

    if (dir == nullptr)
        return true;

    int n = 0;
    while ((d = ::readdir(dir)))
        if (++n > 2)
            break;

    ::closedir(dir);

    // '.' and '..' are always present.
    return n <= 2;
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief Whether file or directory is empty.
// Empty file indicates it's size is 0.
// Empty directory indicates it's not contains anything.
// @note If the file or directory is not exists, throw exception.
inline bool isEmpty(const String& path)
{
    if (!isExists(path))
        throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path);

    return isEmptyFile(path) || isEmptyDirectory(path);
}

// @brief Get current working directory path of program.
inline String getCurrentPath()
{
#ifdef BTF_CPP17
    return fs::current_path().string();
#else
    char path[MAX_PATH] = {};
#ifdef _WIN32
    if (::GetCurrentDirectoryA(MAX_PATH, path) != 0)
        return path;
    else
        throw BTF_MKERR(BTF_ERR_FAILED_OSAPI,
                                  "Error in GetCurrentDirectoryA(), the error code is " +
                                  std::to_string(::GetLastError()));
#else
    if (::getcwd(path, PATH_MAX))
        return path;
    else
        throw BTF_MKERR(BTF_ERR_FAILED_OSAPI, "Error in getcwd().");
#endif // _WIN32
#endif // BTF_CPP17
}

// TODO comment.
inline uintmax_t getFileSize(const String& path)
{
    if (!isExistsFile(path))
        throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path);
#ifdef BTF_CPP17
    return fs::file_size(path);
#else
#ifdef _WIN32
    DWORD size = ::GetFileSize(getFileHandle(path), NULL);

    if (size == INVALID_FILE_SIZE)
        throw BTF_MKERR(BTF_ERR_FAILED_OSAPI,
                                  "Error in GetFileSize(), the error code is " +
                                  std::to_string(::GetLastError()));

    return size;
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// TODO comment.
inline uintmax_t getDirectorySize(const String& path)
{
    uintmax_t result = 0;
    if (!isExistsDirectory(path))
        throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path);
#ifdef BTF_CPP17
    for (const auto& var : fs::recursive_directory_iterator(path))
        result += var.file_size();

    return result;
#else
#ifdef _WIN32
    // TODO
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// TODO comment.
inline uintmax_t getSize(const String& path)
{
    if (isExistsFile(path))
        return getFileSize(path);
    else if (isExistsDirectory(path))
        return getDirectorySize(path);
    else
        throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path);
}

inline bool createDirectory(const String& path)
{
#ifdef BTF_CPP17
    return fs::create_directories(path);
#else
#ifdef _WIN32
    if (::SHCreateDirectoryExA(NULL, path.c_str(), NULL) == ERROR_SUCCESS)
        return true;
    return false;
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @return If the specified path is not exists or failed to delete, return false, else return true.
inline bool deleteFile(const String& path)
{
    if (!isExistsFile(path))
        return false;
#ifdef BTF_CPP17
    return fs::remove(path);
#else
#ifdef _WIN32
    if (::DeleteFileA(path.c_str()) != 0)
        return true;
    return false;
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @return If the specified path is not exists or failed to delete, return 0,
// else return count of deleted file.
inline uintmax_t deleteDirectory(const String& path)
{
    if (!isExistsDirectory(path))
        return 0;
#ifdef BTF_CPP17
    return fs::remove_all(path);
#else
#ifdef _WIN32
    // TODO
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @return If the specified path is not exists or failed to delete, return 0,
// else return count of deleted file.
inline uintmax_t deletes(const String& path)
{
    if (isExistsFile(path))
        return deleteFile(path) ? 1 : 0;
    else if (isExistsDirectory(path))
        return deleteDirectory(path);
    else
        return 0;
}

// @brief Rename(move) the file or directory to a new location.
// @param src The path of source file or directory.
// @param dst The path of target file or directory.
// @param wp The write policy, if the dst exists and wp is Override,
// the process will override the old file else do nothing.
// @param dstIsEnd Whether the specified dst is a finally path,
// if it is false indicates the dst is a directory, the process will
// add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or
// failed to rename return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
inline bool rename(const String& src, const String& dst,
                   WritePolicy wp = SKIP, bool dstIsEnd = true)
{
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));

    if (!isExists(src) || src == _dst)
        return false;

    if (isExists(_dst)) {
        if (wp == SKIP)
            return true;
        else
            deletes(_dst);
    }
#ifdef BTF_CPP17
    if (dstIsEnd) {
        fs::rename(src, _dst);
    } else {
        if (!isExistsDirectory(dst))
            createDirectory(dst);

        fs::rename(src, _dst);
    }

    return true;
#else
#ifdef _WIN32
    // TODO
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief Copy the file to other location.
// @param src The path of source file.
// @param dst The path of target file.
// @param wp The write policy, if the dst exists and wp is Override,
// the process will override the old file else do nothing.
// @param dstIsEnd Whether the specified dst is a finally path,
// if it is false indicates the dst is a directory, the process will
// add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or
// failed to copy return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
inline bool copyFile(const String& src, const String& dst,
                     WritePolicy wp = SKIP, bool dstIsEnd = true)
{
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));

    if (!isExistsFile(src) || src == _dst)
        return false;

    if (!dstIsEnd && !isExistsDirectory(dst))
        createDirectory(dst);
#ifdef BTF_CPP17
    fs::copy_options cop = wp == SKIP ?
        fs::copy_options::skip_existing : fs::copy_options::overwrite_existing;

    return fs::copy_file(src, _dst, cop);
#else
#ifdef _WIN32
    // TODO
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief Copy all files of in the specified directory to other location.
// @param src The path of source directory.
// @param dst The path of target directory.
// @param wp The write policy, if the dst exists and wp is Override,
// the process will override the old file else do nothing.
// @param dstIsEnd Whether the specified dst is a finally path,
// if it is false indicates the dst is a directory, the process will
// add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or
// failed to copy return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
inline bool copyDirectory(const String& src, const String& dst,
                          WritePolicy wp = SKIP, bool dstIsEnd = true)
{
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));

    if (!isExistsDirectory(src) || src == dst)
        return false;

    if (!dstIsEnd && !isExistsDirectory(dst))
        createDirectory(dst);
#ifdef BTF_CPP17
    fs::copy_options cop = wp == SKIP ?
        fs::copy_options::skip_existing : fs::copy_options::overwrite_existing;

    fs::copy(src, _dst, cop);

    return true;
#else
#ifdef _WIN32
    // TODO
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief The copyFile and copyDirectory in one.
inline bool copys(const String& src, const String& dst,
                  WritePolicy wp = SKIP, bool dstIsEnd = true)
{
    if (isExistsFile(src))
        return copyFile(src, dst, wp, dstIsEnd);
    else if (isExistsDirectory(src))
        return copyDirectory(src, dst, wp, dstIsEnd);
    else
        return false;
}

// TODO comment
template<bool isRecursive = true>
inline
std::pair<Strings, Strings> getAlls(const String& path, Strings* errorPaths = nullptr,
                                    bool (*filter) (const String&) = nullptr)
{
    if (!isExistsDirectory(path))
        throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path);

    Strings dirs;
    Strings files;
#ifdef BTF_CPP17
    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (std::exception&) {
                if (errorPaths)
                    errorPaths->push_back(var.path().u8string());
                continue;
            }

            if (var.is_regular_file() && (filter == nullptr || filter(name)))
                files.push_back(name);
            else if (var.is_directory() && (filter == nullptr || filter(name)))
                dirs.push_back(name);
            else
                continue;
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (std::exception&) {
                if (errorPaths)
                    errorPaths->push_back(var.path().u8string());
                continue;
            }

            if (var.is_regular_file() && (filter == nullptr || filter(name)))
                files.push_back(name);
            else if (var.is_directory() && (filter == nullptr || filter(name)))
                dirs.push_back(name);
            else
                continue;
        }
    }

    return std::pair<Strings, Strings>(files, dirs);
#else
#ifdef _WIN32

#else

#endif // _WIN32
#endif // BTF_CPP17
}

// TODO comment
template<bool isRecursive = true>
inline
Strings getAllFiles(const String& path, Strings* errorPaths = nullptr,
                    bool (*filter) (const String&) = nullptr)
{
    if (!isExistsDirectory(path))
        throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path);

    Strings files;
#ifdef BTF_CPP17
    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (std::exception&) {
                if (errorPaths)
                    errorPaths->push_back(var.path().u8string());
                continue;
            }

            if (var.is_regular_file() && (filter == nullptr || filter(name)))
                files.push_back(name);
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (std::exception&) {
                if (errorPaths)
                    errorPaths->push_back(var.path().u8string());
                continue;
            }

            if (var.is_regular_file() && (filter == nullptr || filter(name)))
                files.push_back(name);
        }
    }

    return files;
#else
#ifdef _WIN32

#else

#endif // _WIN32
#endif // BTF_CPP17
}

// TODO comment.
template<bool isRecursive = true>
inline
Strings getAllDirectorys(const String& path, Strings* errorPaths = nullptr,
                         bool (*filter) (const String&) = nullptr)
{
    if (!isExistsDirectory(path))
        throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path);

    Strings dirs;
#ifdef BTF_CPP17
    if (isRecursive) {
        for (const auto& var : fs::recursive_directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (std::exception&) {
                if (errorPaths)
                    errorPaths->push_back(var.path().u8string());
                continue;
            }

            if (var.is_directory() && (filter == nullptr || filter(name)))
                dirs.push_back(name);
        }
    } else {
        for (const auto& var : fs::directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (std::exception&) {
                if (errorPaths)
                    errorPaths->push_back(var.path().u8string());
                continue;
            }

            if (var.is_directory() && (filter == nullptr || filter(name)))
                dirs.push_back(name);
        }
    }

    return dirs;
#else
#ifdef _WIN32

#else

#endif // _WIN32
#endif // BTF_CPP17
}

// @brief Create a file's symlink.
// @param src The path of source file.
// @param dst The path of target file.
// @param wp The write policy, if the dst exists and wp is Override,
// the process will override the old file else do nothing.
// @param dstIsEnd Whether the specified dst is a finally path,
// if it is false indicates the dst is a directory, the process will
// add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or
// failed to create symlink return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
inline bool createFileSymlink(const String& src, const String& dst,
                              WritePolicy wp = SKIP, bool dstIsEnd = true)
{
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));

    if (!isExistsFile(src) || src == _dst)
        return false;

    if (isExistsFile(_dst)) {
        if (wp == SKIP)
            return true;
        else
            deleteFile(_dst);
    }

    if (!dstIsEnd && !isExistsDirectory(dst))
        createDirectory(dst);
#ifdef BTF_CPP17
    fs::create_symlink(src, _dst);

    return true;
#else
#ifdef _WIN32
    if (::CreateSymbolicLinkA(_dst.c_str(), src.c_str(), 0) != 0)
        return true;
    return false;
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief Create a directory's symlink.
// @param src The path of source directory.
// @param dst The path of target directory.
// @param wp The write policy, if the dst exists and wp is Override,
// the process will override the old file else do nothing.
// @param dstIsEnd Whether the specified dst is a finally path,
// if it is false indicates the dst is a directory, the process will
// add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or
// failed to create symlink return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
inline bool createDirectorySymlink(const String& src, const String& dst,
                                   WritePolicy wp = SKIP, bool dstIsEnd = true)
{
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));

    if (!isExistsDirectory(src) || src == _dst)
        return false;

    if (isExistsDirectory(_dst)) {
        if (wp == SKIP)
            return true;
        else
            deleteDirectory(_dst);
    }

    if (!dstIsEnd && !isExistsDirectory(dst))
        createDirectory(dst);
#ifdef BTF_CPP17
    fs::create_directory_symlink(src, _dst);

    return true;
#else
#ifdef _WIN32
    if (::CreateSymbolicLinkA(_dst.c_str(), src.c_str(), 1) != 0)
        return true;
    return false;
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief The createFileSymlink and createDirectorySymlink in one.
inline bool createSymlink(const String& src, const String& dst,
                          WritePolicy wp = SKIP, bool dstIsEnd = true)
{
    if (isExistsFile(src))
        return createFileSymlink(src, dst, wp, dstIsEnd);
    else if (isExistsDirectory(src))
        return createDirectorySymlink(src, dst, wp, dstIsEnd);
    else
        return false;
}

// @brief Create a hardlink.
// @param src The path of source.
// @param to The path of destination.
// @param wp The write policy, if the dst exists and wp is Override,
// the process will override the old file else do nothing.
// @param dstIsEnd Whether the specified dst is a finally path,
// if it is false indicates the dst is a directory, the process will
// add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or
// failed to create hardlink return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
inline bool createHardlink(const String& src, const String& dst,
                           WritePolicy wp = SKIP, bool dstIsEnd = true)
{
// Get the finally path of to.
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));

    // If the src path not exists return false, do nothing.
    if (!isExistsFile(src) || src == _dst)
        return false;

    if (isExistsFile(_dst)) {
        // If the dst path exists and wp is Override delete old file first.
        if (wp == SKIP)
            return true;
        else
            deleteFile(_dst);
    }

    if (!dstIsEnd && !isExistsDirectory(dst))
        createDirectory(dst);
#ifdef BTF_CPP17
    fs::create_hard_link(src, _dst);

    return true;
#else
#ifdef _WIN32
    if (::CreateHardLinkA(_dst.c_str(), src.c_str(), NULL) != 0)
        return true;
    return false;
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

inline uintmax_t getHardlinkCount(const String& path)
{
    if (!isExists(path))
        throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path);
#ifdef BTF_CPP17
    return fs::hard_link_count(path);
#else
#ifdef _WIN32
    // TODO
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

}

namespace btf
{

// classes.

class File
{
public:
    File() = default;

    explicit File(const String& name) : name_(name) {}

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

    ~File()
    {
        clear();
    }

    File copy() const
    {
        return File(*this);
    }

    static File fromPath(const String& filename)
    {
        if (!isExistsFile(filename))
            throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, filename);

        std::ifstream ifs(filename.data(), std::ios_base::binary);

        if (!ifs.is_open())
            throw BTF_MKERR(BTF_ERR_FILE_OPEN_FAILED, filename);

        File file(getPathSuffix(filename));
        file << ifs;

        ifs.close();

        return file;
    }

    String name() const
    {
        return name_;
    }

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

    bool empty() const
    {
        if (data_ == nullptr)
            return true;
        return data_->empty();
    }

    void setName(const String& name)
    {
        name_ = name;
    }

    void clear()
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
        String _path = path + BTF_PATH_SEPARATOR + name_;

        if (isExistsFile(_path) && wp == SKIP)
            return;

        std::ofstream ofs(_path.data(), openmode);

        if (!ofs.is_open())
            throw BTF_MKERR(BTF_ERR_FILE_OPEN_FAILED, _path);

        write(ofs);

        ofs.close();
    }

    File& operator=(const File& other)
    {
        name_ = other.name_;

        clear();

        if (other.data_)
            data_ = new String(*other.data_);

        return *this;
    }

    File& operator=(const String& data)
    {
        clear();

        data_ = new String(data);

        return *this;
    }

    template<typename T>
    File& operator=(const std::vector<T>& data)
    {
        clear();

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

        char buffer[BUFFER_SIZE] = {};
        while (is.read(buffer, BUFFER_SIZE))
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

    explicit Dir(const String& name) : name_(name) {}

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
        other.name_.clear();

        subFiles_ = other.subFiles_;
        other.subFiles_ = nullptr;

        subDirs_ = other.subDirs_;
        other.subDirs_ = nullptr;
    }

    ~Dir()
    {
        clearFiles();
        clearDirs();
    }

    Dir copy() const
    {
        return Dir(*this);
    }

    static Dir fromPath(const String& dirpath)
    {
        if (!isExistsDirectory(dirpath))
            throw BTF_MKERR(BTF_ERR_UNEXISTS_PATH, dirpath);

        Dir root(getPathSuffix(dirpath));

        auto dirs = getAllDirectorys<false>(dirpath);
        for (const auto& var : dirs)
            root << Dir::fromPath(var);

        auto files = getAllFiles<false>(dirpath);
        for (const auto& var : files)
            root << File::fromPath(var);

        return root;
    }

    String name() const
    {
        return name_;
    }

    size_t size() const
    {
        size_t size = 0;

        if (subFiles_) {
            for (const auto& var : *subFiles_)
                size += var.size();
        }

        if (subDirs_) {
            for (const auto& var : *subDirs_)
                size += var.size();
        }

        return size;
    }

    size_t fileCount(bool isRecursive = true) const
    {
        size_t cnt = 0;

        if (subFiles_)
            cnt += subFiles_->size();

        if (isRecursive && subDirs_) {
            for (const auto& var : *subDirs_)
                cnt += var.fileCount();
        }

        return cnt;
    }

    size_t dirCount(bool isRecursive = true) const
    {
        size_t cnt = 0;

        if (subDirs_)
            cnt += subDirs_->size();

        if (isRecursive && subDirs_) {
            for (const auto& var : *subDirs_)
                cnt += var.dirCount();
        }

        return cnt;
    }

    size_t count(bool isRecursive = true) const
    {
        return fileCount(isRecursive) + dirCount(isRecursive);
    }

    bool empty() const
    {
        return (subFiles_ == nullptr || subFiles_->empty()) &&
            (subDirs_ == nullptr || subDirs_->empty());
    }

    bool hasFile(const String& name, bool isRecursive = false) const
    {
        if (hasFile_(name) != 0)
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
        if (hasDir_(name) != 0)
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
        name_ = name;
    }

    const Vec<File>& files() const
    {
        return *subFiles_;
    }

    const Vec<Dir>& dirs() const
    {
        return *subDirs_;
    }

    Vec<File>& files()
    {
        return *subFiles_;
    }

    Vec<Dir>& dirs()
    {
        return *subDirs_;
    }

    File& file(const String& name)
    {
        size_t pos = hasFile_(name);

        if (pos == 0) {
            addFile(name);
            return subFiles_->back();
        }

        return (*subFiles_)[pos - 1];
    }

    Dir& dir(const String& name)
    {
        size_t pos = hasDir_(name);

        if (pos == 0) {
            addDir(name);
            return subDirs_->back();
        }

        return (*subDirs_)[pos - 1];
    }

    void removeFile(const String& name)
    {
        size_t pos = hasFile_(name);

        if (pos == 0)
            return;

        subFiles_->erase(subFiles_->begin() + pos - 1);
    }

    void removeDir(const String& name)
    {
        size_t pos = hasDir_(name);

        if (pos == 0)
            return;

        subDirs_->erase(subDirs_->begin() + pos - 1);
    }

    void remove(const File& file)
    {
        removeFile(file.name());
    }

    void remove(const Dir& dir)
    {
        removeDir(dir.name());
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

    void add(File& file, WritePolicy wp = SKIP)
    {
        if (subFiles_ == nullptr)
            subFiles_ = new Vec<File>();

        size_t pos = hasFile_(file.name());

        if (pos != 0) {
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

        if (pos != 0) {
            if (wp == OVERRIDE)
                (*subDirs_)[pos] = std::move(dir);

            return;
        }

        subDirs_->emplace_back(std::move(dir));
    }

    void add(File&& file)
    {
        add(file);
    }

    void add(Dir&& dir)
    {
        add(dir);
    }

    void addFile(const String& name)
    {
        add(File(name));
    }

    void addDir(const String& name)
    {
        add(Dir(name));
    }

    void write(const String& path, WritePolicy wp = SKIP,
               std::ios_base::openmode openmode = std::ios_base::binary) const
    {
        String root = String(path) + BTF_PATH_SEPARATOR + name_;

        createDirectory(root);

        if (subFiles_) {
            for (const auto& var : *subFiles_)
                var.write(root, wp, openmode);
        }

        if (subDirs_) {
            for (const auto& var : *subDirs_)
                var.write(root, wp);
        }
    }

    Dir& operator=(const Dir& other)
    {
        name_ = other.name_;

        clearFiles();
        clearDirs();

        if (other.subFiles_)
            subFiles_ = new Vec<File>(*other.subFiles_);

        if (other.subDirs_)
            subDirs_ = new Vec<Dir>(*other.subDirs_);

        return *this;
    }

    Dir& operator[](const String& name)
    {
        return dir(name);
    }

    File& operator()(const String& name)
    {
        return file(name);
    }

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
    size_t hasFile_(const String& name) const
    {
        if (subFiles_) {
            for (size_t i = 0; i < subFiles_->size(); i++) {
                if ((*subFiles_)[i].name() == name)
                    return i + 1;
            }
        }

        return 0;
    }

    size_t hasDir_(const String& name) const
    {
        if (subDirs_) {
            for (size_t i = 0; i < subDirs_->size(); i++) {
                if ((*subDirs_)[i].name() == name)
                    return i + 1;
            }
        }

        return 0;
    }

    String name_;
    Vec<File>* subFiles_ = nullptr;
    Vec<Dir>* subDirs_ = nullptr;
};

}

#endif // !BETTERFILE_H
