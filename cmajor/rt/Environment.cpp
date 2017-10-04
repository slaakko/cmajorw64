// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Environment.hpp>
#include <cmajor/rt/Error.hpp>
#include <cmajor/rt/String.hpp>
#include <memory>
#include <mutex>
#include <cstdlib>
#include <cerrno>
#include <direct.h>

extern "C" const char* RtGetEnvironmentVariable(const char* environmentVariableName)
{
    const char* envVar = std::getenv(environmentVariableName);
    if (envVar)
    {
        return envVar;
    }
    return "";
}

std::mutex mtx;

extern "C" int32_t RtGetCurrentWorkingDirectoryHandle()
{
    std::lock_guard<std::mutex> lock(mtx);
    std::unique_ptr<char[]> buffer(new char[8192]);
    if (getcwd(buffer.get(), 8192))
    {
        return cmajor::rt::InstallString(buffer.get());
    }
    else
    {
        return cmajor::rt::InstallError(std::string("could not get current working directory: ") + std::strerror(errno));
    }
}

extern "C" const char* RtGetCurrentWorkingDirectory(int32_t currentWorkingDirectoryHandle)
{
    return cmajor::rt::GetString(currentWorkingDirectoryHandle);
}

extern "C" void RtFreeCurrentWorkingDirectoryHandle(int32_t currentWorkingDirectoryHandle)
{
    cmajor::rt::DisposeString(currentWorkingDirectoryHandle);
}