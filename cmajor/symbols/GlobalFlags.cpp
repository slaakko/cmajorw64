// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/GlobalFlags.hpp>
#include <set>

namespace cmajor { namespace symbols {

GlobalFlags globalFlags;
int optimizationLevel = -1;

inline GlobalFlags operator|(GlobalFlags flags, GlobalFlags flag)
{
    return GlobalFlags(uint16_t(flags) | uint16_t(flag));
}

inline GlobalFlags operator&(GlobalFlags flags, GlobalFlags flag)
{
    return GlobalFlags(uint16_t(flags) & uint16_t(flag));
}

inline GlobalFlags operator~(GlobalFlags flags)
{
    return GlobalFlags(~uint16_t(flags));
}

void SetGlobalFlag(GlobalFlags flag)
{
    globalFlags = globalFlags | flag;
}

void ResetGlobalFlag(GlobalFlags flag)
{
    globalFlags = globalFlags & ~flag;
}

bool GetGlobalFlag(GlobalFlags flag)
{
    return (globalFlags & flag) != GlobalFlags::none;
}

std::string GetConfig()
{
    std::string config = "debug";
    if (GetGlobalFlag(GlobalFlags::release))
    {
        config = "release";
    }
    return config;
}

int GetOptimizationLevel()
{
    if (optimizationLevel == -1)
    {
        if (GetGlobalFlag(GlobalFlags::release))
        {
            return 3;
        }
        else
        {
            return 0;
        }
    }
    else
    {
        return optimizationLevel;
    }
}

void SetOptimizationLevel(int optimizationLevel_)
{
    optimizationLevel = optimizationLevel_;
}

std::u32string currentProjectName;

void SetCurrentProjectName(const std::u32string& currentProjectName_)
{
    currentProjectName = currentProjectName_;
}

std::u32string GetCurrentProjectName()
{
    return currentProjectName;
}

std::u32string currentToolName;

void SetCurrentTooName(const std::u32string& currentToolName_)
{
    currentToolName = currentToolName_;
}

std::u32string GetCurrentToolName()
{
    return currentToolName;
}

std::set<std::u32string> defines;

void ClearDefines()
{
    defines.clear();
}

void DefineSymbol(const std::u32string& symbol)
{
    defines.insert(symbol);
}

bool IsSymbolDefined(const std::u32string& symbol)
{
    auto it = defines.find(symbol);
    if (it != defines.cend())
    {
        return true;
    }
    else
    {
        return false;
    }
}

} } // namespace cmajor::symbols
