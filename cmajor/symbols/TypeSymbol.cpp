// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

TypeSymbol::TypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) : ContainerSymbol(symbolType_, span_, name_), typeId(0)
{
}

void TypeSymbol::Write(SymbolWriter& writer)
{
    ContainerSymbol::Write(writer);
    Assert(typeId != 0, "type id not initialized");
    writer.GetBinaryWriter().Write(typeId);
}

void TypeSymbol::Read(SymbolReader& reader)
{
    ContainerSymbol::Read(reader);
    typeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->AddTypeOrConceptSymbolToTypeIdMap(this);
}

TypeSymbol* TypeSymbol::AddConst(const Span& span)
{
    TypeDerivationRec typeDerivationRec;
    typeDerivationRec.derivations.push_back(Derivation::constDerivation);
    return GetSymbolTable()->MakeDerivedType(this, typeDerivationRec, span);
}

TypeSymbol* TypeSymbol::AddLvalueReference(const Span& span)
{
    TypeDerivationRec typeDerivationRec;
    typeDerivationRec.derivations.push_back(Derivation::lvalueRefDerivation);
    return GetSymbolTable()->MakeDerivedType(this, typeDerivationRec, span);
}

TypeSymbol* TypeSymbol::AddRvalueReference(const Span& span)
{
    TypeDerivationRec typeDerivationRec;
    typeDerivationRec.derivations.push_back(Derivation::rvalueRefDerivation);
    return GetSymbolTable()->MakeDerivedType(this, typeDerivationRec, span);
}

TypeSymbol* TypeSymbol::AddPointer(const Span& span)
{
    TypeDerivationRec typeDerivationRec;
    typeDerivationRec.derivations.push_back(Derivation::pointerDerivation);
    return GetSymbolTable()->MakeDerivedType(this, typeDerivationRec, span);
}

const TypeDerivationRec& TypeSymbol::DerivationRec() const
{
    static TypeDerivationRec emptyDerivationRec;
    return emptyDerivationRec;
}

TypeSymbol* TypeSymbol::RemoveDerivations(const TypeDerivationRec& sourceDerivationRec, const Span& span)
{
    if (HasPointerDerivation(sourceDerivationRec.derivations)) return nullptr;
    return this;
}

bool TypeSymbol::IsRecursive(TypeSymbol* type, std::unordered_set<TypeSymbol*>& tested) 
{ 
    if (tested.find(this) != tested.cend()) return type == this;
    tested.insert(this);
    return TypesEqual(type, this); 
}

ValueType TypeSymbol::GetValueType() const
{
    return ValueType::none;
}

std::u32string TypeSymbol::Id() const 
{ 
    return ToUtf32(std::to_string(TypeId())); 
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
