// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_DERIVED_TYPE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_DERIVED_TYPE_SYMBOL_INCLUDED
#include <cmajor/symbols/TypeSymbol.hpp>
#include <llvm/ADT/SmallVector.h>

namespace cmajor { namespace symbols {

enum class Derivation : uint8_t
{
    none = 0, constDerivation = 1, lvalueRefDerivation = 2, rvalueRefDerivation = 3, pointerDerivation = 4
};

std::u32string DerivationStr(Derivation derivation);

typedef llvm::SmallVector<Derivation, 8> DerivationVec;

struct TypeDerivationRec
{
    DerivationVec derivations;
    bool IsEmpty() const { return derivations.empty(); }
};

inline bool operator==(const TypeDerivationRec& left, const TypeDerivationRec& right)
{
    return left.derivations == right.derivations;
}

inline bool operator!=(const TypeDerivationRec& left, const TypeDerivationRec& right)
{
    return !(left == right);
}

std::u32string MakeDerivedTypeName(TypeSymbol* baseType, const TypeDerivationRec& derivationRec);

bool HasFrontConstDerivation(const DerivationVec& derivations);
bool HasReferenceDerivation(const DerivationVec& derivations);
bool HasLvalueReferenceDerivation(const DerivationVec& derivations);
bool HasRvalueReferenceDerivation(const DerivationVec& derivations);
bool HasReferenceOrConstDerivation(const DerivationVec& derivations);
bool HasPointerDerivation(const DerivationVec& derivations);
int CountPointerDerivations(const DerivationVec& derivations);

TypeDerivationRec MakePlainDerivationRec(const TypeDerivationRec& typeDerivationRec);
TypeDerivationRec RemoveReferenceDerivation(const TypeDerivationRec& typeDerivationRec);
TypeDerivationRec RemovePointerDerivation(const TypeDerivationRec& typeDerivationRec);
TypeDerivationRec RemoveConstDerivation(const TypeDerivationRec& typeDerivationRec);
TypeDerivationRec AddConstDerivation(const TypeDerivationRec& typeDerivationRec);
TypeDerivationRec AddLvalueReferenceDerivation(const TypeDerivationRec& typeDerivationRec);
TypeDerivationRec AddRvalueReferenceDerivation(const TypeDerivationRec& typeDerivationRec);
TypeDerivationRec AddPointerDerivation(const TypeDerivationRec& typeDerivationRec);
TypeDerivationRec UnifyDerivations(const TypeDerivationRec& left, const TypeDerivationRec& right);

class DerivedTypeSymbol : public TypeSymbol
{
public:
    DerivedTypeSymbol(const Span& span_, const std::u32string& name_);
    DerivedTypeSymbol(const Span& span_, const std::u32string& name_, TypeSymbol* baseType_, const TypeDerivationRec& derivationRec_);
    std::string TypeString() const override { return "derived_type"; }
    std::u32string SimpleName() const;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol, int index) override;
    void ComputeExportClosure() override;
    const TypeSymbol* BaseType() const override { return baseType; }
    TypeSymbol* BaseType() override { return baseType; }
    TypeSymbol* PlainType(const Span& span) override;
    TypeSymbol* RemoveReference(const Span& span) override;
    TypeSymbol* RemovePointer(const Span& span) override;
    TypeSymbol* RemoveConst(const Span& span) override;
    TypeSymbol* AddConst(const Span& span) override;
    TypeSymbol* AddLvalueReference(const Span& span) override;
    TypeSymbol* AddRvalueReference(const Span& span) override;
    TypeSymbol* AddPointer(const Span& span) override;
    llvm::Type* IrType(Emitter& emitter) override;
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override;
    bool IsConstType() const override;
    bool IsReferenceType() const override;
    bool IsLvalueReferenceType() const override;
    bool IsRvalueReferenceType() const override;
    bool IsPointerType() const override;
    bool IsVoidPtrType() const override;
    int PointerCount() const override;
    bool ContainsTemplateParameter() const override { return baseType->ContainsTemplateParameter(); }
    const TypeDerivationRec& DerivationRec() const override { return derivationRec; }
    TypeSymbol* RemoveDerivations(const TypeDerivationRec& sourceDerivationRec, const Span& span) override;
    TypeSymbol* Unify(TypeSymbol* sourceType, const Span& span) override;
    bool IsRecursive(TypeSymbol* type, std::unordered_set<TypeSymbol*>& tested) override;
    ValueType GetValueType() const override;
private:
    TypeSymbol* baseType;
    TypeDerivationRec derivationRec;
    llvm::Type* irType;
};

class NullPtrType : public TypeSymbol
{
public:
    NullPtrType(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "nullptr_type"; }
    bool IsPointerType() const override { return true; }
    bool IsNullPtrType() const override { return true; }
    llvm::Type* IrType(Emitter& emitter) override;
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override;
    ValueType GetValueType() const override;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_DERIVED_TYPE_SYMBOL_INCLUDED
