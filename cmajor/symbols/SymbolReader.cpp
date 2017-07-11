// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/Symbol.hpp>

namespace cmajor { namespace symbols {

SymbolReader::SymbolReader(const std::string& fileName_) : astReader(fileName_), symbolTable(nullptr)
{
}

Symbol* SymbolReader::ReadSymbol()
{
    SymbolType symbolType = static_cast<SymbolType>(astReader.GetBinaryReader().ReadByte());
    Span span = astReader.ReadSpan();
    std::u32string name = astReader.GetBinaryReader().ReadUtf32String();
    Symbol* symbol = SymbolFactory::Instance().CreateSymbol(symbolType, span, name);
    symbol->SetSymbolTable(symbolTable);
    symbol->Read(*this);
    return symbol;
}

} } // namespace cmajor::symbols
