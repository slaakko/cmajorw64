// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/DerivedTypeSymbol.hpp>

namespace cmajor { namespace symbols {

DerivedTypeSymbol::DerivedTypeSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::derivedTypeSymbol, span_, name_)
{
}

} } // namespace cmajor::symbols