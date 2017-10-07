// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/InitDone.hpp>
#include <cmajor/rt/Error.hpp>
#include <cmajor/util/InitDone.hpp>
#include <cmajor/rt/Io.hpp>
#include <cmajor/rt/Classes.hpp>
#include <cmajor/rt/Statics.hpp>
#include <cmajor/rt/String.hpp>
#include <cmajor/rt/Mutex.hpp>

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
    cmajor::util::Init();
    InitIo();
    InitError();
    InitString();
    InitStatics();
    InitClasses();
}

void Done()
{
    DoneClasses();
    DoneStatics();
    DoneString();
    DoneError();
    DoneIo();
    cmajor::util::Done();
    DoneMutex();
}

} } // namespace cmajor::rt
