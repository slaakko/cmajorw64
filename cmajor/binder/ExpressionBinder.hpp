// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_EXPRESSION_BINDER_INCLUDED
#define CMAJOR_BINDER_EXPRESSION_BINDER_INCLUDED
#include <cmajor/ast/Node.hpp>
#include <cmajor/symbols/Scope.hpp>

namespace cmajor { namespace binder {

using cmajor::ast::Node;
using cmajor::symbols::ContainerScope;

class BoundCompileUnit;
class BoundFunction;
class BoundExpression;

std::unique_ptr<BoundExpression> BindExpression(Node* node, BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope);

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_EXPRESSION_BINDER_INCLUDED
