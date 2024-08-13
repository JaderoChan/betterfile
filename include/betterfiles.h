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

#include <cstdint> // uintmax_t

#include <vector>
#include <string>
#include <fstream>
#include <iostream>
#include <exception>

#ifndef __cplusplus
#error BetterFiles library must be used in c++.
#endif // !__cplusplus

#ifdef _MSVC_LANG
#define BTF_CPPVERS     _MSVC_LANG
#else
#define Bf_CPPVERS      __cpluscplus
#endif // _MSVC_LANG

#if BTF_CPPVERS >= 201703L
#define BTF_CPP17
#endif // BTF_CPPVERS >= 201703L

// TODO
//#if BTF_CPPVERS < 201103L
//#error BetterFiles library must be used in c++11 or newer.
//#endif // BTF_CPPVERS < 201103L

// Just be used for inline function and avoid redefine.
#define BTF_INLINE          inline
// The function has this attribute indicates it's return value not should discarded.
#define BTF_NODISCARD       [[nodiscard]]
// The function has this attribute indicates it's may removed in the future.
#define BTF_DEPRECATED      [[deprecated]]

#define BTF_PATH_SEPARATOR_WIN          '\\'
#define BTF_PATH_SEPARATOR_LINUX        '/'

#ifdef _WIN32
#define BTF_PATH_SEPARATOR      BTF_PATH_SEPARATOR_WIN
#else
#define BTF_PATH_SEPARATOR      BTF_PATH_SEPARATOR_LINUX
#endif // _WIN32

// Some char constexpr.
#define BTF_CHARV_SPACE     ' '
#define BTF_CHARV_DOT       '.'
#define BTF_CHARV_SQUOT     '"'
#define BTF_CHARV_DQUOT     '"'
#define BTF_CHARV_END       '\0'
#define BTF_CHARV_ENTER     '\n'

#define BTF_EMPTY_STR       ""

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

namespace Btf
{
// BetterFiles's type redefine.

template<typename T>
using Vec = std::vector<T>;
using String = std::string;
using Strings = Vec<String>;
using IOStream = std::iostream;
using IStream = std::istream;
using OStream = std::ostream;
using FStream = std::fstream;
using IFStream = std::ifstream;
using OFStream = std::ofstream;
using Exception = std::exception;

}

#define BTF_ERROR_HINT          "[BetterFiles Error] "
#define BTF_MKERR(et,added)     (Btf::String(BTF_ERROR_HINT) + et + BTF_CHARV_SPACE + added).c_str()

namespace Btf
{
// BetterFiles's constexprs, consts, and enums.

constexpr const int kBufferSize = 4096;

enum WritePolicy : unsigned char
{
    // Skip write process when the file exists.
    Skip = 1,
    // Override the old file.
    Override
};

}

#ifdef BTF_CPP17
#include <string_view>
#include <filesystem>
namespace Btf
{

namespace Fs = std::filesystem;

}
#else
#include <sys/stat.h>

#ifdef _WIN32
#include <io.h>
#include <windows.h>
#include <ShlObj.h>

#undef min
#undef max

#define BTF_ACCESS _access
#define BTF_F_OK 0

#else
#include <unistd.h>
#include <dirent.h>
#endif  // _WIN32

namespace Btf
{
// BetterFiles's aux functions.

BTF_NODISCARD BTF_INLINE std::wstring string2wstring(const String &str) {
    std::wstring result;
#ifdef _WIN32
    int len = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int) str.size(), NULL, 0);
    if (len == 0) {
        return result;
    }
    wchar_t *buffer = new wchar_t[len + 1];
    int rtn = MultiByteToWideChar(CP_ACP, 0, str.c_str(), (int) str.size(), buffer, len);
    if (rtn == 0) {
        return result;
    }
    buffer[len] = BTF_CHARV_END;
    result = buffer;
    delete[] buffer;
#else
    // TODO for linux.
#endif // _WIN32
    return result;
}

BTF_NODISCARD BTF_INLINE String wstring2string(const std::wstring &wstr) {
    String result;
#ifdef _WIN32
    int len = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int) wstr.size(), NULL, 0, NULL, NULL);
    if (len == 0) {
        return result;
    }
    char *buffer = new char[len + 1];
    int rtn = WideCharToMultiByte(CP_ACP, 0, wstr.c_str(), (int) wstr.size(), buffer, len, NULL, NULL);
    if (rtn == 0) {
        return result;
    }
    buffer[len] = BTF_CHARV_END;
    result = buffer;
    delete[] buffer;
#else
    // TODO for linux.
#endif // _WIN32
    return result;
}

#ifdef _WIN32
HANDLE getFileHandle(const String &path) {
    WIN32_FIND_DATAA fd;
    return FindFirstFileA(path.c_str(), &fd);
}
#endif // _WIN32

}

#endif // BTF_CPP17

namespace Btf
{
// BetterFiles's utility functions.

// @brief Normalize path following:
// 1. discard the path separator char at end.
// 2. merge the sequential path separator.
// 3. convert the path separator to suit OS.
// @param removeDQuot If it is true, discard the double quotation mark '"'.
// @note Do not change the paramter.
// @example "C://path_to/\/file\" => "C:\path_to\file"
// @example ""C:/path_to/file name . ext"" => "C:\path_to\file name . ext" (removeDQuot == true)
BTF_NODISCARD BTF_INLINE String normalizePathC(const String &path, bool removeDQuot = true) {
#ifdef BTF_CPP17
    String _path = Fs::path(path).lexically_normal().string();
#else
    String _path = path;
    while (_path.back() == BTF_PATH_SEPARATOR_WIN || _path.back() == BTF_PATH_SEPARATOR_LINUX) {
        _path.pop_back();
    }
    size_t len = 0;
    for (size_t pos = 0; pos < _path.size(); ++pos) {
        if (removeDQuot && _path[pos] == BTF_CHARV_DQUOT) {
            _path.erase(pos, 1);
            continue;
        }
#ifdef _WIN32
        if (_path[pos] == BTF_PATH_SEPARATOR_LINUX) {
            _path.replace(pos, 1, 1, BTF_PATH_SEPARATOR_WIN);
        }
#else
        if (_path[pos] == BTF_PATH_SEPARATOR_WIN) {
            _path.replace(pos, 1, 1, BTF_PATH_SEPARATOR_LINUX);
        }
#endif // _WIN32
        if (_path[pos] == BTF_PATH_SEPARATOR) {
            ++len;
        } else {
            if (len > 1) {
                _path.replace(pos - len, len, 1, BTF_PATH_SEPARATOR);
            }
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
BTF_INLINE String &normalizePath(String &path, bool removeDQuot = true) {
#ifdef BTF_CPP17
    path = Fs::path(path).lexically_normal().string();
#else
    while (path.back() == BTF_PATH_SEPARATOR_WIN || path.back() == BTF_PATH_SEPARATOR_LINUX) {
        path.pop_back();
    }
    size_t len = 0;
    for (size_t pos = 0; pos < path.size(); ++pos) {
        if (removeDQuot && path[pos] == BTF_CHARV_DQUOT) {
            path.erase(pos, 1);
            continue;
        }
#ifdef _WIN32
        if (path[pos] == BTF_PATH_SEPARATOR_LINUX) {
            path.replace(pos, 1, 1, BTF_PATH_SEPARATOR_WIN);
        }
#else
        if (path[pos] == BTF_PATH_SEPARATOR_WIN) {
            path.replace(pos, 1, 1, BTF_PATH_SEPARATOR_LINUX);
        }
#endif // _WIN32
        if (path[pos] == BTF_PATH_SEPARATOR) {
            ++len;
        } else {
            if (len > 1) {
                path.replace(pos - len, len, 1, BTF_PATH_SEPARATOR);
            }
            len = 0;
        }
    }
#endif // BTF_CPP17
    return path;
}

// @brief Get the normal path which discard the filename and extension.
// @example "C:/path_to/file.ext" => "C:\path_to"
BTF_NODISCARD BTF_INLINE String getPathPrefix(const String &path) {
#ifdef BTF_CPP17
    return Fs::path(path).parent_path().string();
#else
    size_t posw = path.rfind(BTF_PATH_SEPARATOR_WIN);
    size_t posl = path.rfind(BTF_PATH_SEPARATOR_LINUX);
    size_t pos = 0;
    if (posw == path.npos && posl == path.npos) {
        return path;
    } else {
        if (posw == path.npos) {
            pos = posl;
        } else if (posl == path.npos) {
            pos = posw;
        } else {
            pos = std::max({ posw, posl });
        }
    }
    String _path = path.substr(0, pos);
    while (_path.back() == BTF_PATH_SEPARATOR_WIN || _path.back() == BTF_PATH_SEPARATOR_LINUX) {
        _path.pop_back();
    }
    return _path;
#endif // BTF_CPP17
}

// @brief Get the normal path which just reserve the filename and extension.
// @example "C:/path_to/file.ext" => "file.ext"
BTF_NODISCARD BTF_INLINE String getPathSuffix(const String &path) {
#ifdef BTF_CPP17
    return Fs::path(path).filename().string();
#else
    size_t posw = path.rfind(BTF_PATH_SEPARATOR_WIN);
    size_t posl = path.rfind(BTF_PATH_SEPARATOR_LINUX);
    size_t pos = 0;
    if (posw == path.npos && posl == path.npos) {
        return path;
    } else {
        if (posw == path.npos) {
            pos = posl;
        } else if (posl == path.npos) {
            pos = posw;
        } else {
            pos = std::max({ posw, posl });
        }
    }
    if (pos == path.size() - 1) {
        return BTF_EMPTY_STR;
    }
    return path.substr(pos + 1);
#endif // BTF_CPP17
}

// @brief Get the file's name, and not include extension.
// @example "C:/path_to/file.ext" => "file"
BTF_NODISCARD BTF_INLINE String getFileName(const String &path) {
#ifdef BTF_CPP17
    return Fs::path(path).filename().replace_extension().string();
#else
    String _path = getPathSuffix(path);
    size_t pos = _path.rfind(BTF_CHARV_DOT);
    if (pos == _path.npos) {
        return _path;
    } else {
        return _path.substr(0, pos);
    }
#endif // BTF_CPP17
}

// @brief Get the file's extension.
// @return Include the symbol dit '.'.
// @example "C:/path_to/file.ext" => ".ext"
BTF_NODISCARD BTF_INLINE String getFileExtension(const String &path) {
#ifdef BTF_CPP17
    return Fs::path(path).filename().extension().string();
#else
    String _path = getPathSuffix(path);
    size_t pos = _path.rfind(BTF_CHARV_DOT);
    if (pos == _path.npos) {
        return _path;
    } else {
        if (pos == _path.size() - 1) {
            return BTF_EMPTY_STR;
        }
        return _path.substr(pos + 1);
    }
#endif // BTF_CPP17
}

// @brief Get the specified path's previously directory path's name.
// @example "C:/path_to/file.ext" => "path_to"
BTF_NODISCARD BTF_INLINE String getParentName(const String &path) {
#ifdef BTF_CPP17
    return Fs::path(path).parent_path().filename().string();
#else
    return getPathSuffix(getPathPrefix(path));
#endif // BTF_CPP17
}

// @brief Concatenate the path with prefered separator.
// @example "C:/path_to" "file.ext" => "C:/path_to\file.ext"
BTF_NODISCARD BTF_INLINE String pathcat(const String &path1, const String &path2) {
    return path1 + BTF_PATH_SEPARATOR + path2;
}

// @brief Whether file or directory exists.
BTF_NODISCARD BTF_INLINE bool isExists(const String &path) {
#ifdef BTF_CPP17
    return Fs::exists(path);
#else
    return BTF_ACCESS(path.c_str(), BTF_F_OK) != -1;
#endif // BTF_CPP17
}

// @brief Whether the file exists.
BTF_NODISCARD BTF_INLINE bool isExistsFile(const String &path) {
#ifdef BTF_CPP17
    return isExists(path) && Fs::is_regular_file(path);
#else
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0)
        return false;
#ifdef _WIN32
    return path_stat.st_mode == _S_IFREG;
#else
    return S_ISREG(path_stat.st_mode);
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief Whether the directory exists.
BTF_NODISCARD BTF_INLINE bool isExistsDirectory(const String &path) {
#ifdef BTF_CPP17
    return isExists(path) && Fs::is_directory(path);
#else
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0)
        return false;
#ifdef _WIN32
    return path_stat.st_mode == _S_IFDIR;
#else
    return S_ISDIR(path_stat.st_mode);
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief Whether file is empty. If it is empty indicates it's size is 0.
// @note If the file is not exists, throw exception.
BTF_NODISCARD BTF_INLINE bool isEmptyFile(const String &path) {
    if (!isExistsFile(path)) {
        throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path));
    }
#ifdef BTF_CPP17
    return Fs::is_empty(path);
#else
    struct stat path_stat;
    if (stat(path.c_str(), &path_stat) != 0)
        return true;
    return path_stat.st_size == 0;
#endif // BTF_CPP17
}

// @brief Whether directory is empty. If it is empty indicates it's not contains anything.
// @note If the directory is not exists, throw exception.
BTF_NODISCARD BTF_INLINE bool isEmptyDirectory(const String &path) {
    if (!isExistsFile(path)) {
        throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path));
    }
#ifdef BTF_CPP17
    return Fs::is_empty(path);
#else
#ifdef _WIN32
    WIN32_FIND_DATA findData;
    HANDLE hFind = FindFirstFileW(string2wstring(path + "\\*").c_str(), &findData);

    if (hFind == INVALID_HANDLE_VALUE)
        return true;

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
        return true;

    int n = 0;
    while ((d = readdir(dir)) != nullptr)
        if (++n > 2)
            break;

    closedir(dir);
    // '.' and '..' are always present.
    return n <= 2;
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief Whether file or directory is empty.
// Empty file indicates it's size is 0.
// Empty directory indicates it's not contains anything.
// @note If the file or directory is not exists, throw exception.
BTF_NODISCARD BTF_INLINE bool isEmpty(const String &path) {
    if (!isExists(path)) {
        throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path));
    }
    return isEmptyFile(path) || isEmptyDirectory(path);
}

// @brief Get current working directory path of program.
BTF_INLINE String getCurrentPath() {
#ifdef BTF_CPP17
    return Fs::current_path().string();
#else
#ifdef _WIN32
    char path[MAX_PATH];
    if (GetCurrentDirectoryA(MAX_PATH, path) != 0) {
        return path;
    } else {
        throw Exception(BTF_MKERR(BTF_ERR_FAILED_OSAPI,
                                  "Error in GetCurrentDirectoryA(), the error code is " + std::to_string(GetLastError())));
    }
#else
    char path[PATH_MAX];
    if (getcwd(path, PATH_MAX) != nullptr) {
        return path;
    } else {
        throw Exception(BTF_MKERR(BTF_ERR_FAILED_OSAPI,
                                  "Error in getcwd()."));
    }
#endif // _WIN32
#endif // BTF_CPP17
}

// TODO comment.
BTF_NODISCARD BTF_INLINE uintmax_t getFileSize(const String &path) {
    if (!isExistsFile(path)) {
        throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path));
    }
#ifdef BTF_CPP17
    return Fs::file_size(path);
#else
#ifdef _WIN32
    DWORD size = GetFileSize(getFileHandle(path), NULL);
    if (size == INVALID_FILE_SIZE) {
        throw Exception(BTF_MKERR(BTF_ERR_FAILED_OSAPI,
                                  "Error in GetFileSize(), the error code is " + std::to_string(GetLastError())));
    }
    return size;
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// TODO comment.
BTF_NODISCARD BTF_INLINE uintmax_t getDirectorySize(const String &path) {
    uintmax_t result = 0;
    if (!isExistsDirectory(path)) {
        throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path));
    }
#ifdef BTF_CPP17
    for (auto &var : Fs::recursive_directory_iterator(path)) {
        result += var.file_size();
    }
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
BTF_NODISCARD BTF_INLINE uintmax_t getSize(const String &path) {
    if (isExistsFile(path)) {
        return getFileSize(path);
    } else if (isExistsDirectory(path)) {
        return getDirectorySize(path);
    } else {
        throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path));
    }
}

BTF_INLINE bool createDirectory(const String &path) {
#ifdef BTF_CPP17
    return Fs::create_directories(path);
#else
#ifdef _WIN32
    if (SHCreateDirectoryExA(NULL, path.c_str(), NULL) == ERROR_SUCCESS) {
        return true;
    } else {
        return false;
    }
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @return If the specified path is not exists or failed to delete, return false, else return true.
BTF_INLINE bool deleteFile(const String &path) {
    if (!isExistsFile(path)) {
        return false;
    }
#ifdef BTF_CPP17
    return Fs::remove(path);
#else
#ifdef _WIN32
    if (DeleteFileA(path.c_str()) == 0) {
        return false;
    } else {
        return true;
    }
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @return If the specified path is not exists or failed to delete, return 0, else return count of deleted file.
BTF_INLINE uintmax_t deleteDirectory(const String &path) {
    if (!isExistsDirectory(path)) {
        return 0;
    }
#ifdef BTF_CPP17
    return Fs::remove_all(path);
#else
#ifdef _WIN32
    // TODO
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @return If the specified path is not exists or failed to delete, return 0, else return count of deleted file.
BTF_INLINE uintmax_t deletes(const String &path) {
    if (isExistsFile(path)) {
        return deleteFile(path) ? 1 : 0;
    } else if (isExistsDirectory(path)) {
        return deleteDirectory(path);
    } else {
        return 0;
    }
}

// @brief Rename(move) the file or directory to a new location.
// @param src The path of source file or directory.
// @param dst The path of target file or directory.
// @param wp The write policy, if the dst exists and wp is Override,
// the process will override the old file else do nothing.
// @param dstIsEnd Whether the specified dst is a finally path,
// if it is false indicates the dst is a directory, the process will add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or failed to rename return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
BTF_INLINE bool rename(const String &src, const String &dst,
                       WritePolicy wp = Skip, bool dstIsEnd = true)
{
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));
    if (!isExists(src) || src == _dst) {
        return false;
    }
    if (isExists(_dst)) {
        if (wp == Skip) {
            return true;
        } else {
            deletes(_dst);
        }
    }
#ifdef BTF_CPP17
    if (dstIsEnd) {
        Fs::rename(src, _dst);
    } else {
        if (!isExistsDirectory(dst)) {
            createDirectory(dst);
        }
        Fs::rename(src, _dst);
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
// if it is false indicates the dst is a directory, the process will add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or failed to copy return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
BTF_INLINE bool copyFile(const String &src, const String &dst,
                         WritePolicy wp = Skip, bool dstIsEnd = true)
{
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));
    if (!isExistsFile(src) || src == _dst) {
        return false;
    }
    if (!dstIsEnd && !isExistsDirectory(dst)) {
        createDirectory(dst);
    }
#ifdef BTF_CPP17
    Fs::copy_options cop = wp == Skip ?
        Fs::copy_options::skip_existing : Fs::copy_options::overwrite_existing;
    return Fs::copy_file(src, _dst, cop);
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
// if it is false indicates the dst is a directory, the process will add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or failed to copy return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
BTF_INLINE bool copyDirectory(const String &src, const String &dst,
                              WritePolicy wp = Skip, bool dstIsEnd = true) {
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));
    if (!isExistsDirectory(src) || src == dst) {
        return false;
    }
    if (!dstIsEnd && !isExistsDirectory(dst)) {
        createDirectory(dst);
    }
#ifdef BTF_CPP17
    Fs::copy_options cop = wp == Skip ?
        Fs::copy_options::skip_existing : Fs::copy_options::overwrite_existing;
    Fs::copy(src, _dst, cop);
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
BTF_INLINE bool copys(const String &src, const String &dst,
                      WritePolicy wp = Skip, bool dstIsEnd = true) {
    if (isExistsFile(src)) {
        return copyFile(src, dst, wp, dstIsEnd);
    } else if (isExistsDirectory(src)) {
        return copyDirectory(src, dst, wp, dstIsEnd);
    } else {
        return false;
    }
}

// TODO comment
template<bool isRecursive = true>
BTF_NODISCARD BTF_INLINE std::pair<Strings, Strings> getAlls(const String &path, Strings *errorPaths = nullptr,
                                                             bool (*filter) (const String &) = nullptr)
{
    if (!isExistsDirectory(path)) {
        throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path));
    }

    Strings dirs;
    Strings files;
#ifdef BTF_CPP17
    if (isRecursive) {
        for (auto &var : Fs::recursive_directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (Exception &) {
                if (errorPaths != nullptr) {
                    errorPaths->push_back(var.path().u8string());
                }
                continue;
            }

            if (var.is_regular_file() && (filter == nullptr || filter(name))) {
                files.push_back(name);
            } else if (var.is_directory() && (filter == nullptr || filter(name))) {
                dirs.push_back(name);
            } else {
                continue;
            }
        }
    } else {
        for (auto &var : Fs::directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (Exception &) {
                if (errorPaths != nullptr) {
                    errorPaths->push_back(var.path().u8string());
                }
                continue;
            }

            if (var.is_regular_file() && (filter == nullptr || filter(name))) {
                files.push_back(name);
            } else if (var.is_directory() && (filter == nullptr || filter(name))) {
                dirs.push_back(name);
            } else {
                continue;
            }
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
BTF_NODISCARD BTF_INLINE Strings getAllFiles(const String &path, Strings *errorPaths = nullptr,
                                             bool (*filter) (const String &) = nullptr)
{
    if (!isExistsDirectory(path)) {
        throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path));
    }

    Strings files;
#ifdef BTF_CPP17
    if (isRecursive) {
        for (auto &var : Fs::recursive_directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (Exception &) {
                if (errorPaths != nullptr) {
                    errorPaths->push_back(var.path().u8string());
                }
                continue;
            }

            if (var.is_regular_file() && (filter == nullptr || filter(name))) {
                files.push_back(name);
            }
        }
    } else {
        for (auto &var : Fs::directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (Exception &) {
                if (errorPaths != nullptr) {
                    errorPaths->push_back(var.path().u8string());
                }
                continue;
            }

            if (var.is_regular_file() && (filter == nullptr || filter(name))) {
                files.push_back(name);
            }
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
BTF_NODISCARD BTF_INLINE Strings getAllDirectorys(const String &path, Strings *errorPaths = nullptr,
                                                  bool (*filter) (const String &) = nullptr)
{
    if (!isExistsDirectory(path)) {
        throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path));
    }

    Strings dirs;
#ifdef BTF_CPP17
    if (isRecursive) {
        for (auto &var : Fs::recursive_directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (Exception &) {
                if (errorPaths != nullptr) {
                    errorPaths->push_back(var.path().u8string());
                }
                continue;
            }

            if (var.is_directory() && (filter == nullptr || filter(name))) {
                dirs.push_back(name);
            }
        }
    } else {
        for (auto &var : Fs::directory_iterator(path)) {
            String name;
            try {
                name = var.path().string();
            } catch (Exception &) {
                if (errorPaths != nullptr) {
                    errorPaths->push_back(var.path().u8string());
                }
                continue;
            }

            if (var.is_directory() && (filter == nullptr || filter(name))) {
                dirs.push_back(name);
            }
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
// if it is false indicates the dst is a directory, the process will add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or failed to copy return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
BTF_INLINE bool createFileSymlink(const String &src, const String &dst,
                                  WritePolicy wp = Skip, bool dstIsEnd = true)
{
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));
    if (!isExistsFile(src) || src == _dst) {
        return false;
    }
    if (isExistsFile(_dst)) {
        if (wp == Skip) {
            return true;
        } else {
            deleteFile(_dst);
        }
    }
    if (!dstIsEnd && !isExistsDirectory(dst)) {
        createDirectory(dst);
    }
#ifdef BTF_CPP17
    Fs::create_symlink(src, _dst);
    return true;
#else
#ifdef _WIN32
    if (CreateSymbolicLinkA(_dst.c_str(), src.c_str(), 0) == 0) {
        return false;
    } else {
        return true;
    }
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
// if it is false indicates the dst is a directory, the process will add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or failed to copy return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
BTF_INLINE bool createDirectorySymlink(const String &src, const String &dst,
                                       WritePolicy wp = Skip, bool dstIsEnd = true)
{
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));
    if (!isExistsDirectory(src) || src == _dst) {
        return false;
    }
    if (isExistsDirectory(_dst)) {
        if (wp == Skip) {
            return true;
        } else {
            deleteDirectory(_dst);
        }
    }
    if (!dstIsEnd && !isExistsDirectory(dst)) {
        createDirectory(dst);
    }
#ifdef BTF_CPP17
    Fs::create_directory_symlink(src, _dst);
    return true;
#else
#ifdef _WIN32
    if (CreateSymbolicLinkA(_dst.c_str(), src.c_str(), 1) == 0) {
        return false;
    } else {
        return true;
    }
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

// @brief The createFileSymlink and createDirectorySymlink in one.
BTF_INLINE bool createSymlink(const String &src, const String &dst,
                              WritePolicy wp = Skip, bool dstIsEnd = true)
{
    if (isExistsFile(src)) {
        return createFileSymlink(src, dst, wp, dstIsEnd);
    } else if (isExistsDirectory(src)) {
        return createDirectorySymlink(src, dst, wp, dstIsEnd);
    } else {
        return false;
    }
}

// @brief Create a hardlink.
// @param src The path of source.
// @param to The path of destination.
// @param wp The write policy, if the dst exists and wp is Override,
// the process will override the old file else do nothing.
// @param dstIsEnd Whether the specified dst is a finally path,
// if it is false indicates the dst is a directory, the process will add the src's filename to dst's end.
// @return If the src is not exists or src is equal destination or failed to copy return false, else return true.
// @note If the dst is not exists and dstIsEnd is false, create the destination directory first.
BTF_INLINE bool createHardlink(const String &src, const String &dst,
                               WritePolicy wp = Skip, bool dstIsEnd = true)
{
    // Get the finally path of to.
    String _dst = dstIsEnd ? dst : pathcat(dst, getPathSuffix(src));

    // If the src path not exists return false, do nothing.
    if (!isExistsFile(src) || src == _dst) {
        return false;
    }

    if (isExistsFile(_dst)) {
        if (wp == Skip) {
            return true;
        } else {
            // If the dst path exists and wp is Override delete old file first.
            deleteFile(_dst);
        }
    }

    if (!dstIsEnd && !isExistsDirectory(dst)) {
        createDirectory(dst);
    }
#ifdef BTF_CPP17
    Fs::create_hard_link(src, _dst);
    return true;
#else
#ifdef _WIN32
    if (CreateHardLinkA(_dst.c_str(), src.c_str(), NULL) == 0) {
        return false;
    } else {
        return true;
    }
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

intmax_t getHardlinkCount(const String &path) {
    if (!isExists(path)) {
        throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, path));
    }
#ifdef BTF_CPP17
    return Fs::hard_link_count(path);
#else
#ifdef _WIN32
    // TODO
#else
    // TODO
#endif // _WIN32
#endif // BTF_CPP17
}

}

namespace Btf
{
// BetterFiles's classes.

class File
{
public:
    File() : mData(nullptr) {};

    explicit File(const String &name) : mName(name), mData(nullptr) {}

    File(const File &other) : mData(nullptr) {
        mName = other.mName;
        if (other.mData == nullptr) {
            return;
        }
        mData = new String(*other.mData);
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

    static File fromPath(const String &filepath) {
        if (!isExistsFile(filepath)) {
            throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, filepath));
        }

        IFStream ifs(filepath.data(), std::ios_base::binary);
        if (!ifs.is_open()) {
            throw Exception(BTF_MKERR(BTF_ERR_FILE_OPEN_FAILED, filepath));
        }
        File file(getPathSuffix(filepath));
        file << ifs;
        ifs.close();
        return file;
    }

    String name() const {
        return mName;
    }

    String data() const {
        if (mData == nullptr) {
            return String();
        }
        return *mData;
    }

    size_t size() const {
        if (mData == nullptr) {
            return 0;
        }
        return mData->size();
    }

    bool empty() const {
        if (mData == nullptr) {
            return true;
        }
        return mData->empty();
    }

    void setName(const String &name) {
        mName = name;
    }

    void clear() {
        if (mData == nullptr) {
            return;
        }
        delete mData;
        mData = nullptr;
    }

    void write(OStream &os) const {
        if (mData == nullptr) {
            return;
        }
        os << *mData;
    }

    void write(const String &path, WritePolicy policy = Skip,
               std::ios_base::openmode openmode = std::ios_base::binary) const
    {
        String _path = path.data();
        _path += BTF_PATH_SEPARATOR + mName;
        if (isExistsFile(_path) && policy == Skip) {
            return;
        }

        OFStream os(_path.data(), openmode);
        if (!os.is_open()) {
            throw Exception(BTF_MKERR(BTF_ERR_FILE_OPEN_FAILED, _path));
        }
        write(os);
        os.close();
    }

    File &operator=(const File &other) {
        mName = other.mName;
        clear();
        if (other.mData != nullptr) {
            mData = new String(*other.mData);
        }
        return *this;
    }

    File &operator=(const String &data) {
        clear();
        mData = new String(data);
        return *this;
    }

    template<typename T>
    File &operator=(const std::vector<T> &data) {
        clear();
        mData = new String;
        size_t size = data.size();
        mData->reserve(mData->size() + size);
        for (auto &var : data) {
            mData->push_back(var);
        }
        return *this;
    }

    File &operator<<(const File &other) {
        if (mData == nullptr) {
            mData = new String;
        }
        mData->append(other.data());
        return *this;
    }

    File &operator<<(IStream &is) {
        if (mData == nullptr) {
            mData = new String();
        }
        is.seekg(0, std::ios_base::end);
        size_t size = is.tellg();
        is.seekg(0, std::ios_base::beg);
        mData->reserve(mData->size() + size);
        char buffer[kBufferSize] {};
        while (is.read(buffer, kBufferSize)) {
            mData->append(String(buffer, is.gcount()));
        }
        mData->append(String(buffer, is.gcount()));
        return *this;
    }

    File &operator<<(const String &data) {
        if (mData == nullptr) {
            mData = new String();
        }
        mData->append(data);
        return *this;
    }

    template<typename T>
    File &operator<<(const std::vector<T> &data) {
        if (mData == nullptr) {
            mData = new String;
        }
        size_t size = data.size();
        mData->reserve(mData->size() + size);
        for (auto &var : data) {
            mData->push_back(var);
        }
        return *this;
    }

    const File &operator>>(OStream &os) const {
        write(os);
        return *this;
    }

private:
    String mName;
    String *mData;
};

class Dir
{
public:
    Dir() : mSubFiles(nullptr), mSubDirs(nullptr) {};

    explicit Dir(const String &name) : mName(name), mSubFiles(nullptr), mSubDirs(nullptr) {}

    Dir(const Dir &other) : mSubFiles(nullptr), mSubDirs(nullptr)
    {
        mName = other.mName;
        if (other.mSubFiles != nullptr) {
            mSubFiles = new Vec<File>(*other.mSubFiles);
        }
        if (other.mSubDirs != nullptr) {
            mSubDirs = new Vec<Dir>(*other.mSubDirs);
        }
    }

    Dir(Dir &&other) noexcept {
        mName = other.mName;
        other.mName.clear();
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

    static Dir fromPath(const String &dirpath) {
        if (!isExistsDirectory(dirpath)) {
            throw Exception(BTF_MKERR(BTF_ERR_UNEXISTS_PATH, dirpath));
        }

        Dir root(getPathSuffix(dirpath));

        auto dirs = getAllDirectorys<false>(dirpath);
        for (auto &var : dirs) {
            root << Dir::fromPath(var);
        }

        auto files = getAllFiles<false>(dirpath);
        for (auto &var : files) {
            root << File::fromPath(var);
        }

        return root;
    }

    String name() const {
        return mName;
    }

    size_t size() const {
        size_t size = 0;
        if (mSubFiles != nullptr) {
            for (auto &var : *mSubFiles) {
                size += var.size();
            }
        }
        if (mSubDirs != nullptr) {
            for (auto &var : *mSubDirs) {
                size += var.size();
            }
        }
        return size;
    }

    size_t fileCount(bool isRecursive = true) const {
        size_t cnt = 0;
        if (mSubFiles != nullptr) {
            cnt += mSubFiles->size();
        }
        if (isRecursive && mSubDirs != nullptr) {
            for (auto &var : *mSubDirs) {
                cnt += var.fileCount();
            }
        }
        return cnt;
    }

    size_t dirCount(bool isRecursive = true) const {
        size_t cnt = 0;
        if (mSubDirs != nullptr) {
            cnt += mSubDirs->size();
        }
        if (isRecursive && mSubDirs != nullptr) {
            for (auto &var : *mSubDirs) {
                cnt += var.dirCount();
            }
        }
        return cnt;
    }

    size_t count(bool isRecursive = true) const {
        return fileCount(isRecursive) + dirCount(isRecursive);
    }

    bool empty() const {
        return (mSubFiles == nullptr || mSubFiles->empty()) &&
            (mSubDirs == nullptr || mSubDirs->empty());
    }

    bool hasFile(const String &name, bool isRecursive = false) const {
        if (_hasFile(name) != 0) {
            return true;
        }
        if (isRecursive && mSubDirs != nullptr) {
            for (auto &var : *mSubDirs) {
                if (var.hasFile(name, true)) {
                    return true;
                }
            }
        }
        return false;
    }

    bool hasDir(const String &name, bool isRecursive = false) const {
        if (_hasDir(name) != 0) {
            return true;
        }
        if (isRecursive && mSubDirs != nullptr) {
            for (auto &var : *mSubDirs) {
                if (var.hasDir(name, true)) {
                    return true;
                }
            }
        }
        return false;
    }

    void setName(const String &name) {
        mName = name;
    }

    const Vec<File> &files() const {
        return *mSubFiles;
    }

    const Vec<Dir> &dirs() const {
        return *mSubDirs;
    }

    Vec<File> &files() {
        return *mSubFiles;
    }

    Vec<Dir> &dirs() {
        return *mSubDirs;
    }

    File &file(const String &name) {
        size_t pos = _hasFile(name);
        if (pos == 0) {
            addFile(name);
            return mSubFiles->back();
        }
        return (*mSubFiles)[pos - 1];
    }

    Dir &dir(const String &name) {
        size_t pos = _hasDir(name);
        if (pos == 0) {
            addDir(name);
            return mSubDirs->back();
        }
        return (*mSubDirs)[pos - 1];
    }

    void removeFile(const String &name) {
        size_t pos = _hasFile(name);
        if (pos == 0) {
            return;
        }
        mSubFiles->erase(mSubFiles->begin() + pos - 1);
    }

    void removeDir(const String &name) {
        size_t pos = _hasDir(name);
        if (pos == 0) {
            return;
        }
        mSubDirs->erase(mSubDirs->begin() + pos - 1);
    }

    void remove(const File &file) {
        removeFile(file.name());
    }

    void remove(const Dir &dir) {
        removeDir(dir.name());
    }

    void clearFiles() {
        if (mSubFiles != nullptr) {
            delete mSubFiles;
            mSubFiles = nullptr;
        }
    }

    void clearDirs() {
        if (mSubDirs != nullptr) {
            delete mSubDirs;
            mSubDirs = nullptr;
        }
    }

    void add(File &file, WritePolicy policy = Skip) {
        if (mSubFiles == nullptr) {
            mSubFiles = new Vec<File>();
        }
        size_t pos = _hasFile(file.name());
        if (pos != 0) {
            if (policy == Override) {
                (*mSubFiles)[pos] = std::move(file);
            }
            return;
        }
        mSubFiles->emplace_back(std::move(file));
    }

    void add(Dir &dir, WritePolicy policy = Skip) {
        if (mSubDirs == nullptr) {
            mSubDirs = new Vec<Dir>();
        }
        size_t pos = _hasDir(dir.name());
        if (pos != 0) {
            if (policy == Override) {
                (*mSubDirs)[pos] = std::move(dir);
            }
            return;
        }
        mSubDirs->emplace_back(std::move(dir));
    }

    void add(File &&file) {
        add(file);
    }

    void add(Dir &&dir) {
        add(dir);
    }

    void addFile(const String &name) {
        add(File(name));
    }

    void addDir(const String &name) {
        add(Dir(name));
    }

    void write(const String &path, WritePolicy policy = Skip,
               std::ios_base::openmode openmode = std::ios_base::binary) const
    {
        String root = String(path) + BTF_PATH_SEPARATOR + mName;
        createDirectory(root);

        if (mSubFiles != nullptr) {
            for (auto &var : *mSubFiles) {
                var.write(root, policy, openmode);
            }
        }

        if (mSubDirs != nullptr) {
            for (auto &var : *mSubDirs) {
                var.write(root, policy);
            }
        }
    }

    Dir &operator=(const Dir &other) {
        mName = other.mName;
        clearFiles();
        clearDirs();
        if (other.mSubFiles != nullptr) {
            mSubFiles = new Vec<File>(*other.mSubFiles);
        }
        if (other.mSubDirs != nullptr) {
            mSubDirs = new Vec<Dir>(*other.mSubDirs);
        }
        return *this;
    }

    Dir &operator[](const String &name) {
        return dir(name);
    }

    File &operator()(const String &name) {
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
    size_t _hasFile(const String &name) const {
        if (mSubFiles != nullptr) {
            for (size_t i = 0; i < mSubFiles->size(); ++i) {
                if ((*mSubFiles)[i].name() == name) {
                    return i + 1;
                }
            }
        }
        return 0;
    }

    size_t _hasDir(const String &name) const {
        if (mSubDirs != nullptr) {
            for (size_t i = 0; i < mSubDirs->size(); ++i) {
                if ((*mSubDirs)[i].name() == name) {
                    return i + 1;
                }
            }
        }
        return 0;
    }

private:
    String mName;
    Vec<File> *mSubFiles;
    Vec<Dir> *mSubDirs;
};

}

#endif // !BETTERFILES_H
