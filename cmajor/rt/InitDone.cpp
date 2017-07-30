// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/InitDone.hpp>
#include <cmajor/util/InitDone.hpp>
#include <cmajor/rt/Io.hpp>
#include <cmajor/rt/Classes.hpp>

extern "C" RT_API void RtInit()
{
    cmajor::rt::Init();
}

extern "C" RT_API void RtDone()
{
    cmajor::rt::Done();
}

namespace cmajor { namespace rt {

void Init()
{
    InitIo();
    InitClasses();
    cmajor::util::Init();
}

void Done()
{
    cmajor::util::Done();
    DoneClasses();
    DoneIo();
}

} } // namespace cmajor::rt
