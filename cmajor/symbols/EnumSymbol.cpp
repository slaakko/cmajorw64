// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/EnumSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/Exception.hpp>

namespace cmajor { namespace symbols {

EnumTypeSymbol::EnumTypeSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::enumTypeSymbol, span_, name_), underlyingType()
{
}

void EnumTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    writer.GetBinaryWriter().WriteEncodedUInt(underlyingType->TypeId());
}

void EnumTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    uint32_t underlyingTypeId = reader.GetBinaryReader().ReadEncodedUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, underlyingTypeId, 0);
}

void EnumTypeSymbol::EmplaceType(TypeSymbol* typeSymbol_, int index)
{
    Assert(index == 0, "invalid emplace type index");
    underlyingType = typeSymbol_;
}

void EnumTypeSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be default", GetSpan());
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be constexpr", GetSpan());
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be nothrow", GetSpan());
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be throw", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be unit_test", GetSpan());
    }
}

EnumConstantSymbol::EnumConstantSymbol(const Span& span_, const std::u32string& name_) : Symbol(SymbolType::enumConstantSymbol, span_, name_), evaluating(false)
{
}

} } // namespace cmajor::symbols
