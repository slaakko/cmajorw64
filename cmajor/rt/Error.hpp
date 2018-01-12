// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_ASSERT_INCLUDED
#define CMAJOR_RT_ASSERT_INCLUDED
#include <cmajor/rt/RtApi.hpp>
#include <string>
#include <stdint.h>

const int exitCodeAccessViolation = 255;
const int exitCodeInternalError = 254;
const int exitCodeAssertionFailed = 253;
const int exitCodeOutOfMemory = 252;
const int exitCodeExceptionEscapedFromThread = 251;

extern "C" RT_API void RtFailAssertion(const char* assertion, const char* function, const char* sourceFilePath, int lineNumber);
extern "C" RT_API const char* RtGetError(int32_t errorId);
extern "C" RT_API void RtDisposeError(int32_t errorId);
extern "C" RT_API void RtThrowException(void* exception, uint32_t exceptionTypeId);
extern "C" RT_API void RtCaptureException(void*& exception, uint64_t& exceptionClassId);
extern "C" RT_API void RtThrowCapturedException(void* exception, uint64_t exceptionClassId);
extern "C" RT_API bool RtHandleException(uint32_t exceptionTypeId);
extern "C" RT_API void* RtGetException();

namespace cmajor { namespace rt {

class Exception
{
};

int32_t InstallError(const std::string& errorMessage);
const char* GetError(int32_t errorId);
void DisposeError(int32_t errorId);
void InitError();
void DoneError();

} } // namespace cmajor::rt

#endif // CMAJOR_RT_ASSERT_INCLUDED
