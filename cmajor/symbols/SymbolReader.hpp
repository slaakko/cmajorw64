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
class ArrayTypeSymbol;
class DerivedTypeSymbol;
class ClassTemplateSpecializationSymbol;
class SymbolTable;
class Module;
class FunctionSymbol;
class ClassTypeSymbol;
class ParameterSymbol;

class SymbolReader
{
public:
    SymbolReader(const std::string& fileName_);
    AstReader& GetAstReader() { return astReader; }
    BinaryReader& GetBinaryReader() { return astReader.GetBinaryReader(); }
    Symbol* ReadSymbol(Symbol* parent);
    ArrayTypeSymbol* ReadArrayTypeSymbol(Symbol* parent);
    DerivedTypeSymbol* ReadDerivedTypeSymbol(Symbol* parent);
    ClassTemplateSpecializationSymbol* ReadClassTemplateSpecializationSymbol(Symbol* parent);
    ParameterSymbol* ReadParameterSymbol(Symbol* parent);
    void SetSymbolTable(SymbolTable* symbolTable_) { symbolTable = symbolTable_; }
    void SetModule(Module* module_) { module = module_; }
    void AddConversion(FunctionSymbol* conversion);
    const std::vector<FunctionSymbol*>& Conversions() const { return conversions; }
    void AddClassType(ClassTypeSymbol* classType);
    const std::vector<ClassTypeSymbol*>& ClassTypes() const { return classTypes; }
    void AddClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization);
    const std::vector<ClassTemplateSpecializationSymbol*>& ClassTemplateSpecializations() const { return classTemplateSpecializations; }
    bool SetProjectBit() const { return setProjectBit; }
    void SetProjectBitForSymbols() { setProjectBit = true; }
private:
    AstReader astReader;
    SymbolTable* symbolTable;
    Module* module;
    std::vector<FunctionSymbol*> conversions;
    std::vector<ClassTypeSymbol*> classTypes;
    std::vector<ClassTemplateSpecializationSymbol*> classTemplateSpecializations;
    bool setProjectBit;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_SYMBOL_READER_INCLUDED
