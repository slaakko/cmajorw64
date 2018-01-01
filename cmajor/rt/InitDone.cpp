// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/InitDone.hpp>
#include <cmajor/rt/Error.hpp>
#include <cmajor/parsing/InitDone.hpp>
#include <cmajor/util/InitDone.hpp>
#include <cmajor/rt/Io.hpp>
#include <cmajor/rt/Classes.hpp>
#include <cmajor/rt/Statics.hpp>
#include <cmajor/rt/String.hpp>
#include <cmajor/rt/Mutex.hpp>
#include <cmajor/rt/Threading.hpp>
#include <cmajor/rt/CommandLine.hpp>

extern "C" RT_API void RtInit()
{
    cmajor::rt::Init();
}

extern "C" RT_API void RtDone()
{
    cmajor::rt::Done();
}

extern "C" RT_API void RtExit(int32_t exitCode)
{
    exit(exitCode);
}

namespace cmajor { namespace rt {

void Init()
{
    InitMutex();
    cmajor::parsing::Init();
    cmajor::util::Init();
    InitIo();
    InitError();
    InitString();
    InitStatics();
    InitClasses();
    InitThreading();
    InitCommandLine();
}

void Done()
{
    DoneCommandLine();
    DoneThreading();
    DoneClasses();
    DoneStatics();
    DoneString();
    DoneError();
    DoneIo();
    cmajor::util::Done();
    cmajor::parsing::Done();
    DoneMutex();
}

} } // namespace cmajor::rt
