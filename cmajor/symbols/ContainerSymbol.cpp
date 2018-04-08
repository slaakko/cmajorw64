// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/ContainerSymbol.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/ConceptSymbol.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

ContainerSymbol::ContainerSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) : Symbol(symbolType_, span_, name_)
{
    containerScope.SetContainer(this);
}

void ContainerSymbol::Write(SymbolWriter& writer)
{
    Symbol::Write(writer);
    std::vector<Symbol*> exportSymbols;
    for (const std::unique_ptr<Symbol>& member : members)
    {
        if (member->IsExportSymbol())
        {
            exportSymbols.push_back(member.get());
        }
    }
    uint32_t n = uint32_t(exportSymbols.size());
    writer.GetBinaryWriter().WriteEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        writer.Write(exportSymbols[i]);
    }
}

void ContainerSymbol::Read(SymbolReader& reader)
{
    Symbol::Read(reader);
    uint32_t n = reader.GetBinaryReader().ReadEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        Symbol* symbol = reader.ReadSymbol(this);
        AddMember(symbol);
    }
}

void ContainerSymbol::AddMember(Symbol* member)
{
    member->SetSymbolTable(GetSymbolTable());
    if (GetModule())
    {
        member->SetModule(GetModule());
    }
    member->SetParent(this);
    members.push_back(std::unique_ptr<Symbol>(member));
    if (member->IsFunctionSymbol())
    {
        FunctionSymbol* functionSymbol = static_cast<FunctionSymbol*>(member);
        FunctionGroupSymbol* functionGroupSymbol = MakeFunctionGroupSymbol(functionSymbol->GroupName(), functionSymbol->GetSpan());
        functionGroupSymbol->AddFunction(functionSymbol);
        functionSymbol->GetContainerScope()->SetParent(GetContainerScope());
    }
    else if (member->GetSymbolType() == SymbolType::conceptSymbol)
    {
        ConceptSymbol* conceptSymbol = static_cast<ConceptSymbol*>(member);
        ConceptGroupSymbol* conceptGroupSymbol = MakeConceptGroupSymbol(conceptSymbol->GroupName(), conceptSymbol->GetSpan());
        conceptGroupSymbol->AddConcept(conceptSymbol);
        conceptSymbol->GetContainerScope()->SetParent(GetContainerScope());
    }
    else if (member->GetSymbolType() == SymbolType::classTypeSymbol || member->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        ClassTypeSymbol* classTypeSymbol = static_cast<ClassTypeSymbol*>(member);
        ClassGroupTypeSymbol* classGroupTypeSymbol = MakeClassGroupTypeSymbol(classTypeSymbol->GroupName(), classTypeSymbol->GetSpan());
        classGroupTypeSymbol->AddClass(classTypeSymbol);
        classTypeSymbol->GetContainerScope()->SetParent(GetContainerScope());
    }
    else
    {
        containerScope.Install(member);
    }
}

void ContainerSymbol::ComputeExportClosure()
{
    if (IsProject())
    {
        for (std::unique_ptr<Symbol>& member : members)
        {
            if (!member->ExportComputed())
            {
                member->SetExportComputed();
                member->ComputeExportClosure();
            }
        }
    }
}

void ContainerSymbol::Accept(SymbolCollector* collector)
{
    if (IsProject())
    {
        for (std::unique_ptr<Symbol>& member : members)
        {
            member->Accept(collector);
        }
    }
}

void ContainerSymbol::Clear()
{
    containerScope.Clear();
    members.clear();
}

FunctionGroupSymbol* ContainerSymbol::MakeFunctionGroupSymbol(const std::u32string& groupName, const Span& span)
{
    Symbol* symbol = containerScope.Lookup(groupName);
    if (!symbol)
    {
        FunctionGroupSymbol* functionGroupSymbol = new FunctionGroupSymbol(span, groupName);
        functionGroupSymbol->SetSymbolTable(GetSymbolTable());
        functionGroupSymbol->SetModule(GetModule());
        AddMember(functionGroupSymbol);
        return functionGroupSymbol;
    }
    if (symbol->GetSymbolType() == SymbolType::functionGroupSymbol)
    {
        return static_cast<FunctionGroupSymbol*>(symbol);
    }
    else
    {
        throw Exception(GetModule(), "name of symbol '" + ToUtf8(symbol->FullName()) + "' conflicts with a function group '" + ToUtf8(groupName) + "'", symbol->GetSpan(), span);
    }
}

ConceptGroupSymbol* ContainerSymbol::MakeConceptGroupSymbol(const std::u32string& groupName, const Span& span)
{
    Symbol* symbol = containerScope.Lookup(groupName);
    if (!symbol)
    {
        ConceptGroupSymbol* conceptGroupSymbol = new ConceptGroupSymbol(span, groupName);
        conceptGroupSymbol->SetSymbolTable(GetSymbolTable());
        conceptGroupSymbol->SetModule(GetModule());
        AddMember(conceptGroupSymbol);
        return conceptGroupSymbol;
    }
    if (symbol->GetSymbolType() == SymbolType::conceptGroupSymbol)
    {
        return static_cast<ConceptGroupSymbol*>(symbol);
    }
    else
    {
        throw Exception(GetModule(), "name of symbol '" + ToUtf8(symbol->FullName()) + "' conflicts with a concept group '" + ToUtf8(groupName) + "'", symbol->GetSpan(), span);
    }
}

ClassGroupTypeSymbol* ContainerSymbol::MakeClassGroupTypeSymbol(const std::u32string& groupName, const Span& span)
{
    Symbol* symbol = containerScope.Lookup(groupName);
    if (!symbol)
    {
        ClassGroupTypeSymbol* classGroupTypeSymbol = new ClassGroupTypeSymbol(span, groupName);
        classGroupTypeSymbol->SetSymbolTable(GetSymbolTable());
        classGroupTypeSymbol->SetModule(GetModule());
        AddMember(classGroupTypeSymbol);
        return classGroupTypeSymbol;
    }
    if (symbol->GetSymbolType() == SymbolType::classGroupTypeSymbol)
    {
        return static_cast<ClassGroupTypeSymbol*>(symbol);
    }
    else
    {
        throw Exception(GetModule(), "name of symbol '" + ToUtf8(symbol->FullName()) + "' conflicts with a class group '" + ToUtf8(groupName) + "'", symbol->GetSpan(), span);
    }
}

DeclarationBlock::DeclarationBlock(const Span& span_, const std::u32string& name_) : ContainerSymbol(SymbolType::declarationBlock, span_, name_)
{
}

void DeclarationBlock::AddMember(Symbol* member) 
{
    ContainerSymbol::AddMember(member);
    if (member->GetSymbolType() == SymbolType::localVariableSymbol)
    {
        FunctionSymbol* fun = Function();
        if (fun)
        {
            fun->AddLocalVariable(static_cast<LocalVariableSymbol*>(member));
        }
    }
}

} } // namespace cmajor::symbols
