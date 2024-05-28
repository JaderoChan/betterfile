#ifndef BETTERFILES_HPP
#define BETTERFILES_HPP

#include <string>
#include <string_view>
#include <fstream>
#include <iostream>
#include <vector>
#include <filesystem>

#define BF_ERROR_UNDEFINED_ "The undefined error be occured."
#define BF_ERROR_FILE_OPEN_FAILED "The file open failed."
#define BF_ERROR_INVALID_PATH "The specify path is invalid."
#define BF_ERROR_INVALID_PARM "The specify parameter is invalid: "
#define BF_ERROR_HINT  "The error be occurred in the function \"" __FUNCTION__ "\". "

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

inline void pathNormalization(std::string &path) {
    path = std::filesystem::path(path).lexically_normal().string();
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
    explicit File(std::string_view name) : mName(name), mData(nullptr) {}
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

    static File fromPath(std::string_view filePath) {
        if (!isExistsFile(filePath))
            throw BF_ERROR_INVALID_PATH;
        std::ifstream ifs(filePath.data(), std::ios_base::binary);
        if (!ifs.is_open())
            throw BF_ERROR_FILE_OPEN_FAILED;
        File file(Bf::getPathSuffix(filePath));
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

    void setName(std::string_view name) {
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
    void write(std::string_view path, WritePolicy policy = Skip,
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
    explicit Dir(std::string_view name) : mName(name), mSubFiles(nullptr), mSubDirs(nullptr) {}
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

    static Dir fromPath(std::string_view dirPath) {
        if (!isExistsDirectory(dirPath))
            throw BF_ERROR_INVALID_PATH;
        Dir root(Bf::getPathSuffix(dirPath));

        auto dirs = Bf::getAllDirectorys(dirPath, false);
        for (auto &var : dirs)
            root << Dir::fromPath(var);

        auto files = Bf::getAllFiles(dirPath, false);
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
    bool hasFile(std::string_view name) const {
        if (mSubFiles == nullptr)
            return false;
        for (auto &var : *mSubFiles) {
            if (var.name() == name)
                return true;
        }
        return false;
    }
    bool hasDir(std::string_view name) const {
        if (mSubDirs == nullptr)
            return false;
        for (auto &var : *mSubDirs) {
            if (var.name() == name)
                return true;
        }
        return false;
    }

    void setName(std::string_view name) {
        mName = name;
    }

    File &file(std::string_view name) {
        if (mSubFiles == nullptr)
            mSubFiles = new std::vector<File>();
        for (auto &var : *mSubFiles) {
            if (var.name() == name)
                return var;
        }
        mSubFiles->push_back(File(name));
        return mSubFiles->back();
    }
    Dir &dir(std::string_view name) {
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
    void addFile(std::string_view name) {
        add(File(name));
    }
    void addDir(std::string_view name) {
        add(Dir(name));
    }

    void write(std::string_view path, WritePolicy policy = Skip,
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
    Dir &operator[](std::string_view name) {
        return dir(name);
    }
    File &operator()(std::string_view name) {
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
