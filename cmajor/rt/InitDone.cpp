// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/InitDone.hpp>
#include <cmajor/rt/Error.hpp>
#include <cmajor/parsing/InitDone.hpp>
#include <cmajor/util/InitDone.hpp>
#include <cmajor/rt/Io.hpp>
#include <cmajor/rt/CallStack.hpp>
#include <cmajor/rt/Classes.hpp>
#include <cmajor/rt/Statics.hpp>
#include <cmajor/rt/String.hpp>
#include <cmajor/rt/Mutex.hpp>
#include <cmajor/rt/ConditionVariable.hpp>
#include <cmajor/rt/Thread.hpp>
#include <cmajor/rt/Memory.hpp>
#ifdef _WIN32
#include <cmajor/rt/CommandLine.hpp>
#endif
#include <cmajor/rt/Socket.hpp>
#include <cmajor/rt/Environment.hpp>
#include <csignal>

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
    InitMemory();
    InitConditionVariable();
    InitThread();
    InitSocket();
    InitEnvironment();
    InitStatics();
    InitClasses();
#ifdef _WIN32
    InitCommandLine();
#endif
}

void Done()
{
#ifdef _WIN32
    DoneCommandLine();
#endif
    DoneClasses();
    DoneStatics();
    DoneEnvironment();
    DoneSocket();
    DoneThread();
    DoneConditionVariable();
    DoneMemory();
    DoneString();
    DoneError();
    DoneIo();
    cmajor::util::Done();
    cmajor::parsing::Done();
    DoneMutex();
}

} } // namespace cmajor::rt
