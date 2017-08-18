// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/DerivedTypeSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

std::u32string DerivationStr(Derivation derivation)
{
    switch (derivation)
    {
        case Derivation::constDerivation: return U"C";
        case Derivation::lvalueRefDerivation: return U"R";
        case Derivation::rvalueRefDerivation: return U"RR";
        case Derivation::pointerDerivation: return U"P";
        case Derivation::arrayDerivation: return U"A";
        default: return std::u32string();
    }
}

bool HasFrontConstDerivation(const DerivationVec& derivations)
{
    if (!derivations.empty())
    {
        if (derivations[0] == Derivation::constDerivation)
        {
            return true;
        }
    }
    return false;
}

bool HasReferenceDerivation(const DerivationVec& derivations)
{
    for (Derivation derivation : derivations)
    {
        if (derivation == Derivation::lvalueRefDerivation || derivation == Derivation::rvalueRefDerivation)
        {
            return true;
        }
    }
    return false;
}

bool HasLvalueReferenceDerivation(const DerivationVec& derivations) 
{
    for (Derivation derivation : derivations)
    {
        if (derivation == Derivation::lvalueRefDerivation)
        {
            return true;
        }
    }
    return false;
}

bool HasRvalueReferenceDerivation(const DerivationVec& derivations)
{
    for (Derivation derivation : derivations)
    {
        if (derivation == Derivation::rvalueRefDerivation)
        {
            return true;
        }
    }
    return false;
}

bool HasArrayDerivation(const DerivationVec& derivations)
{
    for (Derivation derivation : derivations)
    {
        if (derivation == Derivation::arrayDerivation)
        {
            return true;
        }
    }
    return false;
}

bool HasReferenceOrConstDerivation(const DerivationVec& derivations)
{
    for (Derivation derivation : derivations)
    {
        if (derivation == Derivation::lvalueRefDerivation || derivation == Derivation::rvalueRefDerivation || derivation == Derivation::constDerivation)
        {
            return true;
        }
    }
    return false;
}

bool HasPointerDerivation(const DerivationVec& derivations)
{
    for (Derivation derivation : derivations)
    {
        if (derivation == Derivation::pointerDerivation) return true;
    }
    return false;
}

int CountPointerDerivations(const DerivationVec& derivations)
{
    int numPointers = 0;
    for (Derivation derivation : derivations)
    {
        if (derivation == Derivation::pointerDerivation)
        {
            ++numPointers;
        }
    }
    return numPointers;
}

TypeDerivationRec MakePlainDerivationRec(const TypeDerivationRec& typeDerivationRec)
{
    TypeDerivationRec plainDerivationRec;
    plainDerivationRec.arrayDimensions = typeDerivationRec.arrayDimensions;
    for (Derivation derivation : typeDerivationRec.derivations)
    {
        if (derivation == Derivation::pointerDerivation || derivation == Derivation::arrayDerivation)
        {
            plainDerivationRec.derivations.push_back(derivation);
        }
    }
    return plainDerivationRec;
}

TypeDerivationRec RemoveReferenceDerivation(const TypeDerivationRec& typeDerivationRec)
{
    TypeDerivationRec referenceRemovedDerivationRec;
    referenceRemovedDerivationRec.arrayDimensions = typeDerivationRec.arrayDimensions;
    for (Derivation derivation : typeDerivationRec.derivations)
    {
        if (derivation == Derivation::constDerivation || derivation == Derivation::pointerDerivation || derivation == Derivation::arrayDerivation)
        {
            referenceRemovedDerivationRec.derivations.push_back(derivation);
        }
    }
    return referenceRemovedDerivationRec;
}

TypeDerivationRec RemovePointerDerivation(const TypeDerivationRec& typeDerivationRec)
{
    TypeDerivationRec pointerRemovedDerivationRec;
    pointerRemovedDerivationRec.arrayDimensions = typeDerivationRec.arrayDimensions;
    int numPointers = CountPointerDerivations(typeDerivationRec.derivations);
    if (numPointers > 0)
    {
        --numPointers;
    }
    int ip = 0;
    int n = typeDerivationRec.derivations.size();
    for (int i = 0; i < n; ++i)
    {
        Derivation derivation = typeDerivationRec.derivations[i];
        if (derivation == Derivation::pointerDerivation)
        {
            if (ip < numPointers)
            {
                pointerRemovedDerivationRec.derivations.push_back(derivation);
                ++ip;
            }
        }
        else
        {
            pointerRemovedDerivationRec.derivations.push_back(derivation);
        }
    }
    return pointerRemovedDerivationRec;
}

TypeDerivationRec RemoveConstDerivation(const TypeDerivationRec& typeDerivationRec)
{
    TypeDerivationRec constRemovedDerivationRec;
    constRemovedDerivationRec.arrayDimensions = typeDerivationRec.arrayDimensions;
    for (Derivation derivation : typeDerivationRec.derivations)
    {
        if (derivation != Derivation::constDerivation)
        {
            constRemovedDerivationRec.derivations.push_back(derivation);
        }
    }
    return constRemovedDerivationRec;
}

TypeDerivationRec AddConstDerivation(const TypeDerivationRec& typeDerivationRec)
{
    TypeDerivationRec constAddedDerivationRec;
    constAddedDerivationRec.arrayDimensions = typeDerivationRec.arrayDimensions;
    constAddedDerivationRec.derivations.push_back(Derivation::constDerivation);
    for (Derivation derivation : typeDerivationRec.derivations)
    {
        if (derivation != Derivation::constDerivation)
        {
            constAddedDerivationRec.derivations.push_back(derivation);
        }
    }
    return constAddedDerivationRec;
}

TypeDerivationRec AddLvalueReferenceDerivation(const TypeDerivationRec& typeDerivationRec)
{
    TypeDerivationRec lvalueReferenceAddedDerivationRec;
    lvalueReferenceAddedDerivationRec.arrayDimensions = typeDerivationRec.arrayDimensions;
    for (Derivation derivation : typeDerivationRec.derivations)
    {
        if (derivation != Derivation::lvalueRefDerivation && derivation != Derivation::rvalueRefDerivation)
        {
            lvalueReferenceAddedDerivationRec.derivations.push_back(derivation);
        }
    }
    lvalueReferenceAddedDerivationRec.derivations.push_back(Derivation::lvalueRefDerivation);
    return lvalueReferenceAddedDerivationRec;
}

TypeDerivationRec AddRvalueReferenceDerivation(const TypeDerivationRec& typeDerivationRec)
{
    TypeDerivationRec rvalueReferenceAddedDerivationRec;
    rvalueReferenceAddedDerivationRec.arrayDimensions = typeDerivationRec.arrayDimensions;
    for (Derivation derivation : typeDerivationRec.derivations)
    {
        if (derivation != Derivation::lvalueRefDerivation && derivation != Derivation::rvalueRefDerivation)
        {
            rvalueReferenceAddedDerivationRec.derivations.push_back(derivation);
        }
    }
    rvalueReferenceAddedDerivationRec.derivations.push_back(Derivation::rvalueRefDerivation);
    return rvalueReferenceAddedDerivationRec;
}

TypeDerivationRec AddPointerDerivation(const TypeDerivationRec& typeDerivationRec)
{
    TypeDerivationRec pointerAddedDerivationRec;
    pointerAddedDerivationRec.arrayDimensions = typeDerivationRec.arrayDimensions;
    int numPointers = CountPointerDerivations(typeDerivationRec.derivations);
    int np = 0;
    for (Derivation derivation : typeDerivationRec.derivations)
    {
        pointerAddedDerivationRec.derivations.push_back(derivation);
        if (derivation == Derivation::pointerDerivation)
        {
            ++np;
        }
        if (np == numPointers)
        {
            pointerAddedDerivationRec.derivations.push_back(Derivation::pointerDerivation);
            ++np;
        }
    }
    return pointerAddedDerivationRec;
}

TypeDerivationRec UnifyDerivations(const TypeDerivationRec& left, const TypeDerivationRec& right)
{
    if (HasArrayDerivation(left.derivations) || HasArrayDerivation(right.derivations))
    {
        throw std::runtime_error("arrays not supported yet");
    }
    TypeDerivationRec result;
    if (HasFrontConstDerivation(left.derivations) || HasFrontConstDerivation(right.derivations))
    {
        result.derivations.push_back(Derivation::constDerivation);
    }
    int pointerCount = CountPointerDerivations(left.derivations) + CountPointerDerivations(right.derivations);
    for (int i = 0; i < pointerCount; ++i)
    {
        result.derivations.push_back(Derivation::pointerDerivation);
    }
    if (HasLvalueReferenceDerivation(left.derivations))
    {
        result.derivations.push_back(Derivation::lvalueRefDerivation);
    }
    else if (HasRvalueReferenceDerivation(left.derivations))
    {
        result.derivations.push_back(Derivation::rvalueRefDerivation);
    }
    else if (HasLvalueReferenceDerivation(right.derivations))
    {
        result.derivations.push_back(Derivation::lvalueRefDerivation);
    }
    else if (HasRvalueReferenceDerivation(right.derivations))
    {
        result.derivations.push_back(Derivation::rvalueRefDerivation);
    }
    return result;
}

std::u32string MakeDerivedTypeName(TypeSymbol* baseType, const TypeDerivationRec& derivationRec)
{
    std::u32string derivedTypeName;
    bool constAdded = false;
    if (HasFrontConstDerivation(derivationRec.derivations))
    {
        derivedTypeName.append(U"const");
        constAdded = true;
    }
    if (!derivedTypeName.empty())
    {
        derivedTypeName.append(1, U' ');
    }
    derivedTypeName.append(baseType->Name());
    int dimensionIndex = 0;
    for (Derivation derivation : derivationRec.derivations)
    {
        switch (derivation)
        {
            case Derivation::constDerivation:
            {
                if (constAdded)
                {
                    constAdded = false;
                }
                else
                {
                    if (!derivedTypeName.empty())
                    {
                        derivedTypeName.append(1, U' ');
                    }
                    derivedTypeName.append(U"const ");
                }
                break;
            }
            case Derivation::lvalueRefDerivation:
            {
                derivedTypeName.append(U"&");
                break;
            }
            case Derivation::rvalueRefDerivation:
            {
                derivedTypeName.append(U"&&");
                break;
            }
            case Derivation::pointerDerivation:
            {
                derivedTypeName.append(U"*");
                break;
            }
            case Derivation::arrayDerivation:
            {
                if (dimensionIndex < derivationRec.arrayDimensions.size())
                {
                    derivedTypeName.append(1, U'[').append(ToUtf32(std::to_string(derivationRec.arrayDimensions[dimensionIndex++]))).append(1, U']');
                }
                else
                {
                    derivedTypeName.append(U"[]");
                }
                break;
            }
        }
    }
    return derivedTypeName;
}

DerivedTypeSymbol::DerivedTypeSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::derivedTypeSymbol, span_, name_), baseType(), derivationRec(), irType(nullptr)
{
}

DerivedTypeSymbol::DerivedTypeSymbol(const Span& span_, const std::u32string& name_, TypeSymbol* baseType_, const TypeDerivationRec& derivationRec_) : 
    TypeSymbol(SymbolType::derivedTypeSymbol, span_, name_), baseType(baseType_), derivationRec(derivationRec_), irType(nullptr)
{
}

std::u32string DerivedTypeSymbol::SimpleName() const
{
    std::u32string simpleName = baseType->SimpleName();
    for (Derivation derivation : derivationRec.derivations)
    {
        simpleName.append(U"_").append(DerivationStr(derivation));
    }
    return simpleName;
}

void DerivedTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    writer.GetBinaryWriter().WriteEncodedUInt(baseType->TypeId());
    writer.GetBinaryWriter().Write(static_cast<uint8_t>(derivationRec.derivations.size()));
    for (Derivation derivation : derivationRec.derivations)
    {
        writer.GetBinaryWriter().Write(static_cast<uint8_t>(derivation));
    }
    writer.GetBinaryWriter().Write(static_cast<uint8_t>(derivationRec.arrayDimensions.size()));
    for (uint64_t dimension : derivationRec.arrayDimensions)
    {
        writer.GetBinaryWriter().Write(dimension);
    }
}

void DerivedTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    uint32_t typeId = reader.GetBinaryReader().ReadEncodedUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, typeId, 0);
    uint8_t nd = reader.GetBinaryReader().ReadByte();
    for (uint8_t i = 0; i < nd; ++i)
    {
        Derivation derivation = static_cast<Derivation>(reader.GetBinaryReader().ReadByte());
        derivationRec.derivations.push_back(derivation);
    }
    uint8_t na = reader.GetBinaryReader().ReadByte();
    for (uint8_t i = 0; i < na; ++i)
    {
        uint64_t dimension = reader.GetBinaryReader().ReadULong();
        derivationRec.arrayDimensions.push_back(dimension);
    }
}

void DerivedTypeSymbol::EmplaceType(TypeSymbol* typeSymbol_, int index)
{
    Assert(index == 0, "invalid emplace type index");
    baseType = typeSymbol_;
}

void DerivedTypeSymbol::ComputeExportClosure()
{
    if (IsProject())
    {
        if (!baseType->ExportComputed())
        {
            baseType->SetExportComputed();
            baseType->ComputeExportClosure();
        }
        MarkExport();
    }
}

bool DerivedTypeSymbol::IsConstType() const
{
    return HasFrontConstDerivation(derivationRec.derivations);
}

bool DerivedTypeSymbol::IsReferenceType() const 
{
    return HasReferenceDerivation(derivationRec.derivations);
}

bool DerivedTypeSymbol::IsLvalueReferenceType() const
{
    return HasLvalueReferenceDerivation(derivationRec.derivations);
}

bool DerivedTypeSymbol::IsRvalueReferenceType() const
{
    return HasRvalueReferenceDerivation(derivationRec.derivations);
}

bool DerivedTypeSymbol::IsArrayType() const
{
    return HasArrayDerivation(derivationRec.derivations);
}

bool DerivedTypeSymbol::IsPointerType() const 
{
    return HasPointerDerivation(derivationRec.derivations);
}

bool DerivedTypeSymbol::IsVoidPtrType() const 
{
    return baseType->IsVoidType() && derivationRec.derivations.size() == 1 && derivationRec.derivations.front() == Derivation::pointerDerivation;
}

int DerivedTypeSymbol::PointerCount() const
{
    return CountPointerDerivations(derivationRec.derivations);
}

TypeSymbol* DerivedTypeSymbol::PlainType(const Span& span) 
{
    return GetSymbolTable()->MakeDerivedType(baseType, MakePlainDerivationRec(derivationRec), span);
}

TypeSymbol* DerivedTypeSymbol::RemoveReference(const Span& span)
{
    return GetSymbolTable()->MakeDerivedType(baseType, RemoveReferenceDerivation(derivationRec), span);
}

TypeSymbol* DerivedTypeSymbol::RemovePointer(const Span& span)
{
    return GetSymbolTable()->MakeDerivedType(baseType, RemovePointerDerivation(derivationRec), span);
}

TypeSymbol* DerivedTypeSymbol::RemoveConst(const Span& span) 
{
    return GetSymbolTable()->MakeDerivedType(baseType, RemoveConstDerivation(derivationRec), span);
}

TypeSymbol* DerivedTypeSymbol::AddConst(const Span& span)
{
    if (IsConstType())
    {
        return this;
    }
    else
    {
        return GetSymbolTable()->MakeDerivedType(baseType, AddConstDerivation(derivationRec), span);
    }
}

TypeSymbol* DerivedTypeSymbol::AddLvalueReference(const Span& span)
{
    if (IsLvalueReferenceType())
    {
        return this;
    }
    else
    {
        return GetSymbolTable()->MakeDerivedType(baseType, AddLvalueReferenceDerivation(derivationRec), span);
    }
}

TypeSymbol* DerivedTypeSymbol::AddRvalueReference(const Span& span)
{
    if (IsRvalueReferenceType())
    {
        return this;
    }
    else
    {
        return GetSymbolTable()->MakeDerivedType(baseType, AddRvalueReferenceDerivation(derivationRec), span);
    }
}

TypeSymbol* DerivedTypeSymbol::AddPointer(const Span& span)
{
    return GetSymbolTable()->MakeDerivedType(baseType, AddPointerDerivation(derivationRec), span);
}

TypeSymbol* DerivedTypeSymbol::RemoveDerivations(const TypeDerivationRec& sourceDerivationRec, const Span& span)
{
    TypeDerivationRec result;
    if (HasArrayDerivation(derivationRec.derivations) || HasArrayDerivation(sourceDerivationRec.derivations))
    {
        throw std::runtime_error("arrays not supported yet");
    }
    const DerivationVec& sourceDerivations = sourceDerivationRec.derivations;
    if (!HasFrontConstDerivation(sourceDerivations) && HasFrontConstDerivation(derivationRec.derivations))
    {
        result.derivations.push_back(Derivation::constDerivation);
    }
    int pointerDiff = CountPointerDerivations(derivationRec.derivations) - CountPointerDerivations(sourceDerivations);
    if (pointerDiff != 0)
    {
        for (int i = 0; i < pointerDiff; ++i)
        {
            result.derivations.push_back(Derivation::pointerDerivation);
        }
    }
    if (!HasLvalueReferenceDerivation(sourceDerivations) && HasLvalueReferenceDerivation(derivationRec.derivations))
    {
        result.derivations.push_back(Derivation::lvalueRefDerivation);
    }
    else if (!HasRvalueReferenceDerivation(sourceDerivations) && HasRvalueReferenceDerivation(derivationRec.derivations))
    {
        result.derivations.push_back(Derivation::rvalueRefDerivation);
    }
    return GetSymbolTable()->MakeDerivedType(baseType, result, span);
}

TypeSymbol* DerivedTypeSymbol::Unify(TypeSymbol* sourceType, const Span& span) 
{
    TypeSymbol* newBaseType = baseType->Unify(sourceType->BaseType(), span);
    return GetSymbolTable()->MakeDerivedType(newBaseType, UnifyDerivations(derivationRec, sourceType->DerivationRec()), span);
}

llvm::Type* DerivedTypeSymbol::IrType(Emitter& emitter) 
{
    if (!irType)
    { 
        if (baseType->IsVoidType())
        {
            irType = emitter.Builder().getInt8Ty();
        }
        else
        {
            irType = baseType->IrType(emitter);
        }
        for (Derivation derivation : derivationRec.derivations)
        {
            switch (derivation)
            {
                case Derivation::lvalueRefDerivation:
                case Derivation::rvalueRefDerivation:
                case Derivation::pointerDerivation:
                {
                    irType = llvm::PointerType::get(irType, 0);
                    break;
                }
                default:
                {
                    // todo
                    break;
                }
            }
        }
    }
    return irType;
}

llvm::Constant* DerivedTypeSymbol::CreateDefaultIrValue(Emitter& emitter)
{
    if (HasFrontConstDerivation(derivationRec.derivations) && !HasPointerDerivation(derivationRec.derivations) && !HasReferenceDerivation(derivationRec.derivations))
    {
        return baseType->CreateDefaultIrValue(emitter);
    }
    else
    {
        return llvm::Constant::getNullValue(IrType(emitter));
    }
}

NullPtrType::NullPtrType(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::nullPtrTypeSymbol, span_, name_)
{
}

llvm::Type* NullPtrType::IrType(Emitter& emitter)
{
    return emitter.Builder().getInt8PtrTy();
}

llvm::Constant* NullPtrType::CreateDefaultIrValue(Emitter& emitter)
{
    return llvm::Constant::getNullValue(IrType(emitter));
}

} } // namespace cmajor::symbols
