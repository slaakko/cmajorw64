// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/TemplateSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/Sha1.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

TemplateParameterSymbol::TemplateParameterSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::templateParameterSymbol, span_, name_), hasDefault(false)
{
}


void TemplateParameterSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    writer.GetBinaryWriter().Write(hasDefault);
}

void TemplateParameterSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    hasDefault = reader.GetBinaryReader().ReadBool();
}

TypeSymbol* TemplateParameterSymbol::Unify(TypeSymbol* type, const Span& span)
{
    return type;
}

BoundTemplateParameterSymbol::BoundTemplateParameterSymbol(const Span& span_, const std::u32string& name_) : Symbol(SymbolType::boundTemplateParameterSymbol, span_, name_), type(nullptr)
{
}

} } // namespace cmajor::symbols
