// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/InlineFunctionRepository.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/TypeBinder.hpp>
#include <cmajor/binder/StatementBinder.hpp>
#include <cmajor/binder/BoundClass.hpp>
#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/binder/BoundStatement.hpp>
#include <cmajor/symbols/SymbolCreatorVisitor.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/util/Util.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::util;

InlineFunctionRepository::InlineFunctionRepository(BoundCompileUnit& boundCompileUnit_) : boundCompileUnit(boundCompileUnit_)
{
}

InlineFunctionRepository::~InlineFunctionRepository()
{
    for (FunctionSymbol* inlineFunctionSymbol : instantiatedInlineFunctions)
    {
        if (inlineFunctionSymbol->IsTemplateSpecialization())
        {
            FunctionGroupSymbol* functionGroup = inlineFunctionSymbol->FunctionGroup();
            if (functionGroup)
            {
                functionGroup->RemoveFunction(inlineFunctionSymbol);
            }
        }
    }
}

void InlineFunctionRepository::Instantiate(FunctionSymbol* inlineFunction, ContainerScope* containerScope, const Span& span)
{
    if (inlineFunction->GetCompileUnit() == boundCompileUnit.GetCompileUnitNode()) return;
    if (instantiatedInlineFunctions.find(inlineFunction) != instantiatedInlineFunctions.cend()) return;
    instantiatedInlineFunctions.insert(inlineFunction);
    SymbolTable& symbolTable = boundCompileUnit.GetSymbolTable();
    Node* node = symbolTable.GetNodeNoThrow(inlineFunction);
    if (!node)
    {
        //inlineFunction->ReadAstNodes();
        node = inlineFunction->GetFunctionNode();
        Assert(node, "function node not read");
    }
    FunctionNode* functionNode = static_cast<FunctionNode*>(node);
    std::unique_ptr<NamespaceNode> globalNs(new NamespaceNode(inlineFunction->GetSpan(), new IdentifierNode(inlineFunction->GetSpan(), U"")));
    NamespaceNode* currentNs = globalNs.get();
    CloneContext cloneContext;
    cloneContext.SetInstantiateFunctionNode();
    bool fileScopeAdded = false;
    int n = inlineFunction->UsingNodes().Count();
    if (!inlineFunction->Ns()->IsGlobalNamespace() || n > 0)
    {
        FileScope* primaryFileScope = new FileScope(&boundCompileUnit.GetModule());
        if (!inlineFunction->Ns()->IsGlobalNamespace())
        {
            primaryFileScope->AddContainerScope(inlineFunction->Ns()->GetContainerScope());
        }
        for (int i = 0; i < n; ++i)
        {
            Node* usingNode = inlineFunction->UsingNodes()[i];
            if (usingNode->GetNodeType() == NodeType::namespaceImportNode)
            {
                primaryFileScope->InstallNamespaceImport(containerScope, static_cast<NamespaceImportNode*>(usingNode));
            }
            else if (usingNode->GetNodeType() == NodeType::aliasNode)
            {
                primaryFileScope->InstallAlias(containerScope, static_cast<AliasNode*>(usingNode));
            }
        }
        boundCompileUnit.AddFileScope(primaryFileScope);
        fileScopeAdded = true;
        std::u32string fullNsName = inlineFunction->Ns()->FullName();
        std::vector<std::u32string> nsComponents = Split(fullNsName, '.');
        for (const std::u32string& nsComponent : nsComponents)
        {
            NamespaceNode* nsNode = new NamespaceNode(inlineFunction->GetSpan(), new IdentifierNode(inlineFunction->GetSpan(), nsComponent));
            currentNs->AddMember(nsNode);
            currentNs = nsNode;
        }
    }
    FunctionNode* functionInstanceNode = static_cast<FunctionNode*>(functionNode->Clone(cloneContext));
    currentNs->AddMember(functionInstanceNode);
    symbolTable.SetCurrentCompileUnit(boundCompileUnit.GetCompileUnitNode());
    SymbolCreatorVisitor symbolCreatorVisitor(symbolTable);
    symbolTable.BeginContainer(inlineFunction);
    functionInstanceNode->Body()->Accept(symbolCreatorVisitor);
    symbolTable.EndContainer();
    TypeBinder typeBinder(boundCompileUnit);
    typeBinder.SetContainerScope(inlineFunction->GetContainerScope());
    functionInstanceNode->Body()->Accept(typeBinder);
    StatementBinder statementBinder(boundCompileUnit);
    std::unique_ptr<BoundClass> boundClass;
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(&boundCompileUnit.GetModule(), inlineFunction));
    statementBinder.SetCurrentFunction(boundFunction.get());
    if (inlineFunction->GetSymbolType() == SymbolType::constructorSymbol ||
        inlineFunction->GetSymbolType() == SymbolType::memberFunctionSymbol)
    {
        boundClass.reset(new BoundClass(&boundCompileUnit.GetModule(), static_cast<ClassTypeSymbol*>(inlineFunction->Parent())));
        statementBinder.SetCurrentClass(boundClass.get());
        if (inlineFunction->GetSymbolType() == SymbolType::constructorSymbol)
        {
            ConstructorSymbol* constructorSymbol = static_cast<ConstructorSymbol*>(inlineFunction);
            Assert(node->GetNodeType() == NodeType::constructorNode, "constructor node expected");
            ConstructorNode* constructorNode = static_cast<ConstructorNode*>(node);
            statementBinder.SetCurrentConstructor(constructorSymbol, constructorNode);
        }
        else if (inlineFunction->GetSymbolType() == SymbolType::memberFunctionSymbol)
        {
            MemberFunctionSymbol* memberFunctionSymbol = static_cast<MemberFunctionSymbol*>(inlineFunction);
            Assert(node->GetNodeType() == NodeType::memberFunctionNode, "member function node expected");
            MemberFunctionNode* memberFunctionNode = static_cast<MemberFunctionNode*>(node);
            statementBinder.SetCurrentMemberFunction(memberFunctionSymbol, memberFunctionNode);
        }
    }
    statementBinder.SetContainerScope(inlineFunction->GetContainerScope());
    functionInstanceNode->Body()->Accept(statementBinder);
    BoundStatement* boundStatement = statementBinder.ReleaseStatement();
    Assert(boundStatement->GetBoundNodeType() == BoundNodeType::boundCompoundStatement, "bound compound statement expected");
    BoundCompoundStatement* compoundStatement = static_cast<BoundCompoundStatement*>(boundStatement);
    boundFunction->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    if (boundClass)
    {
        boundClass->AddMember(std::move(boundFunction));
        boundCompileUnit.AddBoundNode(std::move(boundClass));
    }
    else
    {
        boundCompileUnit.AddBoundNode(std::move(boundFunction));
    }
    if (fileScopeAdded)
    {
        boundCompileUnit.RemoveLastFileScope();
    }
    inlineFunction->SetGlobalNs(std::move(globalNs));
}

} } // namespace cmajor::binder
