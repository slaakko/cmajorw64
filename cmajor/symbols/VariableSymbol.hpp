// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_VARIABLE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_VARIABLE_SYMBOL_INCLUDED
#include <cmajor/symbols/Symbol.hpp>

namespace cmajor { namespace symbols {

class VariableSymbol : public Symbol
{
public:
    VariableSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    const TypeSymbol* GetType() const { return typeSymbol; }
    void SetType(TypeSymbol* typeSymbol_) { typeSymbol = typeSymbol_; }
    void EmplaceType(TypeSymbol* typeSymbol_, int index) override;
private:
    TypeSymbol* typeSymbol;
};

class ParameterSymbol : public VariableSymbol
{
public:    
    ParameterSymbol(const Span& span_, const std::u32string& name_);
};

class LocalVariableSymbol : public VariableSymbol
{
public:     
    LocalVariableSymbol(const Span& span_, const std::u32string& name_);
};

class MemberVariableSymbol : public VariableSymbol
{
public:
    MemberVariableSymbol(const Span& span_, const std::u32string& name_);
    void SetSpecifiers(Specifiers specifiers);
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_VARIABLE_SYMBOL_INCLUDED
