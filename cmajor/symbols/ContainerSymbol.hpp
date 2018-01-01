// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_CONTAINER_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_CONTAINER_SYMBOL_INCLUDED
#include <cmajor/symbols/Symbol.hpp>
#include <cmajor/symbols/Scope.hpp>

namespace cmajor { namespace symbols {

class FunctionGroupSymbol;
class ConceptGroupSymbol;
class ClassGroupTypeSymbol;

class ContainerSymbol : public Symbol
{
public:
    ContainerSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    virtual void AddMember(Symbol* member);
    void ComputeExportClosure() override;
    void Accept(SymbolCollector* collector) override;
    void Clear();
    std::string TypeString() const override { return "container"; }
    bool IsContainerSymbol() const override { return true; }
    const ContainerScope* GetContainerScope() const override { return &containerScope; }
    ContainerScope* GetContainerScope() override { return &containerScope; }
    const std::vector<std::unique_ptr<Symbol>>& Members() const { return members; }
    std::vector<std::unique_ptr<Symbol>>& Members() { return members; }
private:
    std::vector<std::unique_ptr<Symbol>> members;
    ContainerScope containerScope;
    FunctionGroupSymbol* MakeFunctionGroupSymbol(const std::u32string& groupName, const Span& span);
    ConceptGroupSymbol* MakeConceptGroupSymbol(const std::u32string& groupName, const Span& span);
    ClassGroupTypeSymbol* MakeClassGroupTypeSymbol(const std::u32string& groupName, const Span& span);
};

class DeclarationBlock : public ContainerSymbol
{
public:
    DeclarationBlock(const Span& span_, const std::u32string& name_);
    void AddMember(Symbol* member) override;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_CONTAINER_SYMBOL_INCLUDED
