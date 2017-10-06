// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_ENVIRONMENT_INCLUDED
#define CMAJOR_RT_ENVIRONMENT_INCLUDED
#include <cmajor/rt/RtApi.hpp>
#include <stdint.h>

extern "C" RT_API const char* RtGetEnvironmentVariable(const char* environmentVariableName);
extern "C" RT_API int32_t RtGetCurrentWorkingDirectoryHandle();
extern "C" RT_API const char* RtGetCurrentWorkingDirectory(int32_t currentWorkingDirectoryHandle);
extern "C" RT_API void RtFreeCurrentWorkingDirectoryHandle(int32_t currentWorkingDirectoryHandle);

#endif // CMAJOR_RT_ENVIRONMENT_INCLUDED
