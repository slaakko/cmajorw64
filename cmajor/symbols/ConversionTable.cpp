// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/ConversionTable.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>

namespace cmajor { namespace symbols {

void ConversionTable::AddConversion(FunctionSymbol* conversion)
{
    ConversionTableEntry entry(conversion->ConversionSourceType()->PlainType(conversion->GetSpan()), conversion->ConversionTargetType()->PlainType(conversion->GetSpan()));
    conversionMap.insert(std::make_pair(entry, conversion));
}

void ConversionTable::Add(ConversionTable& that)
{
    for (std::unique_ptr<FunctionSymbol>& conversion : that.generatedConversions)
    {
        generatedConversions.push_back(std::move(conversion));
    }
    for (const auto& p : that.conversionMap)
    {
        conversionMap[p.first] = p.second;
    }
}

FunctionSymbol* ConversionTable::GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType, const Span& span) const
{
    TypeSymbol* sourcePlainType = sourceType->PlainType(span);
    TypeSymbol* targetPlainType = targetType->PlainType(span);
    ConversionTableEntry entry(sourcePlainType, targetPlainType);
    auto it = conversionMap.find(entry);
    if (it != conversionMap.cend())
    {
        return it->second;
    }
    return nullptr;
}

void ConversionTable::AddGeneratedConversion(std::unique_ptr<FunctionSymbol>&& generatedConversion)
{
    generatedConversions.push_back(std::move(generatedConversion));
}

} } // namespace cmajor::symbols