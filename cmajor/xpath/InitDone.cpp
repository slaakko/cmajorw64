// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/xpath/InitDone.hpp>
#include <cmajor/xpath/XPathFunction.hpp>

namespace cmajor { namespace xpath {

void Init()
{
    InitFunction();
}

void Done()
{
    DoneFunction();
}

} } // namespace cmajor::xpath
