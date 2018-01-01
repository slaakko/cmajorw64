// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Memory.hpp>
#include <cmajor/rt/Error.hpp>
#include <cmajor/rt/Io.hpp>
#include <cmajor/rt/CallStack.hpp>
#include <sstream>
#include <malloc.h>

namespace cmajor { namespace rt {

extern "C" RT_API void* RtMemAlloc(int64_t size)
{
    void* ptr = malloc(size);
    if (!ptr)
    {
        std::stringstream s;
        s << "program out of memory\n";
        std::string str = s.str();
        RtWrite(stdErrFileHandle, reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
        RtPrintCallStack(stdErrFileHandle);
        exit(exitCodeOutOfMemory);
    }
    return ptr;
}

extern "C" RT_API void RtMemFree(void* ptr)
{
    free(ptr);
}

} }  // namespace cmajor::rt
