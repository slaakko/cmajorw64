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

typedef llvm::SmallVector<Derivation, 8> DerivationVec;
typedef llvm::SmallVector<uint64_t, 4> ArrayDimensionVec;

class DerivedTypeSymbol : public TypeSymbol
{
public:
    DerivedTypeSymbol(const Span& span_, const std::u32string& name_);
private:
    DerivationVec derivations;
    ArrayDimensionVec arrayDimensions;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_DERIVED_TYPE_SYMBOL_INCLUDED
