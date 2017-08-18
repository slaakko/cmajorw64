// =================================
// Copyright (c) 2017 Seppo Laakko
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

void InlineFunctionRepository::Instantiate(FunctionSymbol* inlineFunction, ContainerScope* containerScope, const Span& span)
{
    if (inlineFunction->GetCompileUnit() == boundCompileUnit.GetCompileUnitNode()) return;
    if (instantiatedInlineFunctions.find(inlineFunction) != instantiatedInlineFunctions.cend()) return;
    instantiatedInlineFunctions.insert(inlineFunction);
    SymbolTable& symbolTable = boundCompileUnit.GetSymbolTable();
    Node* node = symbolTable.GetNodeNoThrow(inlineFunction);
    if (!node)
    {
        inlineFunction->ReadAstNodes();
        node = inlineFunction->GetFunctionNode();
        Assert(node, "function node not read");
    }
    Assert(node->GetNodeType() == NodeType::functionNode, "function node expected");
    FunctionNode* functionNode = static_cast<FunctionNode*>(node);
    std::unique_ptr<NamespaceNode> globalNs(new NamespaceNode(span, new IdentifierNode(span, U"")));
    NamespaceNode* currentNs = globalNs.get();
    CloneContext cloneContext;
    cloneContext.SetInstantiateFunctionNode();
    int n = inlineFunction->UsingNodes().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* usingNode = inlineFunction->UsingNodes()[i];
        globalNs->AddMember(usingNode->Clone(cloneContext));
    }
    bool fileScopeAdded = false;
    if (!inlineFunction->Ns()->IsGlobalNamespace())
    {
        FileScope* primaryFileScope = new FileScope();
        primaryFileScope->AddContainerScope(inlineFunction->Ns()->GetContainerScope());
        boundCompileUnit.AddFileScope(primaryFileScope);
        fileScopeAdded = true;
        std::u32string fullNsName = inlineFunction->Ns()->FullName();
        std::vector<std::u32string> nsComponents = Split(fullNsName, '.');
        for (const std::u32string& nsComponent : nsComponents)
        {
            NamespaceNode* nsNode = new NamespaceNode(span, new IdentifierNode(span, nsComponent));
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
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(inlineFunction));
    statementBinder.SetCurrentFunction(boundFunction.get());
    if (inlineFunction->GetSymbolType() == SymbolType::constructorSymbol ||
        inlineFunction->GetSymbolType() == SymbolType::memberFunctionSymbol)
    {
        boundClass.reset(new BoundClass(static_cast<ClassTypeSymbol*>(inlineFunction->Parent())));
        statementBinder.SetCurrentClass(boundClass.get());
        if (inlineFunction->GetSymbolType() == SymbolType::constructorSymbol)
        {
            ConstructorSymbol* constructorSymbol = static_cast<ConstructorSymbol*>(inlineFunction);
            Node* node = symbolTable.GetNode(inlineFunction);
            Assert(node->GetNodeType() == NodeType::constructorNode, "constructor node expected");
            ConstructorNode* constructorNode = static_cast<ConstructorNode*>(node);
            statementBinder.SetCurrentConstructor(constructorSymbol, constructorNode);
        }
        else if (inlineFunction->GetSymbolType() == SymbolType::memberFunctionSymbol)
        {
            MemberFunctionSymbol* memberFunctionSymbol = static_cast<MemberFunctionSymbol*>(inlineFunction);
            Node* node = symbolTable.GetNode(inlineFunction);
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
}

} } // namespace cmajor::binder
