// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_IO_INCLUDED
#define CMAJOR_RT_IO_INCLUDED
#include <cmajor/rt/RtApi.hpp>
#include <stdexcept>
#include <stdint.h>

const int stdOutFileHandle = 1;
const int stdErrFileHandle = 2;

enum class OpenMode : uint8_t
{
    none = 0,
    read = 1 << 0, 
    write = 1 << 1,
    append = 1 << 2,
    binary = 1 << 3
};

inline OpenMode operator&(OpenMode left, OpenMode right)
{
    return OpenMode(uint8_t(left) & uint8_t(right));
}

inline OpenMode operator|(OpenMode left, OpenMode right)
{
    return OpenMode(uint8_t(left) | uint8_t(right));
}

extern "C" RT_API int32_t RtOpen(const char* filePath, OpenMode openMode);
extern "C" RT_API int32_t RtClose(int32_t fileHandle);
extern "C" RT_API int32_t RtWrite(int32_t fileHandle, const uint8_t* buffer, int64_t count);
extern "C" RT_API int32_t RtRead(int32_t fileHandle, uint8_t* buffer, int64_t bufferSize);

namespace cmajor { namespace rt {

class FileSystemError : public std::runtime_error
{
public:
    FileSystemError(const std::string& message_);
};

void InitIo();
void DoneIo();

} }  // namespace cmajor::rt

#endif // CMAJOR_RT_IO_INCLUDED
