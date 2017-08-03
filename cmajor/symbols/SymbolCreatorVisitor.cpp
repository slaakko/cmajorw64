// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/SymbolCreatorVisitor.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/ast/CompileUnit.hpp>
#include <cmajor/ast/Class.hpp>
#include <cmajor/ast/Interface.hpp>

namespace cmajor { namespace symbols {

SymbolCreatorVisitor::SymbolCreatorVisitor(SymbolTable& symbolTable_) : symbolTable(symbolTable_)
{
}

void SymbolCreatorVisitor::Visit(CompileUnitNode& compileUnitNode)
{
    compileUnitNode.GlobalNs()->Accept(*this);
}

void SymbolCreatorVisitor::Visit(NamespaceNode& namespaceNode)
{
    symbolTable.BeginNamespace(namespaceNode);
    NodeList<Node>& members = namespaceNode.Members();
    int n = members.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = members[i];
        member->Accept(*this);
    }
    symbolTable.EndNamespace();
}

void SymbolCreatorVisitor::Visit(FunctionNode& functionNode)
{
    symbolTable.BeginFunction(functionNode);
    int nt = functionNode.TemplateParameters().Count();
    for (int i = 0; i < nt; ++i)
    {
        symbolTable.AddTemplateParameter(*functionNode.TemplateParameters()[i]);
    }
    int n = functionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = functionNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    if (nt == 0)
    {
        if (functionNode.Body())
        {
            functionNode.Body()->Accept(*this);
        }
    }
    symbolTable.EndFunction();
}

void SymbolCreatorVisitor::Visit(ParameterNode& parameterNode)
{
    symbolTable.AddParameter(parameterNode);
}

void SymbolCreatorVisitor::Visit(ClassNode& classNode)
{
    symbolTable.BeginClass(classNode);
    int nt = classNode.TemplateParameters().Count();
    for (int i = 0; i < nt; ++i)
    {
        symbolTable.AddTemplateParameter(*classNode.TemplateParameters()[i]);
    }
    if (nt == 0)
    {
        int n = classNode.Members().Count();
        for (int i = 0; i < n; ++i)
        {
            Node* member = classNode.Members()[i];
            member->Accept(*this);
        }
    }
    symbolTable.EndClass();
}

void SymbolCreatorVisitor::Visit(StaticConstructorNode& staticConstructorNode)
{
    symbolTable.BeginStaticConstructor(staticConstructorNode);
    if (staticConstructorNode.Body())
    {
        staticConstructorNode.Body()->Accept(*this);
    }
    symbolTable.EndStaticConstructor();
}

void SymbolCreatorVisitor::Visit(ConstructorNode& constructorNode)
{
    symbolTable.BeginConstructor(constructorNode);
    int n = constructorNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = constructorNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    if (constructorNode.Body())
    {
        constructorNode.Body()->Accept(*this);
    }
    symbolTable.EndConstructor();
}

void SymbolCreatorVisitor::Visit(DestructorNode& destructorNode)
{
    symbolTable.BeginDestructor(destructorNode);
    if (destructorNode.Body())
    {
        destructorNode.Body()->Accept(*this);
    }
    symbolTable.EndDestructor();
}

void SymbolCreatorVisitor::Visit(MemberFunctionNode& memberFunctionNode)
{
    symbolTable.BeginMemberFunction(memberFunctionNode);
    int n = memberFunctionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = memberFunctionNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    if (memberFunctionNode.Body())
    {
        memberFunctionNode.Body()->Accept(*this);
    }
    symbolTable.EndMemberFunction();
}

void SymbolCreatorVisitor::Visit(MemberVariableNode& memberVariableNode)
{
    symbolTable.AddMemberVariable(memberVariableNode);
}

void SymbolCreatorVisitor::Visit(InterfaceNode& interfaceNode)
{
    symbolTable.BeginInterface(interfaceNode);
    int n = interfaceNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = interfaceNode.Members()[i];
        member->Accept(*this);
    }
    symbolTable.EndInterface();
}

void SymbolCreatorVisitor::Visit(DelegateNode& delegateNode)
{
    symbolTable.BeginDelegate(delegateNode);
    int n = delegateNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = delegateNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    symbolTable.EndDelegate();
}

void SymbolCreatorVisitor::Visit(ClassDelegateNode& classDelegateNode)
{
    symbolTable.BeginClassDelegate(classDelegateNode);
    int n = classDelegateNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = classDelegateNode.Parameters()[i];
        parameterNode->Accept(*this);
    }
    symbolTable.EndClassDelegate();
}

void SymbolCreatorVisitor::Visit(CompoundStatementNode& compoundStatementNode)
{
    symbolTable.BeginDeclarationBlock(compoundStatementNode);
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statement = compoundStatementNode.Statements()[i];
        statement->Accept(*this);
    }
    symbolTable.EndDeclarationBlock();
}

void SymbolCreatorVisitor::Visit(IfStatementNode& ifStatementNode)
{
    ifStatementNode.ThenS()->Accept(*this);
    if (ifStatementNode.ElseS())
    {
        ifStatementNode.ElseS()->Accept(*this);
    }
}

void SymbolCreatorVisitor::Visit(WhileStatementNode& whileStatementNode)
{
    whileStatementNode.Statement()->Accept(*this);
}

void SymbolCreatorVisitor::Visit(DoStatementNode& doStatementNode)
{
    doStatementNode.Statement()->Accept(*this);
}

void SymbolCreatorVisitor::Visit(ForStatementNode& forStatementNode)
{
    symbolTable.BeginDeclarationBlock(forStatementNode);
    forStatementNode.InitS()->Accept(*this);
    forStatementNode.ActionS()->Accept(*this);
    symbolTable.EndDeclarationBlock();
}

void SymbolCreatorVisitor::Visit(ConstructionStatementNode& constructionStatementNode)
{
    symbolTable.AddLocalVariable(constructionStatementNode);
}

void SymbolCreatorVisitor::Visit(SwitchStatementNode& switchStatementNode)
{
    int n = switchStatementNode.Cases().Count();
    for (int i = 0; i < n; ++i)
    {
        CaseStatementNode* caseStatementNode = switchStatementNode.Cases()[i];
        caseStatementNode->Accept(*this);
    }
    if (switchStatementNode.Default())
    {
        switchStatementNode.Default()->Accept(*this);
    }
}

void SymbolCreatorVisitor::Visit(CaseStatementNode& caseStatementNode)
{
    int n = caseStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = caseStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
}

void SymbolCreatorVisitor::Visit(DefaultStatementNode& defaultStatementNode)
{
    int n = defaultStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = defaultStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
}

void SymbolCreatorVisitor::Visit(CatchNode& catchNode)
{
    symbolTable.BeginDeclarationBlock(catchNode);
    if (catchNode.Id())
    {
        symbolTable.AddLocalVariable(*catchNode.Id());
    }
    catchNode.CatchBlock()->Accept(*this);
    symbolTable.EndDeclarationBlock();
}

void SymbolCreatorVisitor::Visit(TryStatementNode& tryStatementNode)
{
    tryStatementNode.TryBlock()->Accept(*this);
    int n = tryStatementNode.Catches().Count();
    for (int i = 0; i < n; ++i)
    {
        CatchNode* catchNode = tryStatementNode.Catches()[i];
        catchNode->Accept(*this);
    }
}

void SymbolCreatorVisitor::Visit(TypedefNode& typedefNode)
{
    symbolTable.AddTypedef(typedefNode);
}

void SymbolCreatorVisitor::Visit(ConstantNode& constantNode) 
{
    symbolTable.AddConstant(constantNode);
}

void SymbolCreatorVisitor::Visit(EnumTypeNode& enumTypeNode) 
{
    symbolTable.BeginEnumType(enumTypeNode);
    int n = enumTypeNode.Constants().Count();
    for (int i = 0; i < n; ++i)
    {
        enumTypeNode.Constants()[i]->Accept(*this);
    }
    symbolTable.EndEnumType();
}

void SymbolCreatorVisitor::Visit(EnumConstantNode& enumConstantNode) 
{
    symbolTable.AddEnumConstant(enumConstantNode);
}

} } // namespace cmajor::symbols
