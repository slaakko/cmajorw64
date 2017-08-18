// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/GlobalFlags.hpp>

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

} } // namespace cmajor::symbols
