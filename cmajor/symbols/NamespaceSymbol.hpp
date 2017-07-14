// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_NAMESPACE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_NAMESPACE_SYMBOL_INCLUDED
#include <cmajor/symbols/ContainerSymbol.hpp>

namespace cmajor { namespace symbols {

class NamespaceSymbol : public ContainerSymbol
{
public:   
    NamespaceSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "namespace"; }
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_NAMESPACE_SYMBOL_INCLUDED
