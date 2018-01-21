// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_TIME_INCLUDED
#define CMAJOR_RT_TIME_INCLUDED

#include <cmajor/rt/RtApi.hpp>
#include <stdint.h>

extern "C" RT_API int64_t RtNow();
extern "C" RT_API void RtSleep(int64_t nanoSeconds);

#endif // CMAJOR_RT_TIME_INCLUDED
