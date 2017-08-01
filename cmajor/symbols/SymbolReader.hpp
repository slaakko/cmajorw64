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
class ClassTypeSymbol;

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
    void AddClassType(ClassTypeSymbol* classType);
    const std::vector<ClassTypeSymbol*>& ClassTypes() const { return classTypes; }
private:
    AstReader astReader;
    SymbolTable* symbolTable;
    Module* module;
    std::vector<FunctionSymbol*> conversions;
    std::vector<ClassTypeSymbol*> classTypes;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_SYMBOL_READER_INCLUDED
