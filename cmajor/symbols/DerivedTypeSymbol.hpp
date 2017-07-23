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
    none = 0, constDerivation = 1, lvalueRefDerivation = 2, rvalueRefDerivation = 3, pointerDerivation = 4, arrayDerivation = 5
};

typedef llvm::SmallVector<Derivation, 8> DerivationVec;
typedef llvm::SmallVector<uint64_t, 4> ArrayDimensionVec;

struct TypeDerivationRec
{
    DerivationVec derivations;
    ArrayDimensionVec arrayDimensions;
    bool IsEmpty() const { return derivations.empty() && arrayDimensions.empty(); }
};

inline bool operator==(const TypeDerivationRec& left, const TypeDerivationRec& right)
{
    return left.derivations == right.derivations && left.arrayDimensions == right.arrayDimensions;
}

inline bool operator!=(const TypeDerivationRec& left, const TypeDerivationRec& right)
{
    return !(left == right);
}

std::u32string MakeDerivedTypeName(TypeSymbol* baseType, const TypeDerivationRec& derivationRec);

bool HasReferenceDerivation(const DerivationVec& derivations);

class DerivedTypeSymbol : public TypeSymbol
{
public:
    DerivedTypeSymbol(const Span& span_, const std::u32string& name_);
    DerivedTypeSymbol(const Span& span_, const std::u32string& name_, TypeSymbol* baseType_, const TypeDerivationRec& derivationRec_);
    std::string TypeString() const override { return "derived_type"; }
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol_, int index) override;
    const TypeSymbol* BaseType() const override { return baseType; }
    TypeSymbol* BaseType() override { return baseType; }
    TypeSymbol* PlainType() override;
    llvm::Type* IrType(Emitter& emitter) const override { return nullptr; } // todo
    bool IsReferenceType() const override;
    const TypeDerivationRec& DerivationRec() const { return derivationRec; }
private:
    TypeSymbol* baseType;
    TypeDerivationRec derivationRec;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_DERIVED_TYPE_SYMBOL_INCLUDED
