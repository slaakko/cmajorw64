// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/ConversionTable.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>

namespace cmajor { namespace symbols {

void ConversionTable::AddConversion(FunctionSymbol* conversion)
{
    ConversionTableEntry entry(conversion->ConversionSourceType(), conversion->ConversionTargetType());
    conversionMap.insert(std::make_pair(entry, conversion));
}

FunctionSymbol* ConversionTable::GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType) const
{
    ConversionTableEntry entry(sourceType, targetType);
    auto it = conversionMap.find(entry);
    if (it != conversionMap.cend())
    {
        return it->second;
    }
    return nullptr;
}

} } // namespace cmajor::symbols