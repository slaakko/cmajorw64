// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Io.hpp>
#include <cmajor/rt/Error.hpp>
#include <cmajor/util/Error.hpp>
#include <cmajor/util/Unicode.hpp>
#include <atomic>
#include <mutex>
#include <io.h>
#include <fcntl.h>

namespace cmajor { namespace rt {

using namespace cmajor::unicode;

class FileTable
{
public:
    ~FileTable();
    static void Init();
    static void Done();
    static FileTable& Instance() { Assert(instance, "file table not initialized"); return *instance; }
    int32_t OpenFile(const char* filePath, OpenMode openMode);
    void CloseFile(int32_t fileHandle);
    void WriteFile(int32_t fileHandle, const uint8_t* buffer, int64_t count);
    int32_t ReadFile(int32_t fileHandle, uint8_t* buffer, int64_t bufferSize);
private:
    static std::unique_ptr<FileTable> instance;
    const int32_t maxNoLockFileHandles = 256;
    std::vector<FILE*> files;
    std::vector<std::string> filePaths;
    std::unordered_map<int32_t, FILE*> fileMap;
    std::unordered_map<int32_t, std::string> filePathMap;
    std::atomic<int32_t> nextFileHandle;
    std::mutex mtx;
    bool stdoutInUtf16Mode;
    bool stderrInUtf16Mode;
    FileTable();
};

std::unique_ptr<FileTable> FileTable::instance;

void FileTable::Init()
{
    instance.reset(new FileTable());
}

void FileTable::Done()
{
    instance.reset();
}

FileTable::FileTable() : stdoutInUtf16Mode(false), stderrInUtf16Mode(false), nextFileHandle(3)
{
    files.resize(maxNoLockFileHandles);
    filePaths.resize(maxNoLockFileHandles);
    files[0] = stdin;
    files[1] = stdout;
    files[2] = stderr;
    std::fflush(stdout);
    std::fflush(stderr);
    if (_isatty(1))
    {
        _setmode(1, _O_U16TEXT);
        stdoutInUtf16Mode = true;
    }
    if (_isatty(2))
    {
        _setmode(2, _O_U16TEXT);
        stderrInUtf16Mode = true;
    }
}

FileTable::~FileTable()
{
    std::fflush(stdout);
    std::fflush(stderr);
    if (stdoutInUtf16Mode)
    {
        _setmode(1, _O_TEXT);
    }
    if (stderrInUtf16Mode)
    {
        _setmode(2, _O_TEXT);
    }
}

int32_t FileTable::OpenFile(const char* filePath, OpenMode openMode)
{
    const char* mode = nullptr;
    if ((openMode & OpenMode::read) != OpenMode::none)
    {
        if ((openMode & (OpenMode::write | OpenMode::append)) != OpenMode::none)
        {
            throw FileSystemError("open mode not supported");
        }
        if ((openMode & OpenMode::binary) != OpenMode::none)
        {
            mode = "rb";
        }
        else
        {
            mode = "r";
        }
    }
    else if ((openMode & OpenMode::write) != OpenMode::none)
    {
        if ((openMode & (OpenMode::read | OpenMode::append)) != OpenMode::none)
        {
            throw FileSystemError("open mode not supported");
        }
        if ((openMode & OpenMode::binary) != OpenMode::none)
        {
            mode = "wb";
        }
        else
        {
            mode = "w";
        }
    }
    else if ((openMode & OpenMode::append) != OpenMode::none)
    {
        if ((openMode & (OpenMode::read | OpenMode::write)) != OpenMode::none)
        {
            throw FileSystemError("open mode not supported");
        }
        if ((openMode & OpenMode::binary) != OpenMode::none)
        {
            mode = "ab";
        }
        else
        {
            mode = "a";
        }
    }
    else
    {
        throw FileSystemError("open mode supported");
    }
    FILE* file = std::fopen(filePath, mode);
    if (!file)
    {
        throw FileSystemError("could not open file '" + std::string(filePath) + "': " + strerror(errno));
    }
    int32_t fileHandle = nextFileHandle++;
    if (fileHandle < maxNoLockFileHandles)
    {
        files[fileHandle] = file;
        filePaths[fileHandle] = filePath;
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        fileMap[fileHandle] = file;
        filePathMap[fileHandle] = filePath;
    }
    return fileHandle;
}

void FileTable::CloseFile(int32_t fileHandle)
{
    FILE* file = nullptr;
    std::string filePath;
    if (fileHandle < 0)
    {
        throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
    }
    else if (fileHandle < maxNoLockFileHandles)
    {
        file = files[fileHandle];
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = fileMap.find(fileHandle);
        if (it != fileMap.cend())
        {
            file = it->second;
        }
        else
        {
            throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
        }
    }
    if (!file)
    {
        throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
    }
    int result = fclose(file);
    if (result != 0)
    {
        std::string filePath;
        if (fileHandle < maxNoLockFileHandles)
        {
            filePath = filePaths[fileHandle];
        }
        else
        {
            filePath = filePathMap[fileHandle];
        }
        throw FileSystemError("could not close file '" + filePath + "': " + strerror(errno));
    }
}

void FileTable::WriteFile(int32_t fileHandle, const uint8_t* buffer, int64_t count)
{
    FILE* file = nullptr;
    if (fileHandle < 0)
    {
        throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
    }
    else if (fileHandle < maxNoLockFileHandles)
    {
        file = files[fileHandle];
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = fileMap.find(fileHandle);
        if (it != fileMap.cend())
        {
            file = it->second;
        }
        else
        {
            throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
        }
    }
    if (!file)
    {
        throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
    }
    int32_t result = 0;
    if (fileHandle == 1 && stdoutInUtf16Mode || fileHandle == 2 && stderrInUtf16Mode)
    {
        std::u16string s(ToUtf16(std::string(buffer, buffer + count)));
        result = int32_t(std::fwrite(s.c_str(), sizeof(char16_t), s.length(), file));
        count = s.length();
    }
    else
    {
        result = int32_t(std::fwrite(buffer, 1, count, file));
    }
    if (result != count)
    {
        std::string filePath;
        if (fileHandle < maxNoLockFileHandles)
        {
            filePath = filePaths[fileHandle];
        }
        else
        {
            filePath = filePathMap[fileHandle];
        }
        throw FileSystemError("could not write to '" + filePath + "': " + strerror(errno));
    }
}

int32_t FileTable::ReadFile(int32_t fileHandle, uint8_t* buffer, int64_t bufferSize)
{
    FILE* file = nullptr;
    if (fileHandle < 0)
    {
        throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
    }
    else if (fileHandle < maxNoLockFileHandles)
    {
        file = files[fileHandle];
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = fileMap.find(fileHandle);
        if (it != fileMap.cend())
        {
            file = it->second;
        }
        else
        {
            throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
        }
    }
    if (!file)
    {
        throw FileSystemError("invalid file handle " + std::to_string(fileHandle));
    }
    int32_t result = 0;
    result = int32_t(std::fread(buffer, 1, bufferSize, file));
    if (result < bufferSize)
    {
        if (std::ferror(file) != 0)
        {
            std::string filePath;
            if (fileHandle < maxNoLockFileHandles)
            {
                filePath = filePaths[fileHandle];
            }
            else
            {
                filePath = filePathMap[fileHandle];
            }
            throw FileSystemError("could not read from '" + filePath + "': " + strerror(errno));
        }
    }
    return result;
}

FileSystemError::FileSystemError(const std::string& message_) : std::runtime_error(message_)
{
}

void InitIo()
{
    FileTable::Init();
}

void DoneIo()
{
    FileTable::Done();
}

} }  // namespace cmajor::rt

extern "C" RT_API int32_t RtOpen(const char* filePath, OpenMode openMode)
{
    try
    {
        return cmajor::rt::FileTable::Instance().OpenFile(filePath, openMode);
    }
    catch (const cmajor::rt::FileSystemError& ex)
    {
        return cmajor::rt::InstallError(ex.what());
    }
}

extern "C" RT_API int32_t RtClose(int32_t fileHandle)
{
    try
    {
        cmajor::rt::FileTable::Instance().CloseFile(fileHandle);
        return 0;
    }
    catch (const cmajor::rt::FileSystemError& ex)
    {
        return cmajor::rt::InstallError(ex.what());
    }
}

extern "C" RT_API int32_t RtWrite(int32_t fileHandle, const uint8_t* buffer, int64_t count)
{
    try
    {
        cmajor::rt::FileTable::Instance().WriteFile(fileHandle, buffer, count);
        return 0;
    }
    catch (const cmajor::rt::FileSystemError& ex)
    {
        return cmajor::rt::InstallError(ex.what());
    }
}

extern "C" RT_API int32_t RtRead(int32_t fileHandle, uint8_t* buffer, int64_t bufferSize)
{
    try
    {
        return cmajor::rt::FileTable::Instance().ReadFile(fileHandle, buffer, bufferSize);
    }
    catch (const cmajor::rt::FileSystemError& ex)
    {
        return cmajor::rt::InstallError(ex.what());
    }
}
