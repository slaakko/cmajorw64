// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_BOUND_EXPRESSION_INCLUDED
#define CMAJOR_BINDER_BOUND_EXPRESSION_INCLUDED
#include <cmajor/binder/BoundNode.hpp>
#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::symbols;

class BoundExpression : public BoundNode
{
public:
    BoundExpression(const Span& span_, TypeSymbol* type_);
    const TypeSymbol* GetType() const { return type; }
    TypeSymbol* GetType() { return type; }
private:
    TypeSymbol* type;
};

class BoundLocalVariable : public BoundExpression
{
public:
    BoundLocalVariable(LocalVariableSymbol* localVariableSymbol_);
    void Load(Emitter& emitter) override;
    void Store(Emitter& emitter) override;
private:
    LocalVariableSymbol* localVariableSymbol;
};

class BoundFunctionCall : public BoundExpression
{
public:
    BoundFunctionCall(const Span& span_, FunctionSymbol* functionSymbol_);
    void AddArgument(std::unique_ptr<BoundExpression>&& argument);
    void Load(Emitter& emitter) override;
    void Store(Emitter& emitter) override;
    const FunctionSymbol* GetFunctionSymbol() const { return functionSymbol; }
    FunctionSymbol* GetFunctionSymbol() { return functionSymbol; }
private:
    FunctionSymbol* functionSymbol;
    std::vector<std::unique_ptr<BoundExpression>> arguments;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_BOUND_EXPRESSION_INCLUDED
