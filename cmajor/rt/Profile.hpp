// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_PROFILE_INCLUDED
#define CMAJOR_RT_PROFILE_INCLUDED
#include <cmajor/rt/RtApi.hpp>
#include <stdint.h>

extern "C" RT_API void RtStartProfiling();
extern "C" RT_API void RtEndProfiling();
extern "C" RT_API void RtProfileStartFunction(uint32_t functionId);
extern "C" RT_API void RtProfileEndFunction(uint32_t functionId);

#endif // CMAJOR_RT_PROFILE_INCLUDED
