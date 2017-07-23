// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Io.hpp>
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
    void WriteFile(int32_t fileHandle, const uint8_t* buffer, int32_t count);
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

FileTable::FileTable() : stdoutInUtf16Mode(false), stderrInUtf16Mode(false)
{
    files.resize(maxNoLockFileHandles);
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

void FileTable::WriteFile(int32_t fileHandle, const uint8_t* buffer, int32_t count)
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

extern "C" RT_API void RtWrite(int32_t fileHandle, const uint8_t* buffer, int32_t count)
{
    try
    {
        cmajor::rt::FileTable::Instance().WriteFile(fileHandle, buffer, count);
    }
    catch (const cmajor::rt::FileSystemError& ex)
    {
        int x = 0;
        // todo
    }
}

