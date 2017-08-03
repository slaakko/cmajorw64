// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_TEMPLATE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_TEMPLATE_SYMBOL_INCLUDED
#include <cmajor/symbols/TypeSymbol.hpp>

namespace cmajor { namespace symbols {

class TemplateParameterSymbol : public TypeSymbol
{
public:
    TemplateParameterSymbol(const Span& span_, const std::u32string& name_);
    std::u32string FullName() const override { return Name(); }
    llvm::Type* IrType(Emitter& emitter) override { return nullptr; }
    TypeSymbol* Unify(TypeSymbol* type, const Span& span) override;
};

class BoundTemplateParameterSymbol : public Symbol
{
public:
    BoundTemplateParameterSymbol(const Span& span_, const std::u32string& name_);
    std::u32string FullName() const override { return Name(); }
    bool IsBoundTemplateParameterSymbol() const override { return true; }
    TypeSymbol* GetType() const { return type; }
    void SetType(TypeSymbol* type_) { type = type_; }
private:
    TypeSymbol* type;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_TEMPLATE_SYMBOL_INCLUDED
