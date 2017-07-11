// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_INTERFACE_TYPE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_INTERFACE_TYPE_SYMBOL_INCLUDED
#include <cmajor/symbols/TypeSymbol.hpp>

namespace cmajor { namespace symbols {

class InterfaceTypeSymbol : public TypeSymbol
{
public:
    InterfaceTypeSymbol(const Span& span_, const std::u32string& name_);
    void SetSpecifiers(Specifiers specifiers);
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_INTERFACE_TYPE_SYMBOL_INCLUDED
