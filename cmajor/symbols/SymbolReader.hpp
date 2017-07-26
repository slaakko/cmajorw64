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
class DerivedTypeSymbol;
class SymbolTable;
class Module;
class FunctionSymbol;

class SymbolReader
{
public:
    SymbolReader(const std::string& fileName_);
    AstReader& GetAstReader() { return astReader; }
    BinaryReader& GetBinaryReader() { return astReader.GetBinaryReader(); }
    Symbol* ReadSymbol();
    DerivedTypeSymbol* ReadDerivedTypeSymbol();
    void SetSymbolTable(SymbolTable* symbolTable_) { symbolTable = symbolTable_; }
    void SetModule(Module* module_) { module = module_; }
    void AddConversion(FunctionSymbol* conversion);
    const std::vector<FunctionSymbol*>& Conversions() const { return conversions; }
private:
    AstReader astReader;
    SymbolTable* symbolTable;
    Module* module;
    std::vector<FunctionSymbol*> conversions;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_SYMBOL_READER_INCLUDED
