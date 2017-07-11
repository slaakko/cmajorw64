// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/NamespaceSymbol.hpp>

namespace cmajor { namespace symbols {

NamespaceSymbol::NamespaceSymbol(const Span& span_, const std::u32string& name_) : ContainerSymbol(SymbolType::namespaceSymbol, span_, name_)
{
}

} } // namespace cmajor::symbols
