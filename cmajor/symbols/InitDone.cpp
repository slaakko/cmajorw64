// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/InitDone.hpp>
#include <cmajor/symbols/Warning.hpp>

namespace cmajor { namespace symbols {

void Init()
{
    InitWarning();
}

void Done()
{
    DoneWarning();
}

} } // namespace cmajor::symbols
