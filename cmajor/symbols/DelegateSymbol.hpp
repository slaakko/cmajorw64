// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_DELEGATE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_DELEGATE_SYMBOL_INCLUDED
#include <cmajor/symbols/ClassTypeSymbol.hpp>

namespace cmajor { namespace symbols {

class ParameterSymbol;

class DelegateTypeSymbol : public TypeSymbol
{
public:
    DelegateTypeSymbol(const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol_, int index) override;
    void AddMember(Symbol* member) override;
    std::string TypeString() const override { return "delegate"; }
    void SetSpecifiers(Specifiers specifiers);
    const TypeSymbol* ReturnType() const { return returnType; }
    void SetReturnType(TypeSymbol* returnType_) { returnType = returnType_; }
    llvm::Type* IrType(Emitter& emitter) override { return nullptr; } // todo
private:
    TypeSymbol* returnType;
    std::vector<ParameterSymbol*> parameters;
};

class ClassDelegateTypeSymbol : public ClassTypeSymbol
{
public:
    ClassDelegateTypeSymbol(const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol_, int index) override;
    void AddMember(Symbol* member) override;
    std::string TypeString() const override { return "class_delegate"; }
    bool IsClassTypeSymbol() const override { return false; }
    void SetSpecifiers(Specifiers specifiers);
    const TypeSymbol* ReturnType() const { return returnType; }
    void SetReturnType(TypeSymbol* returnType_) { returnType = returnType_; }
    llvm::Type* IrType(Emitter& emitter) override { return nullptr; } // todo
private:
    TypeSymbol* returnType;
    std::vector<ParameterSymbol*> parameters;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_DELEGATE_SYMBOL_INCLUDED
