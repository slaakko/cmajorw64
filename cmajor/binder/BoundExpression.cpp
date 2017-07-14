// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundExpression.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/ir/Emitter.hpp>

namespace cmajor { namespace binder {

BoundExpression::BoundExpression(const Span& span_, TypeSymbol* type_) : BoundNode(span_), type(type_)
{
}

BoundLocalVariable::BoundLocalVariable(LocalVariableSymbol* localVariableSymbol_) : BoundExpression(localVariableSymbol_->GetSpan(), localVariableSymbol_->GetType()), localVariableSymbol(localVariableSymbol_)
{
}

void BoundLocalVariable::Load(Emitter& emitter)
{
    emitter.Stack().Push(emitter.Builder().CreateLoad(localVariableSymbol->Storage()));
}

void BoundLocalVariable::Store(Emitter& emitter)
{
    llvm::Value* value = emitter.Stack().Pop();
    emitter.Builder().CreateStore(value, localVariableSymbol->Storage());
}

BoundFunctionCall::BoundFunctionCall(const Span& span_, FunctionSymbol* functionSymbol_) : BoundExpression(span_, functionSymbol_->ReturnType()), functionSymbol(functionSymbol_)
{
}

void BoundFunctionCall::AddArgument(std::unique_ptr<BoundExpression>&& argument)
{
    arguments.push_back(std::move(argument));
}

void BoundFunctionCall::Load(Emitter& emitter)
{
    std::vector<GenObject*> genObjects;
    for (const std::unique_ptr<BoundExpression>& argument : arguments)
    {
        genObjects.push_back(argument.get());
    }
    functionSymbol->GenerateCall(emitter, genObjects);
}

void BoundFunctionCall::Store(Emitter& emitter)
{
    throw Exception("cannot store to function call", GetSpan(), functionSymbol->GetSpan());
}

} } // namespace cmajor::binder
