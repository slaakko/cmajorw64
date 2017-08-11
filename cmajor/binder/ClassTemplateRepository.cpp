// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/ClassTemplateRepository.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/TypeResolver.hpp>
#include <cmajor/binder/TypeBinder.hpp>
#include <cmajor/binder/StatementBinder.hpp>
#include <cmajor/binder/BoundClass.hpp>
#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/binder/BoundStatement.hpp>
#include <cmajor/symbols/TemplateSymbol.hpp>
#include <cmajor/symbols/SymbolCreatorVisitor.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/util/Util.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::util;

ClassTemplateRepository::ClassTemplateRepository(BoundCompileUnit& boundCompileUnit_) : boundCompileUnit(boundCompileUnit_)
{
}

void ClassTemplateRepository::ResolveDefaultTemplateArguments(std::vector<TypeSymbol*>& templateArgumentTypes, ClassTypeSymbol* classTemplate, ContainerScope* containerScope, const Span& span)
{
    int n = classTemplate->TemplateParameters().size();
    int m = templateArgumentTypes.size();
    if (m == n) return;
    SymbolTable& symbolTable = boundCompileUnit.GetSymbolTable();
    Node* node = symbolTable.GetNodeNoThrow(classTemplate);
    if (!node)
    {
        classTemplate->ReadAstNodes();
        node = classTemplate->GetClassNode();
        Assert(node, "class node not read");
    }
    Assert(node->GetNodeType() == NodeType::classNode, "class node expected");
    ClassNode* classNode = static_cast<ClassNode*>(node);
    int numFileScopeAdded = 0;
    int nu = classTemplate->UsingNodes().Count();
    if (nu > 0)
    {
        FileScope* fileScope = new FileScope();
        for (int i = 0; i < nu; ++i)
        {
            Node* usingNode = classTemplate->UsingNodes()[i];
            if (usingNode->GetNodeType() == NodeType::namespaceImportNode)
            {
                NamespaceImportNode* namespaceImportNode = static_cast<NamespaceImportNode*>(usingNode);
                fileScope->InstallNamespaceImport(containerScope, namespaceImportNode);
            }
            else if (usingNode->GetNodeType() == NodeType::aliasNode)
            {
                AliasNode* aliasNode = static_cast<AliasNode*>(usingNode);
                fileScope->InstallAlias(containerScope, aliasNode);
            }
        }
        boundCompileUnit.AddFileScope(fileScope);
        ++numFileScopeAdded;
    }
    if (!classTemplate->Ns()->IsGlobalNamespace())
    {
        FileScope* primaryFileScope = new FileScope();
        primaryFileScope->AddContainerScope(classTemplate->Ns()->GetContainerScope());
        boundCompileUnit.AddFileScope(primaryFileScope);
        ++numFileScopeAdded;
    }
    ContainerScope resolveScope;
    resolveScope.SetParent(containerScope);
    std::vector<std::unique_ptr<BoundTemplateParameterSymbol>> boundTemplateParameters;
    for (int i = 0; i < n; ++i)
    {
        TemplateParameterSymbol* templateParameterSymbol = classTemplate->TemplateParameters()[i];
        BoundTemplateParameterSymbol* boundTemplateParameter = new BoundTemplateParameterSymbol(span, templateParameterSymbol->Name());
        boundTemplateParameters.push_back(std::unique_ptr<BoundTemplateParameterSymbol>(boundTemplateParameter));
        if (i < m)
        {
            boundTemplateParameter->SetType(templateArgumentTypes[i]);
            resolveScope.Install(boundTemplateParameter);
        }
        else
        {
            if (i >= classNode->TemplateParameters().Count())
            {
                throw Exception("too few template arguments", span);
            }
            Node* defaultTemplateArgumentNode = classNode->TemplateParameters()[i]->DefaultTemplateArgument();
            if (!defaultTemplateArgumentNode)
            {
                throw Exception("too few template arguments", span);
            }
            TypeSymbol* templateArgumentType = ResolveType(defaultTemplateArgumentNode, boundCompileUnit, &resolveScope, false);
            templateArgumentTypes.push_back(templateArgumentType);
        }
    }
    for (int i = 0; i < numFileScopeAdded; ++i)
    {
        boundCompileUnit.RemoveLastFileScope();
    }
}

void ClassTemplateRepository::BindClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization, ContainerScope* containerScope, const Span& span)
{
    if (classTemplateSpecialization->IsBound()) return;
    SymbolTable& symbolTable = boundCompileUnit.GetSymbolTable();
    ClassTypeSymbol* classTemplate = classTemplateSpecialization->GetClassTemplate();
    Node* node = symbolTable.GetNodeNoThrow(classTemplate);
    if (!node)
    {
        classTemplate->ReadAstNodes();
        node = classTemplate->GetClassNode();
        Assert(node, "class node not read");
    }
    Assert(node->GetNodeType() == NodeType::classNode, "class node expected");
    ClassNode* classNode = static_cast<ClassNode*>(node);
    std::unique_ptr<NamespaceNode> globalNs(new NamespaceNode(span, new IdentifierNode(span, U"")));
    NamespaceNode* currentNs = globalNs.get();
    CloneContext cloneContext;
    cloneContext.SetInstantiateClassNode();
    int nu = classTemplate->UsingNodes().Count();
    for (int i = 0; i < nu; ++i)
    {
        Node* usingNode = classTemplate->UsingNodes()[i];
        globalNs->AddMember(usingNode->Clone(cloneContext));
    }
    bool fileScopeAdded = false;
    if (!classTemplate->Ns()->IsGlobalNamespace())
    {
        FileScope* primaryFileScope = new FileScope();
        primaryFileScope->AddContainerScope(classTemplate->Ns()->GetContainerScope());
        boundCompileUnit.AddFileScope(primaryFileScope);
        fileScopeAdded = true;
        std::u32string fullNsName = classTemplate->Ns()->FullName();
        std::vector<std::u32string> nsComponents = Split(fullNsName, '.');
        for (const std::u32string& nsComponent : nsComponents)
        {
            NamespaceNode* nsNode = new NamespaceNode(span, new IdentifierNode(span, nsComponent));
            currentNs->AddMember(nsNode);
            currentNs = nsNode;
        }
    }
    ClassNode* classInstanceNode = static_cast<ClassNode*>(classNode->Clone(cloneContext));
    currentNs->AddMember(classInstanceNode);
    int n = classTemplate->TemplateParameters().size();
    int m = classTemplateSpecialization->TemplateArgumentTypes().size();
    if (n != m)
    {
        throw Exception("wrong number of template arguments", span);
    }
    bool markedExport = classTemplateSpecialization->MarkedExport();
    for (int i = 0; i < n; ++i)
    {
        TemplateParameterSymbol* templateParameter = classTemplate->TemplateParameters()[i];
        BoundTemplateParameterSymbol* boundTemplateParameter = new BoundTemplateParameterSymbol(span, templateParameter->Name());
        TypeSymbol* templateArgumentType = classTemplateSpecialization->TemplateArgumentTypes()[i];
        if (markedExport)
        {
            templateArgumentType->MarkExport();
        }
        boundTemplateParameter->SetType(templateArgumentType);
        classTemplateSpecialization->AddMember(boundTemplateParameter);
    }
    SymbolCreatorVisitor symbolCreatorVisitor(symbolTable);
    symbolCreatorVisitor.SetClassInstanceNode(classInstanceNode);
    symbolCreatorVisitor.SetClassTemplateSpecialization(classTemplateSpecialization);
    globalNs->Accept(symbolCreatorVisitor);
    TypeBinder typeBinder(boundCompileUnit);
    globalNs->Accept(typeBinder);
    StatementBinder statementBinder(boundCompileUnit);
    globalNs->Accept(statementBinder);
    classTemplateSpecialization->SetGlobalNs(std::move(globalNs));
    if (fileScopeAdded)
    {
        boundCompileUnit.RemoveLastFileScope();
    }
    bool templateArgumentsContainTemplateParameter = false;
    int ntp = classTemplateSpecialization->TemplateArgumentTypes().size();
    for (int i = 0; i < ntp; ++i)
    {
        TypeSymbol* templateTypeArgument = classTemplateSpecialization->TemplateArgumentTypes()[i];
        if (templateTypeArgument->ContainsTemplateParameter())
        {
            templateArgumentsContainTemplateParameter = true;
            break;
        }
    }
    if (!templateArgumentsContainTemplateParameter)
    {
        for (FunctionSymbol* virtualMemberFunction : classTemplateSpecialization->Vmt())
        {
            Instantiate(virtualMemberFunction, containerScope, span);
        }
        if (classTemplateSpecialization->Destructor())
        {
            Instantiate(classTemplateSpecialization->Destructor(), containerScope, span);
        }
    }
}

void ClassTemplateRepository::Instantiate(FunctionSymbol* memberFunction, ContainerScope* containerScope, const Span& span)
{
    if (instantiatedMemberFunctions.find(memberFunction) != instantiatedMemberFunctions.cend()) return;
    instantiatedMemberFunctions.insert(memberFunction);
    SymbolTable& symbolTable = boundCompileUnit.GetSymbolTable();
    Symbol* parent = memberFunction->Parent();
    Assert(parent->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol, "class template specialization expected");
    ClassTemplateSpecializationSymbol* classTemplateSpecialization = static_cast<ClassTemplateSpecializationSymbol*>(parent);
    ClassTypeSymbol* classTemplate = classTemplateSpecialization->GetClassTemplate();
    FileScope* fileScope = new FileScope();
    int nu = classTemplate->UsingNodes().Count();
    for (int i = 0; i < nu; ++i)
    {
        Node* usingNode = classTemplate->UsingNodes()[i];
        if (usingNode->GetNodeType() == NodeType::namespaceImportNode)
        {
            NamespaceImportNode* namespaceImportNode = static_cast<NamespaceImportNode*>(usingNode);
            fileScope->InstallNamespaceImport(containerScope, namespaceImportNode);
        }
        else if (usingNode->GetNodeType() == NodeType::aliasNode)
        {
            AliasNode* aliasNode = static_cast<AliasNode*>(usingNode);
            fileScope->InstallAlias(containerScope, aliasNode);
        }
    }
    if (!classTemplate->Ns()->IsGlobalNamespace())
    {
        fileScope->AddContainerScope(classTemplate->Ns()->GetContainerScope());
    }
    boundCompileUnit.AddFileScope(fileScope);
    Node* node = symbolTable.GetNode(memberFunction);
    Assert(node->IsFunctionNode(), "function node expected");
    FunctionNode* functionInstanceNode = static_cast<FunctionNode*>(node);
    Assert(functionInstanceNode->BodySource(), "body source expected");
    CloneContext cloneContext;
    functionInstanceNode->SetBody(static_cast<CompoundStatementNode*>(functionInstanceNode->BodySource()->Clone(cloneContext)));
    SymbolCreatorVisitor symbolCreatorVisitor(symbolTable);
    symbolTable.BeginContainer(memberFunction);
    functionInstanceNode->Body()->Accept(symbolCreatorVisitor);
    symbolTable.EndContainer();
    TypeBinder typeBinder(boundCompileUnit);
    typeBinder.SetContainerScope(memberFunction->GetContainerScope());
    functionInstanceNode->Body()->Accept(typeBinder);
    StatementBinder statementBinder(boundCompileUnit);
    std::unique_ptr<BoundClass> boundClass(new BoundClass(classTemplateSpecialization));
    statementBinder.SetCurrentClass(boundClass.get());
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(memberFunction));
    statementBinder.SetCurrentFunction(boundFunction.get());
    statementBinder.SetContainerScope(memberFunction->GetContainerScope());
    if (memberFunction->GetSymbolType() == SymbolType::constructorSymbol)
    {
        ConstructorSymbol* constructorSymbol = static_cast<ConstructorSymbol*>(memberFunction);
        Node* node = symbolTable.GetNode(memberFunction);
        Assert(node->GetNodeType() == NodeType::constructorNode, "constructor node expected");
        ConstructorNode* constructorNode = static_cast<ConstructorNode*>(node);
        statementBinder.SetCurrentConstructor(constructorSymbol, constructorNode);
    }
    else if (memberFunction->GetSymbolType() == SymbolType::destructorSymbol)
    {
        DestructorSymbol* destructorSymbol = static_cast<DestructorSymbol*>(memberFunction);
        Node* node = symbolTable.GetNode(memberFunction);
        Assert(node->GetNodeType() == NodeType::destructorNode, "destructor node expected");
        DestructorNode* destructorNode = static_cast<DestructorNode*>(node);
        statementBinder.SetCurrentDestructor(destructorSymbol, destructorNode);
    }
    else if (memberFunction->GetSymbolType() == SymbolType::memberFunctionSymbol)
    {
        MemberFunctionSymbol* memberFunctionSymbol = static_cast<MemberFunctionSymbol*>(memberFunction);
        Node* node = symbolTable.GetNode(memberFunction);
        Assert(node->GetNodeType() == NodeType::memberFunctionNode, "member function node expected");
        MemberFunctionNode* memberFunctionNode = static_cast<MemberFunctionNode*>(node);
        statementBinder.SetCurrentMemberFunction(memberFunctionSymbol, memberFunctionNode);
    }
    functionInstanceNode->Body()->Accept(statementBinder);
    BoundStatement* boundStatement = statementBinder.ReleaseStatement();
    Assert(boundStatement->GetBoundNodeType() == BoundNodeType::boundCompoundStatement, "bound compound statement expected");
    BoundCompoundStatement* compoundStatement = static_cast<BoundCompoundStatement*>(boundStatement);
    boundFunction->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    boundClass->AddMember(std::move(boundFunction));
    boundCompileUnit.AddBoundNode(std::move(boundClass));
    boundCompileUnit.RemoveLastFileScope();
}

} } // namespace cmajor::binder
