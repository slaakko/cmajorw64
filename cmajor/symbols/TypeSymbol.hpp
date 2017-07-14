// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_TYPE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_TYPE_SYMBOL_INCLUDED
#include <cmajor/symbols/ContainerSymbol.hpp>
#include <cmajor/ir/Emitter.hpp>
#include <llvm/IR/Type.h>

namespace cmajor { namespace symbols {

using namespace cmajor::ir;

class TypeSymbol : public ContainerSymbol
{
public:
    TypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    bool IsTypeSymbol() const override { return true; }
    virtual bool IsInComplete() const { return false; }
    std::string TypeString() const override { return "type"; }
    void SetTypeId(uint32_t typeId_) { typeId = typeId_; }
    uint32_t TypeId() const { Assert(typeId != 0, "type id not initialized");  return typeId; }
    virtual llvm::Type* IrType(Emitter& emitter) const = 0;
private:
    uint32_t typeId;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_TYPE_SYMBOL_INCLUDED
