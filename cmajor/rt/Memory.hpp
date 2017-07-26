// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_MEMORY_INCLUDED
#define CMAJOR_RT_MEMORY_INCLUDED
#include <cmajor/rt/RtApi.hpp>
#include <stdint.h>

namespace cmajor { namespace rt {

extern "C" RT_API void* RtMemAlloc(uint64_t size);
extern "C" RT_API void RtMemFree(void* ptr);

} }  // namespace cmajor::rt

#endif // CMAJOR_RT_MEMORY_INCLUDED
