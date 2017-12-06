// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/ConstExprFunctionRepository.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>

namespace cmajor { namespace binder {

ConstExprFunctionRepository::ConstExprFunctionRepository(BoundCompileUnit& boundCompileUnit_) : boundCompileUnit(boundCompileUnit_)
{
}

FunctionNode* ConstExprFunctionRepository::GetFunctionNodeFor(FunctionSymbol* constExprFunctionSymbol)
{
    Node* node = boundCompileUnit.GetSymbolTable().GetNodeNoThrow(constExprFunctionSymbol);
    if (!node)
    {
        constExprFunctionSymbol->ReadAstNodes();
        node = boundCompileUnit.GetSymbolTable().GetNode(constExprFunctionSymbol);
    }
    if (node->IsFunctionNode())
    {
        return static_cast<FunctionNode*>(node);
    }
    else
    {
        throw Exception("internal error: function node expected", constExprFunctionSymbol->GetSpan());
    }
}

} } // namespace cmajor::binder
