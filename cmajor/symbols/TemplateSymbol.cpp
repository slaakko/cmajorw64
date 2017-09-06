// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/TemplateSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>

namespace cmajor { namespace symbols {

TemplateParameterSymbol::TemplateParameterSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::templateParameterSymbol, span_, name_)
{
}

TypeSymbol* TemplateParameterSymbol::Unify(TypeSymbol* type, const Span& span)
{
    return type;
}

BoundTemplateParameterSymbol::BoundTemplateParameterSymbol(const Span& span_, const std::u32string& name_) : Symbol(SymbolType::boundTemplateParameterSymbol, span_, name_), type(nullptr)
{
}

} } // namespace cmajor::symbols
