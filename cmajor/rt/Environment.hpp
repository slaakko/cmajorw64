// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_ENVIRONMENT_INCLUDED
#define CMAJOR_RT_ENVIRONMENT_INCLUDED
#include <stdint.h>

extern "C" const char* RtGetEnvironmentVariable(const char* environmentVariableName);
extern "C" int32_t RtGetCurrentWorkingDirectoryHandle();
extern "C" const char* RtGetCurrentWorkingDirectory(int32_t currentWorkingDirectoryHandle);
extern "C" void RtFreeCurrentWorkingDirectoryHandle(int32_t currentWorkingDirectoryHandle);

#endif // CMAJOR_RT_ENVIRONMENT_INCLUDED
