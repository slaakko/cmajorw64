// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>

namespace cmajor { namespace symbols {

TypeSymbol::TypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) : ContainerSymbol(symbolType_, span_, name_), typeId(0)
{
}

void TypeSymbol::Write(SymbolWriter& writer)
{
    ContainerSymbol::Write(writer);
    Assert(typeId != 0, "type id not initialized");
    writer.GetBinaryWriter().WriteEncodedUInt(typeId);
}

void TypeSymbol::Read(SymbolReader& reader)
{
    ContainerSymbol::Read(reader);
    typeId = reader.GetBinaryReader().ReadEncodedUInt();
    GetSymbolTable()->AddTypeSymbolToTypeIdMap(this);
}

bool CompareTypesForEquality(const TypeSymbol* left, const TypeSymbol* right)
{
    if (left->GetSymbolType() == SymbolType::derivedTypeSymbol && right->GetSymbolType() == SymbolType::derivedTypeSymbol)
    {
        const DerivedTypeSymbol* derivedLeft = static_cast<const DerivedTypeSymbol*>(left);
        const DerivedTypeSymbol* derivedRight = static_cast<const DerivedTypeSymbol*>(right);
        if (TypesEqual(derivedLeft->BaseType(), derivedRight->BaseType()) && derivedLeft->DerivationRec() == derivedRight->DerivationRec())
        {
            return true;
        }
    }
    return false;
}

} } // namespace cmajor::symbols
