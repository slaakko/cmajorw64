// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_INTERFACE_TYPE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_INTERFACE_TYPE_SYMBOL_INCLUDED
#include <cmajor/symbols/TypeSymbol.hpp>

namespace cmajor { namespace symbols {

class MemberFunctionSymbol;

class InterfaceTypeSymbol : public TypeSymbol
{
public:
    InterfaceTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "interface type"; }
    void AddMember(Symbol* member) override;
    void SetSpecifiers(Specifiers specifiers);
    llvm::Type* IrType(Emitter& emitter) override { return nullptr; } // todo
    const std::vector<MemberFunctionSymbol*>& MemberFunctions() const { return memberFunctions; }
private:
    std::vector<MemberFunctionSymbol*> memberFunctions;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_INTERFACE_TYPE_SYMBOL_INCLUDED
