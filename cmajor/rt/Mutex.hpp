// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_MUTEX_INCLUDED
#define CMAJOR_RT_MUTEX_INCLUDED
#include <cmajor/rt/RtApi.hpp>
#include <stdint.h>

extern "C" RT_API int32_t RtAllocateMutex();
extern "C" RT_API void RtFreeMutex(int32_t mutexId);
extern "C" RT_API void RtLockMutex(int32_t mutexId);
extern "C" RT_API void RtUnlockMutex(int32_t mutexId);

namespace cmajor { namespace rt { 

void InitMutex();
void DoneMutex();

} } // namespace cmajor::rt

#endif // CMAJOR_RT_MUTEX_INCLUDED
