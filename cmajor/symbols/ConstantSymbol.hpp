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
    void EmplaceType(TypeSymbol* typeSymbol, int index) override;
    void Accept(SymbolCollector* collector) override;
    void Dump(CodeFormatter& formatter) override;
    std::string TypeString() const override { return "constant"; }
    std::string Syntax() const override;
    void SetSpecifiers(Specifiers specifiers);
    bool Evaluating() const { return evaluating; }
    void SetEvaluating() { evaluating = true; }
    void ResetEvaluating() { evaluating = false; }
    const TypeSymbol* GetType() const { return type; }
    TypeSymbol* GetType() { return type; }
    void SetType(TypeSymbol* typeSymbol) { type = typeSymbol; }
    void SetValue(Value* value_);
    Value* GetValue();
    llvm::Value* ArrayIrObject(Emitter& emitter, bool create);
    llvm::Value* StructureIrObject(Emitter& emitter, bool create);
private:
    TypeSymbol* type;
    std::unique_ptr<Value> value;
    bool evaluating;
    uint32_t sizeOfValue;
    uint32_t valuePos;
    std::string filePathReadFrom;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_CONSTANT_SYMBOL_INCLUDED
