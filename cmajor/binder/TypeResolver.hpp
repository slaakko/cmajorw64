// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_TYPE_RESOLVER_INCLUDED
#define CMAJOR_BINDER_TYPE_RESOLVER_INCLUDED
#include <cmajor/ast/TypeExpr.hpp>
#include <cmajor/symbols/TypeSymbol.hpp>

namespace cmajor {  namespace binder {

using namespace cmajor::ast;
using namespace cmajor::symbols;

class BoundCompileUnit;

TypeSymbol* ResolveType(Node* typeExprNode, BoundCompileUnit& boundCompileUnit, ContainerScope* containerScope);

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_TYPE_RESOLVER_INCLUDED
