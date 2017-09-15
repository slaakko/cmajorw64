// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/DerivedTypeSymbol.hpp>
#include <cmajor/symbols/ClassTemplateSpecializationSymbol.hpp>

namespace cmajor { namespace symbols {

SymbolReader::SymbolReader(const std::string& fileName_) : astReader(fileName_), symbolTable(nullptr), module(nullptr)
{
}

Symbol* SymbolReader::ReadSymbol(Symbol* parent)
{
    SymbolType symbolType = static_cast<SymbolType>(astReader.GetBinaryReader().ReadByte());
    Span span = astReader.ReadSpan();
    std::u32string name = astReader.GetBinaryReader().ReadUtf32String();
    Symbol* symbol = SymbolFactory::Instance().CreateSymbol(symbolType, span, name);
    symbol->SetSymbolTable(symbolTable);
    symbol->SetModule(module);
    symbol->SetParent(parent);
    symbol->Read(*this);
    return symbol;
}

DerivedTypeSymbol* SymbolReader::ReadDerivedTypeSymbol(Symbol* parent)
{
    Symbol* symbol = ReadSymbol(parent);
    if (symbol->GetSymbolType() == SymbolType::derivedTypeSymbol)
    {
        return static_cast<DerivedTypeSymbol*>(symbol);
    }
    else
    {
        throw std::runtime_error("derived type symbol expected");
    }
}

ClassTemplateSpecializationSymbol* SymbolReader::ReadClassTemplateSpecializationSymbol(Symbol* parent)
{
    Symbol* symbol = ReadSymbol(parent);
    if (symbol->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        return static_cast<ClassTemplateSpecializationSymbol*>(symbol);
    }
    else
    {
        throw std::runtime_error("class template specialization symbol expected");
    }
}

ParameterSymbol* SymbolReader::ReadParameterSymbol(Symbol* parent)
{
    Symbol* symbol = ReadSymbol(parent);
    if (symbol->GetSymbolType() == SymbolType::parameterSymbol)
    {
        return static_cast<ParameterSymbol*>(symbol);
    }
    else
    {
        throw std::runtime_error("parameter symbol expected");
    }
}

void SymbolReader::AddConversion(FunctionSymbol* conversion)
{
    conversions.push_back(conversion);
}

void SymbolReader::AddClassType(ClassTypeSymbol* classType)
{
    classTypes.push_back(classType);
}

void SymbolReader::AddClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization)
{
    classTemplateSpecializations.push_back(classTemplateSpecialization);
}

} } // namespace cmajor::symbols