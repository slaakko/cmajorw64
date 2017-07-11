// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_CONSTANT_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_CONSTANT_SYMBOL_INCLUDED
#include <cmajor/symbols/Symbol.hpp>
#include <cmajor/symbols/Value.hpp>

namespace cmajor { namespace symbols {

class ConstantSymbol : public Symbol
{
public:
    ConstantSymbol(const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol_, int index) override;
    void SetSpecifiers(Specifiers specifiers);
    const TypeSymbol* GetType() const { return typeSymbol; }
    void SetType(TypeSymbol* typeSymbol_) { typeSymbol = typeSymbol_; }
private:
    TypeSymbol* typeSymbol;
    std::unique_ptr<Value> value;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_CONSTANT_SYMBOL_INCLUDED
