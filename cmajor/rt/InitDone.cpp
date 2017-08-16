// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/InitDone.hpp>
#include <cmajor/util/InitDone.hpp>
#include <cmajor/rt/Io.hpp>
#include <cmajor/rt/Classes.hpp>
#include <cmajor/rt/Statics.hpp>

extern "C" RT_API void RtInit()
{
    cmajor::rt::Init();
}

extern "C" RT_API void RtDone()
{
    cmajor::rt::Done();
}

extern "C" RT_API void RtExit(int exitCode)
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
}

void Done()
{
    DoneClasses();
    DoneStatics();
    DoneIo();
    cmajor::util::Done();
}

} } // namespace cmajor::rt
