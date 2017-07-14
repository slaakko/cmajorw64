// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_BOUND_FUNCTION_INCLUDED
#define CMAJOR_BINDER_BOUND_FUNCTION_INCLUDED
#include <cmajor/binder/BoundNode.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::symbols;

class BoundFunction : public BoundNode
{
public:
    BoundFunction(FunctionSymbol* functionSymbol_);
    const FunctionSymbol* GetFunctionSymbol() const { return functionSymbol; }
    FunctionSymbol* GetFunctionSymbol() { return functionSymbol; }
private:
    FunctionSymbol* functionSymbol;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_BOUND_FUNCTION_INCLUDED
