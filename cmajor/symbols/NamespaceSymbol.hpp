// =================================
// Copyright (c) 2018 Seppo Laakko
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
    std::u32string Id() const override { return FullName(); }
    void Import(NamespaceSymbol* that, SymbolTable& symbolTable);
    bool IsGlobalNamespace() const { return Name().empty(); }
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_NAMESPACE_SYMBOL_INCLUDED
