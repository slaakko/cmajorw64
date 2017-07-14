// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundFunction.hpp>

namespace cmajor { namespace binder {

BoundFunction::BoundFunction(FunctionSymbol* functionSymbol_) : BoundNode(functionSymbol->GetSpan()), functionSymbol(functionSymbol_)
{
}

} } // namespace cmajor::binder
