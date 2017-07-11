// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_SYMBOL_READER_INCLUDED
#define CMAJOR_SYMBOLS_SYMBOL_READER_INCLUDED
#include <cmajor/ast/AstReader.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::ast;

class Symbol;
class SymbolTable;

class SymbolReader
{
public:
    SymbolReader(const std::string& fileName_);
    AstReader& GetAstReader() { return astReader; }
    BinaryReader& GetBinaryReader() { return astReader.GetBinaryReader(); }
    Symbol* ReadSymbol();
    void SetSymbolTable(SymbolTable* symbolTable_) { symbolTable = symbolTable_; }
private:
    AstReader astReader;
    SymbolTable* symbolTable;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_SYMBOL_READER_INCLUDED
