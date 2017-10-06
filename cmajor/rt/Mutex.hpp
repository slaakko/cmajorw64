// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_MUTEX_INCLUDED
#define CMAJOR_RT_MUTEX_INCLUDED
#include <stdint.h>

extern "C" int32_t RtAllocateMutex();
extern "C" void RtFreeMutex(int32_t mutexId);
extern "C" void RtLockMutex(int32_t mutexId);
extern "C" void RtUnlockMutex(int32_t mutexId);

namespace cmajor { namespace rt { 

void InitMutex();
void DoneMutex();

} } // namespace cmajor::rt

#endif // CMAJOR_RT_MUTEX_INCLUDED
