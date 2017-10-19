// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/TypedefSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/SymbolCollector.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

TypedefSymbol::TypedefSymbol(const Span& span_, const std::u32string& name_) : Symbol(SymbolType::typedefSymbol, span_, name_), type()
{
}

void TypedefSymbol::Write(SymbolWriter& writer)
{
    Symbol::Write(writer);
    writer.GetBinaryWriter().Write(type->TypeId());
}

void TypedefSymbol::Read(SymbolReader& reader)
{
    Symbol::Read(reader);
    uint32_t typeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, typeId, 0);
}

void TypedefSymbol::EmplaceType(TypeSymbol* typeSymbol, int index)
{
    Assert(index == 0, "invalid emplace type index");
    type = typeSymbol;
}

bool TypedefSymbol::IsExportSymbol() const
{
    if (Parent()->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol) return false;
    return Symbol::IsExportSymbol();
}

void TypedefSymbol::ComputeExportClosure()
{
    if (IsProject())
    {
        if (!type->ExportComputed())
        {
            type->SetExportComputed();
            type->ComputeExportClosure();
        }
    }
}

void TypedefSymbol::Accept(SymbolCollector* collector)
{
    if (IsProject())
    {
        collector->AddTypedef(this);
    }
}

void TypedefSymbol::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine(ToUtf8(Name()));
    formatter.WriteLine("full name: " + ToUtf8(FullNameWithSpecifiers()));
    formatter.WriteLine("type: " + ToUtf8(type->FullName()));
}

void TypedefSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("typedef cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("typedef cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("typedef cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("typedef cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("typedef cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception("typedef cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("typedef cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception("typedef cannot be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        throw Exception("typedef cannot be default", GetSpan());
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        throw Exception("typedef cannot be constexpr", GetSpan());
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception("typedef cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        throw Exception("typedef cannot be nothrow", GetSpan());
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        throw Exception("typedef cannot be throw", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("typedef cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception("typedef cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("typedef cannot be unit_test", GetSpan());
    }
}

} } // namespace cmajor::symbols
