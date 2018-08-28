// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/NamespaceSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>

namespace cmajor { namespace symbols {

NamespaceSymbol::NamespaceSymbol(const Span& span_, const std::u32string& name_) : ContainerSymbol(SymbolType::namespaceSymbol, span_, name_)
{
}

void NamespaceSymbol::Import(NamespaceSymbol* that, SymbolTable& symbolTable)
{
    symbolTable.BeginNamespace(that->Name(), that->GetSpan(), that->GetOriginalModule());
    for (std::unique_ptr<Symbol>& symbol : that->Members())
    {
        if (symbol->GetSymbolType() == SymbolType::namespaceSymbol)
        {
            NamespaceSymbol* thatNs = static_cast<NamespaceSymbol*>(symbol.get());
            Import(thatNs, symbolTable);
        }
        else if (symbol->GetSymbolType() != SymbolType::functionGroupSymbol && 
            symbol->GetSymbolType() != SymbolType::conceptGroupSymbol && 
            symbol->GetSymbolType() != SymbolType::classGroupTypeSymbol)
        {
            symbolTable.Container()->AddMember(symbol.release());
        }
    }
    symbolTable.EndNamespace();
}

std::unique_ptr<dom::Element> NamespaceSymbol::CreateDomElement(TypeMap& typeMap)
{
    if (HasProjectMembers())
    {
        return std::unique_ptr<dom::Element>(new dom::Element(U"NamespaceSymbol"));
    }
    else
    {
        return std::unique_ptr<dom::Element>();
    }
}

} } // namespace cmajor::symbols
