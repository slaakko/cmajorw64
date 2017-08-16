// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Error.hpp>
#include <cmajor/rt/CallStack.hpp>
#include <cmajor/rt/Io.hpp>
#include <sstream>
#include <stdlib.h>

extern "C" RT_API void RtFailAssertion(const char* assertion, const char* function, const char* sourceFilePath, int lineNumber)
{
    std::stringstream s;
    s << "assertion '" << assertion << "' failed in function '" << function << "' at " << sourceFilePath << ":" << lineNumber << "\n";
    std::string str = s.str();
    RtWrite(stdErrFileHandle, reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
    RtPrintCallStack(stdErrFileHandle);
    exit(exitCodeAssertionFailed);
}
