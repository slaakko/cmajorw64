// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_INLINE_FUNCTION_REPOSITORY_INCLUDED
#define CMAJOR_BINDER_INLINE_FUNCTION_REPOSITORY_INCLUDED
#include <cmajor/symbols/FunctionSymbol.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::symbols;

class BoundCompileUnit;

class InlineFunctionRepository
{
public:
    InlineFunctionRepository(BoundCompileUnit& boundCompileUnit_);
    ~InlineFunctionRepository();
    void Instantiate(FunctionSymbol* inlineFunction, ContainerScope* containerScope, const Span& span);
private:
    BoundCompileUnit& boundCompileUnit;
    std::unordered_set<FunctionSymbol*> instantiatedInlineFunctions;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_INLINE_FUNCTION_REPOSITORY_INCLUDED
