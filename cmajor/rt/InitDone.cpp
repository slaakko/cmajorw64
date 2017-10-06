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
    cmajor::util::Init();
    InitIo();
    InitStatics();
    InitClasses();
    InitError();
    InitString();
    InitMutex();
}

void Done()
{
    DoneMutex();
    DoneString();
    DoneError();
    DoneClasses();
    DoneStatics();
    DoneIo();
    cmajor::util::Done();
}

} } // namespace cmajor::rt
