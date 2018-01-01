// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/ConstExprFunctionRepository.hpp>
#include <cmajor/binder/TypeBinder.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>

namespace cmajor { namespace binder {

ConstExprFunctionRepository::ConstExprFunctionRepository(BoundCompileUnit& boundCompileUnit_) : boundCompileUnit(boundCompileUnit_)
{
}

FunctionNode* ConstExprFunctionRepository::GetFunctionNodeFor(FunctionSymbol* constExprFunctionSymbol)
{
    constExprFunctionSymbol->SetSymbolTable(&boundCompileUnit.GetSymbolTable());
    Node* node = boundCompileUnit.GetSymbolTable().GetNodeNoThrow(constExprFunctionSymbol);
    if (!node)
    {
        constExprFunctionSymbol->ReadAstNodes();
        node = boundCompileUnit.GetSymbolTable().GetNode(constExprFunctionSymbol);
    }
    if (node->IsFunctionNode())
    {
        if (constExprFunctionSymbol->IsProject() && !constExprFunctionSymbol->IsBound())
        {
            TypeBinder typeBinder(boundCompileUnit);
            node->Accept(typeBinder);
        }
        return static_cast<FunctionNode*>(node);
    }
    else
    {
        throw Exception("internal error: function node expected", constExprFunctionSymbol->GetSpan());
    }
}

} } // namespace cmajor::binder
