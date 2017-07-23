// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_CONVERSION_TABLE_INCLUDED
#define CMAJOR_SYMBOLS_CONVERSION_TABLE_INCLUDED
#include <cmajor/symbols/TypeSymbol.hpp>

namespace cmajor { namespace symbols {

struct ConversionTableEntry
{
    ConversionTableEntry(TypeSymbol* sourceType_, TypeSymbol* targetType_) : sourceType(sourceType_), targetType(targetType_) {}
    TypeSymbol* sourceType;
    TypeSymbol* targetType;
};

inline bool operator==(const ConversionTableEntry& left, const ConversionTableEntry& right)
{
    return TypesEqual(left.sourceType, right.sourceType) && TypesEqual(left.targetType, right.targetType);
}

struct ConversionTableEntryHash
{
    size_t operator()(const ConversionTableEntry& entry) const { return std::hash<std::u32string>()(entry.sourceType->Name()) ^ std::hash<std::u32string>()(entry.targetType->Name()); }
};

class ConversionTable
{
public:
    bool IsEmpty() const { return conversionMap.empty(); }
    void AddConversion(FunctionSymbol* conversion);
    FunctionSymbol* GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType) const;
private:
    std::unordered_map<ConversionTableEntry, FunctionSymbol*, ConversionTableEntryHash> conversionMap;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_CONVERSION_TABLE_INCLUDED
