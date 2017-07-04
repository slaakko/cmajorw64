// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/InitDone.hpp>
#include <cmajor/ast/Node.hpp>

namespace cmajor { namespace ast {

void Init()
{
    NodeInit();
}

void Done()
{
    NodeDone();
}

} } // namespace cmajor::ast