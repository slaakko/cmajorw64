// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
#define CMAJOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
#include <string>
#include <stdint.h>

namespace cmajor { namespace symbols {

enum class GlobalFlags : uint8_t
{
    none = 0,
    verbose = 1 << 0,
    quiet = 1 << 1,
    release = 1 << 2,
    clean = 1 << 3,
    debugParsing = 1 << 4,
    emitLlvm = 1 << 5,
    emitOptLlvm = 1 << 6,
    linkWithDebugRuntime = 1 << 7
};

void SetGlobalFlag(GlobalFlags flag);
void ResetGlobalFlag(GlobalFlags flag);
bool GetGlobalFlag(GlobalFlags flag);

std::string GetConfig();
int GetOptimizationLevel();

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
