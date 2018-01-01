// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/Clone.hpp>

namespace cmajor { namespace ast {

CloneContext::CloneContext() : instantiateFunctionNode(false), instantiateClassNode(false)
{
}

} } // namespace cmajor::ast
