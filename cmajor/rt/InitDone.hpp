// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_INIT_DONE_INCLUDED
#define CMAJOR_RT_INIT_DONE_INCLUDED
#include <cmajor/rt/RtApi.hpp>
#include <stdint.h>

extern "C" RT_API void RtInit();
extern "C" RT_API void RtDone();
extern "C" RT_API void RtExit(int32_t exitCode);

namespace cmajor { namespace rt {

void Init();
void Done();

} } // namespace cmajor::rt

#endif // CMAJOR_RT_INIT_DONE_INCLUDED
