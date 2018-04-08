// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/util/Unicode.hpp>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

TypeSymbol::TypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) :
    ContainerSymbol(symbolType_, span_, name_), typeId(boost::uuids::nil_generator()()), compileUnitIndex(-1), diType(nullptr)
{
}

void TypeSymbol::Write(SymbolWriter& writer)
{
    ContainerSymbol::Write(writer);
    Assert(!typeId.is_nil(), "type id not set");
    writer.GetBinaryWriter().Write(typeId);
}

void TypeSymbol::Read(SymbolReader& reader)
{
    ContainerSymbol::Read(reader);
    reader.GetBinaryReader().ReadUuid(typeId);
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

llvm::DIType* TypeSymbol::CreateDIType(Emitter& emitter)
{
    return emitter.DIBuilder()->createUnspecifiedType(ToUtf8(Name()));
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
    return ToUtf32(boost::uuids::to_string(TypeId())); 
}

llvm::DIType* TypeSymbol::GetDIType(Emitter& emitter)
{
    if (!diType || compileUnitIndex != emitter.CompileUnitIndex())
    {
        compileUnitIndex = emitter.CompileUnitIndex();
        diType = emitter.GetDIType(this);
        if (!diType)
        {
            if (IsClassTypeSymbol())
            {
                ClassTypeSymbol* classTypeSymbol = static_cast<ClassTypeSymbol*>(this);
                diType = classTypeSymbol->CreateDIForwardDeclaration(emitter);
                emitter.SetDIType(this, diType);
            }
            diType = CreateDIType(emitter);
            emitter.SetDIType(this, diType);
        }
    }
    return diType;
}

uint64_t TypeSymbol::SizeInBits(Emitter& emitter) 
{
    return emitter.DataLayout()->getTypeSizeInBits(IrType(emitter));
}

uint32_t TypeSymbol::AlignmentInBits(Emitter& emitter)
{
    return 8 * emitter.DataLayout()->getABITypeAlignment(IrType(emitter));
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
