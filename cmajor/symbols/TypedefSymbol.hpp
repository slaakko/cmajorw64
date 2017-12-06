// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_TYPEDEF_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_TYPEDEF_SYMBOL_INCLUDED
#include <cmajor/symbols/Symbol.hpp>

namespace cmajor { namespace symbols {

class TypedefSymbol : public Symbol
{
public:
    TypedefSymbol(const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol, int index) override;
    bool IsExportSymbol() const override;
    void ComputeExportClosure() override;
    void Accept(SymbolCollector* collector) override;
    void Dump(CodeFormatter& formatter) override;
    std::string TypeString() const override { return "typedef"; }
    std::string Syntax() const override;
    void SetSpecifiers(Specifiers specifiers);
    const TypeSymbol* GetType() const { return type; }
    TypeSymbol* GetType() { return type; }
    void SetType(TypeSymbol* typeSymbol) { type = typeSymbol; }
private:
    TypeSymbol* type;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_TYPEDEF_SYMBOL_INCLUDED
