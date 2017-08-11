// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_STATICS_INCLUDED
#define CMAJOR_RT_STATICS_INCLUDED
#include <cmajor/rt/RtApi.hpp>
#include <vector>
#include <stdint.h>

extern "C" RT_API void RtBeginStaticInitCriticalSection(uint32_t staticClassId);
extern "C" RT_API void RtEndStaticInitCriticalSection(uint32_t staticClassId);
extern "C" RT_API void RtEnqueueDestruction(void* destructor, void* arg);

namespace cmajor { namespace rt {

void AllocateMutexes(const std::vector<uint32_t>& staticClassIds);

void InitStatics();
void DoneStatics();

} }  // namespace cmajor::rt

#endif // CMAJOR_RT_STATICS_INCLUDED
