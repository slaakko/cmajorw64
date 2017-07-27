// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Memory.hpp>
#include <malloc.h>

namespace cmajor { namespace rt {

extern "C" RT_API void* RtMemAlloc(int64_t size)
{
    return malloc(size);
}

extern "C" RT_API void RtMemFree(void* ptr)
{
    free(ptr);
}

} }  // namespace cmajor::rt
