// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_UTIL_LOG_INCLUDED
#define CMAJOR_UTIL_LOG_INCLUDED
#include <string>

namespace cmajor { namespace util {

void LogMessage(int logStreamId, const std::string& message);

} } // namespace cmajor::util

#endif // CMAJOR_UTIL_LOG_INCLUDED
