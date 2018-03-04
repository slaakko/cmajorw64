// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
#define CMAJOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
#include <string>
#include <vector>
#include <stdint.h>

namespace cmajor { namespace symbols {

enum class GlobalFlags : uint16_t
{
    none = 0,
    verbose = 1 << 0,
    quiet = 1 << 1,
    release = 1 << 2,
    clean = 1 << 3,
    debugParsing = 1 << 4,
    emitLlvm = 1 << 5,
    emitOptLlvm = 1 << 6,
    linkWithDebugRuntime = 1 << 7,
    linkUsingMsLink = 1 << 8,
    ide = 1 << 9,
    strictNothrow = 1 << 10,
    time = 1 << 11,
    info = 1 << 12,
    unitTest = 1 << 13,
    profile = 1 << 14
};

void SetGlobalFlag(GlobalFlags flag);
void ResetGlobalFlag(GlobalFlags flag);
bool GetGlobalFlag(GlobalFlags flag);

std::string GetConfig();
int GetOptimizationLevel();
void SetOptimizationLevel(int optimizationLevel_);

void SetCurrentProjectName(const std::u32string& currentProjectName_);
std::u32string GetCurrentProjectName();
void SetCurrentTooName(const std::u32string& currentToolName_);
std::u32string GetCurrentToolName();

void DefineCommandLineConditionalSymbol(const std::u32string& symbol);

void ClearDefines();
void DefineSymbol(const std::u32string& symbol);
bool IsSymbolDefined(const std::u32string& symbol);

bool BeginUnitTest();
bool InUnitTest();
void ResetUnitTestAssertionNumber();
int32_t GetNextUnitTestAssertionNumber();
int32_t GetNumUnitTestAssertions();
void EndUnitTest(bool prevUnitTest);
void SetAssertionLineNumberVector(std::vector<int32_t>* assertionLineNumberVector_);
void AddAssertionLineNumber(int32_t lineNumber);

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_GLOBAL_FLAGS_INCLUDED
