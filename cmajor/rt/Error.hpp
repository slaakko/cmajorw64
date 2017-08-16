// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_ASSERT_INCLUDED
#define CMAJOR_RT_ASSERT_INCLUDED
#include <cmajor/rt/RtApi.hpp>

const int exitCodeInternalError = 255;
const int exitCodeAssertionFailed = 254;
const int exitCodeOutOfMemory = 253;

extern "C" RT_API void RtFailAssertion(const char* assertion, const char* function, const char* sourceFilePath, int lineNumber);

#endif // CMAJOR_RT_ASSERT_INCLUDED
