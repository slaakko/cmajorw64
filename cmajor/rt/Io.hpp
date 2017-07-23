// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_IO_INCLUDED
#define CMAJOR_RT_IO_INCLUDED
#include <cmajor/rt/RtApi.hpp>
#include <stdexcept>
#include <stdint.h>

extern "C" RT_API void RtWrite(int32_t fileHandle, const uint8_t* buffer, int32_t count);

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
