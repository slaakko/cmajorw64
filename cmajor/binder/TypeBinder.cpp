// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/TypeBinder.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/TypeResolver.hpp>
#include <cmajor/ast/CompileUnit.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/DelegateSymbol.hpp>
#include <cmajor/symbols/TypedefSymbol.hpp>
#include <cmajor/symbols/ConstantSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::unicode;

TypeBinder::TypeBinder(BoundCompileUnit& boundCompileUnit_) : boundCompileUnit(boundCompileUnit_), symbolTable(boundCompileUnit.GetSymbolTable()), containerScope(), enumType(nullptr)
{
}

void TypeBinder::Visit(CompileUnitNode& compileUnitNode)
{
    boundCompileUnit.AddFileScope(new FileScope());
    compileUnitNode.GlobalNs()->Accept(*this);
}

void TypeBinder::Visit(NamespaceNode& namespaceNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = symbolTable.GetSymbol(&namespaceNode);
    containerScope = symbol->GetContainerScope();
    int n = namespaceNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = namespaceNode.Members()[i];
        member->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(AliasNode& aliasNode)
{
    boundCompileUnit.FirstFileScope()->InstallAlias(containerScope, &aliasNode);
    usingNodes.push_back(&aliasNode);
}

void TypeBinder::Visit(NamespaceImportNode& namespaceImportNode)
{
    boundCompileUnit.FirstFileScope()->InstallNamespaceImport(containerScope, &namespaceImportNode);
    usingNodes.push_back(&namespaceImportNode);
}

void TypeBinder::Visit(FunctionNode& functionNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = symbolTable.GetSymbol(&functionNode);
    Assert(symbol->GetSymbolType() == SymbolType::functionSymbol, "function symbol expected");
    FunctionSymbol* functionSymbol = static_cast<FunctionSymbol*>(symbol);
    if (functionSymbol->IsFunctionTemplate())
    {
        functionSymbol->CloneUsingNodes(usingNodes);
        return;
    }
    containerScope = functionSymbol->GetContainerScope();
    Specifiers specifiers = functionNode.GetSpecifiers();
    functionSymbol->SetSpecifiers(specifiers);
    int n = functionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = functionNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(parameterNode->TypeExpr(), boundCompileUnit, containerScope);
        Symbol* symbol = symbolTable.GetSymbol(parameterNode);
        Assert(symbol->GetSymbolType() == SymbolType::parameterSymbol, "parameter symbol expected");
        ParameterSymbol* parameterSymbol = static_cast<ParameterSymbol*>(symbol);
        parameterSymbol->SetType(parameterType);
    }
    TypeSymbol* returnType = ResolveType(functionNode.ReturnTypeExpr(), boundCompileUnit, containerScope);
    functionSymbol->SetReturnType(returnType);
    functionSymbol->ComputeName();
    if (functionNode.Body())
    {
        functionNode.Body()->Accept(*this);
    }
    else
    {
        if (!functionSymbol->IsExternal())
        {
            throw Exception("function has no body", functionSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(ClassNode& classNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&classNode);
    Assert(symbol->GetSymbolType() == SymbolType::classTypeSymbol, "class type symbol expected");
    ClassTypeSymbol* classTypeSymbol = static_cast<ClassTypeSymbol*>(symbol);
    BindClass(classTypeSymbol, &classNode);
}

void TypeBinder::BindClass(ClassTypeSymbol* classTypeSymbol, ClassNode* classNode)
{
    if (classTypeSymbol->IsBound()) return;
    classTypeSymbol->SetBound();
    if (classTypeSymbol->IsClassTemplate())
    {
        classTypeSymbol->CloneUsingNodes(usingNodes);
        return;
    }
    ContainerScope* prevContainerScope = containerScope;
    containerScope = classTypeSymbol->GetContainerScope();
    classTypeSymbol->SetSpecifiers(classNode->GetSpecifiers());
    int nb = classNode->BaseClassOrInterfaces().Count();
    for (int i = 0; i < nb; ++i)
    {
        Node* baseOrInterfaceNode = classNode->BaseClassOrInterfaces()[i];
        TypeSymbol* baseOrInterfaceSymbol = ResolveType(baseOrInterfaceNode, boundCompileUnit, containerScope);
        if (baseOrInterfaceSymbol->IsClassTypeSymbol())
        {
            ClassTypeSymbol* baseClassSymbol = static_cast<ClassTypeSymbol*>(baseOrInterfaceSymbol);
            if (baseClassSymbol->IsProject())
            {
                Node* node = symbolTable.GetNode(baseClassSymbol);
                Assert(node->GetNodeType() == NodeType::classNode, "class node expected");
                ClassNode* baseClassNode = static_cast<ClassNode*>(node);
                BindClass(baseClassSymbol, baseClassNode);
            }
            if (classTypeSymbol->BaseClass())
            {
                throw Exception("class type can have at most one base class", classTypeSymbol->GetSpan(), baseClassSymbol->GetSpan());
            }
            else if (baseClassSymbol == classTypeSymbol)
            {
                throw Exception("class cannot derive from itself", classTypeSymbol->GetSpan());
            }
            else
            {
                classTypeSymbol->SetBaseClass(baseClassSymbol);
            }
        }
        else if (baseOrInterfaceSymbol->GetSymbolType() == SymbolType::interfaceTypeSymbol)
        {
            InterfaceTypeSymbol* interfaceTypeSymbol = static_cast<InterfaceTypeSymbol*>(baseOrInterfaceSymbol);
            if (interfaceTypeSymbol->IsProject())
            {
                Node* node = symbolTable.GetNode(interfaceTypeSymbol);
                Assert(node->GetNodeType() == NodeType::interfaceNode, "interface node expected");
                InterfaceNode* interfaceNode = static_cast<InterfaceNode*>(node);
                BindInterface(interfaceTypeSymbol, interfaceNode);
            }
            classTypeSymbol->AddImplementedInterface(interfaceTypeSymbol);
        }
        else
        {
            throw Exception("symbol '" + ToUtf8(baseOrInterfaceSymbol->FullName()) + "' is not a class or interface type symbol", baseOrInterfaceNode->GetSpan(), baseOrInterfaceSymbol->GetSpan());
        }
    }
    int nm = classNode->Members().Count();
    for (int i = 0; i < nm; ++i)
    {
        Node* member = classNode->Members()[i];
        member->Accept(*this);
    }
    if (classTypeSymbol->BaseClass())
    {
        classTypeSymbol->GetContainerScope()->SetBase(classTypeSymbol->BaseClass()->GetContainerScope());
    }
    // todo: InitVmt, InitImts, create object layout
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(StaticConstructorNode& staticConstructorNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&staticConstructorNode);
    Assert(symbol->GetSymbolType() == SymbolType::staticConstructorSymbol, "static constructor symbol expected");
    StaticConstructorSymbol* staticConstructorSymbol = static_cast<StaticConstructorSymbol*>(symbol);
    ContainerScope* prevContainerScope = containerScope;
    containerScope = staticConstructorSymbol->GetContainerScope();
    staticConstructorSymbol->SetSpecifiers(staticConstructorNode.GetSpecifiers());
    staticConstructorSymbol->ComputeName();
    if (staticConstructorNode.Body())
    {
        staticConstructorNode.Body()->Accept(*this);
    }
    else
    {
        throw Exception("static constructor has no body", staticConstructorSymbol->GetSpan());
    }
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(ConstructorNode& constructorNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&constructorNode);
    Assert(symbol->GetSymbolType() == SymbolType::constructorSymbol, "constructor symbol expected");
    ConstructorSymbol* constructorSymbol = static_cast<ConstructorSymbol*>(symbol);
    ContainerScope* prevContainerScope = containerScope;
    containerScope = constructorSymbol->GetContainerScope();
    constructorSymbol->SetSpecifiers(constructorNode.GetSpecifiers());
    constructorSymbol->ComputeName();
    const Symbol* parent = constructorSymbol->Parent();
    if (parent->IsStatic())
    {
        throw Exception("static class cannot contain instance constructors", constructorSymbol->GetSpan(), parent->GetSpan());
    }
    int n = constructorNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = constructorNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(parameterNode->TypeExpr(), boundCompileUnit, containerScope);
        Symbol* symbol = symbolTable.GetSymbol(parameterNode);
        Assert(symbol->GetSymbolType() == SymbolType::parameterSymbol, "parameter symbol expected");
        ParameterSymbol* parameterSymbol = static_cast<ParameterSymbol*>(symbol);
        parameterSymbol->SetType(parameterType);
    }
    // todo add conversion to symbol table
    if (constructorNode.Body())
    {
        if (constructorSymbol->IsDefault() || constructorSymbol->IsSuppressed())
        {
            throw Exception("default or suppressed constructor cannot have a body", constructorSymbol->GetSpan());
        }
        constructorNode.Body()->Accept(*this);
    }
    else
    {
        if (constructorSymbol->IsDefault() || constructorSymbol->IsSuppressed())
        {
            // todo
        }
        else
        {
            throw Exception("constructor has no body", constructorSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(DestructorNode& destructorNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&destructorNode);
    Assert(symbol->GetSymbolType() == SymbolType::destructorSymbol, "destructor symbol expected");
    DestructorSymbol* destructorSymbol = static_cast<DestructorSymbol*>(symbol);
    ContainerScope* prevContainerScope = containerScope;
    containerScope = destructorSymbol->GetContainerScope();
    destructorSymbol->SetSpecifiers(destructorNode.GetSpecifiers());
    destructorSymbol->ComputeName();
    const Symbol* parent = destructorSymbol->Parent();
    if (parent->IsStatic())
    {
        throw Exception("static class cannot contain a destructor", destructorSymbol->GetSpan(), parent->GetSpan());
    }
    if (destructorNode.Body())
    {
        if (destructorSymbol->IsDefault())
        {
            throw Exception("default destructor cannot have a body", destructorSymbol->GetSpan());
        }
        destructorNode.Body()->Accept(*this);
    }
    else
    {
        if (destructorSymbol->IsDefault())
        {
            // todo
        }
        else
        {
            throw Exception("destructor has no body", destructorSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(MemberFunctionNode& memberFunctionNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&memberFunctionNode);
    Assert(symbol->GetSymbolType() == SymbolType::memberFunctionSymbol, "member function symbol expected");
    MemberFunctionSymbol* memberFunctionSymbol = static_cast<MemberFunctionSymbol*>(symbol);
    ContainerScope* prevContainerScope = containerScope;
    containerScope = memberFunctionSymbol->GetContainerScope();
    memberFunctionSymbol->SetSpecifiers(memberFunctionNode.GetSpecifiers());
    memberFunctionSymbol->ComputeName();
    const Symbol* parent = memberFunctionSymbol->Parent();
    if (parent->IsStatic() && !memberFunctionSymbol->IsStatic())
    {
        throw Exception("static class cannot contain nonstatic member functions", memberFunctionSymbol->GetSpan(), parent->GetSpan());
    }
    int n = memberFunctionNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = memberFunctionNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(parameterNode->TypeExpr(), boundCompileUnit, containerScope);
        Symbol* symbol = symbolTable.GetSymbol(parameterNode);
        Assert(symbol->GetSymbolType() == SymbolType::parameterSymbol, "parameter symbol expected");
        ParameterSymbol* parameterSymbol = static_cast<ParameterSymbol*>(symbol);
        parameterSymbol->SetType(parameterType);
    }
    TypeSymbol* returnType = ResolveType(memberFunctionNode.ReturnTypeExpr(), boundCompileUnit, containerScope);
    memberFunctionSymbol->SetReturnType(returnType);
    if (memberFunctionNode.Body())
    {
        if (memberFunctionSymbol->IsDefault() || memberFunctionSymbol->IsSuppressed())
        {
            throw Exception("default or suppressed member function cannot have a body", memberFunctionSymbol->GetSpan());
        }
        memberFunctionNode.Body()->Accept(*this);
    }
    else
    {
        if (memberFunctionSymbol->IsDefault() || memberFunctionSymbol->IsSuppressed())
        {
            // todo
        }
        else
        {
            throw Exception("member function has no body", memberFunctionSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(MemberVariableNode& memberVariableNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&memberVariableNode);
    Assert(symbol->GetSymbolType() == SymbolType::memberVariableSymbol, "member variable symbol expected");
    MemberVariableSymbol* memberVariableSymbol = static_cast<MemberVariableSymbol*>(symbol);
    memberVariableSymbol->SetSpecifiers(memberVariableNode.GetSpecifiers());
    const Symbol* parent = memberVariableSymbol->Parent();
    if (parent->IsStatic() && !memberVariableSymbol->IsStatic())
    {
        throw Exception("static class cannot contain instance variables", memberVariableSymbol->GetSpan(), parent->GetSpan());
    }
    TypeSymbol* memberVariableType = ResolveType(memberVariableNode.TypeExpr(), boundCompileUnit, containerScope);
    memberVariableSymbol->SetType(memberVariableType);
}

void TypeBinder::Visit(InterfaceNode& interfaceNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&interfaceNode);
    Assert(symbol->GetSymbolType() == SymbolType::interfaceTypeSymbol, "interface type symbol expected");
    InterfaceTypeSymbol* interfaceTypeSymbol = static_cast<InterfaceTypeSymbol*>(symbol);
    BindInterface(interfaceTypeSymbol, &interfaceNode);
}

void TypeBinder::BindInterface(InterfaceTypeSymbol* interfaceTypeSymbol, InterfaceNode* interfaceNode)
{
    if (interfaceTypeSymbol->IsBound()) return;
    interfaceTypeSymbol->SetBound();
    interfaceTypeSymbol->SetSpecifiers(interfaceNode->GetSpecifiers());
    ContainerScope* prevContainerScope = containerScope;
    containerScope = interfaceTypeSymbol->GetContainerScope();
    int nm = interfaceNode->Members().Count();
    for (int i = 0; i < nm; ++i)
    {
        Node* member = interfaceNode->Members()[i];
        member->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(DelegateNode& delegateNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&delegateNode);
    Assert(symbol->GetSymbolType() == SymbolType::delegateTypeSymbol, "delegate type symbol expected");
    DelegateTypeSymbol* delegateTypeSymbol = static_cast<DelegateTypeSymbol*>(symbol);
    delegateTypeSymbol->SetSpecifiers(delegateNode.GetSpecifiers());
    int n = delegateNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = delegateNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(parameterNode->TypeExpr(), boundCompileUnit, containerScope);
        Symbol* symbol = symbolTable.GetSymbol(parameterNode);
        Assert(symbol->GetSymbolType() == SymbolType::parameterSymbol, "parameter symbol expected");
        ParameterSymbol* parameterSymbol = static_cast<ParameterSymbol*>(symbol);
        parameterSymbol->SetType(parameterType);
    }
    TypeSymbol* returnType = ResolveType(delegateNode.ReturnTypeExpr(), boundCompileUnit, containerScope);
    delegateTypeSymbol->SetReturnType(returnType);
}

void TypeBinder::Visit(ClassDelegateNode& classDelegateNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&classDelegateNode);
    Assert(symbol->GetSymbolType() == SymbolType::classDelegateTypeSymbol, "class delegate type symbol expected");
    ClassDelegateTypeSymbol* classDelegateTypeSymbol = static_cast<ClassDelegateTypeSymbol*>(symbol);
    classDelegateTypeSymbol->SetSpecifiers(classDelegateNode.GetSpecifiers());
    int n = classDelegateNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = classDelegateNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(parameterNode->TypeExpr(), boundCompileUnit, containerScope);
        Symbol* symbol = symbolTable.GetSymbol(parameterNode);
        Assert(symbol->GetSymbolType() == SymbolType::parameterSymbol, "parameter symbol expected");
        ParameterSymbol* parameterSymbol = static_cast<ParameterSymbol*>(symbol);
        parameterSymbol->SetType(parameterType);
    }
    TypeSymbol* returnType = ResolveType(classDelegateNode.ReturnTypeExpr(), boundCompileUnit, containerScope);
    classDelegateTypeSymbol->SetReturnType(returnType);
}

void TypeBinder::Visit(CompoundStatementNode& compoundStatementNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&compoundStatementNode);
    Assert(symbol->GetSymbolType() == SymbolType::declarationBlock, "declaration block expected");
    DeclarationBlock* declarationBlock = static_cast<DeclarationBlock*>(symbol);
    ContainerScope* prevContainerScope = containerScope;
    containerScope = declarationBlock->GetContainerScope();
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(IfStatementNode& ifStatementNode)
{
    ifStatementNode.ThenS()->Accept(*this);
    if (ifStatementNode.ElseS())
    {
        ifStatementNode.ElseS()->Accept(*this);
    }
}

void TypeBinder::Visit(WhileStatementNode& whileStatementNode)
{
    whileStatementNode.Statement()->Accept(*this);
}

void TypeBinder::Visit(DoStatementNode& doStatementNode)
{
    doStatementNode.Statement()->Accept(*this);
}

void TypeBinder::Visit(ForStatementNode& forStatementNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&forStatementNode);
    Assert(symbol->GetSymbolType() == SymbolType::declarationBlock, "declaration block expected");
    DeclarationBlock* declarationBlock = static_cast<DeclarationBlock*>(symbol);
    ContainerScope* prevContainerScope = containerScope;
    containerScope = declarationBlock->GetContainerScope();
    forStatementNode.InitS()->Accept(*this);
    forStatementNode.ActionS()->Accept(*this);
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&constructionStatementNode);
    Assert(symbol->GetSymbolType() == SymbolType::localVariableSymbol, "local variable symbol expected");
    LocalVariableSymbol* localVariableSymbol = static_cast<LocalVariableSymbol*>(symbol);
    TypeSymbol* type = ResolveType(constructionStatementNode.TypeExpr(), boundCompileUnit, containerScope);
    localVariableSymbol->SetType(type);
}

void TypeBinder::Visit(SwitchStatementNode& switchStatementNode)
{
    int n = switchStatementNode.Cases().Count();
    for (int i = 0; i < n; ++i)
    {
        CaseStatementNode* caseStatement = switchStatementNode.Cases()[i];
        caseStatement->Accept(*this);
    }
    if (switchStatementNode.Default())
    {
        switchStatementNode.Default()->Accept(*this);
    }
}

void TypeBinder::Visit(CaseStatementNode& caseStatementNode)
{
    int n = caseStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = caseStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
}

void TypeBinder::Visit(DefaultStatementNode& defaultStatementNode)
{
    int n = defaultStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = defaultStatementNode.Statements()[i];
        statementNode->Accept(*this);
    }
}

void TypeBinder::Visit(CatchNode& catchNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&catchNode);
    Assert(symbol->GetSymbolType() == SymbolType::declarationBlock, "declaration block expected");
    DeclarationBlock* declarationBlock = static_cast<DeclarationBlock*>(symbol);
    ContainerScope* prevContainerScope = containerScope;
    containerScope = declarationBlock->GetContainerScope();
    if (catchNode.Id())
    {
        Symbol* symbol = symbolTable.GetSymbol(catchNode.Id()); 
        Assert(symbol->GetSymbolType() == SymbolType::localVariableSymbol, "local variable symbol expected");
        LocalVariableSymbol* exceptionVarSymbol = static_cast<LocalVariableSymbol*>(symbol);
        TypeSymbol* type = ResolveType(catchNode.TypeExpr(), boundCompileUnit, containerScope);
        if (type->IsClassTypeSymbol())
        {
            ClassTypeSymbol* exceptionVarClassType = static_cast<ClassTypeSymbol*>(type);
            TypeSymbol* systemExceptionType = symbolTable.GetTypeByName(U"System.Exception");
            Assert(systemExceptionType->IsClassTypeSymbol(), "System.Exception not of class type");
            ClassTypeSymbol* systemExceptionClassType = static_cast<ClassTypeSymbol*>(systemExceptionType);
            if (exceptionVarClassType->IsProject())
            {
                Node* exceptionVarNode = symbolTable.GetNode(type);
                Assert(exceptionVarNode->GetNodeType() == NodeType::classNode, "class node expected");
                ClassNode* exceptionVarClassNode = static_cast<ClassNode*>(exceptionVarNode);
                BindClass(exceptionVarClassType, exceptionVarClassNode);
            }
            if (exceptionVarClassType == systemExceptionClassType || exceptionVarClassType->HasBaseClass(systemExceptionClassType))
            {
                exceptionVarSymbol->SetType(exceptionVarClassType);
            }
            else
            {
                throw Exception("exception variable must be of class type equal to System.Exception class or derive from it", catchNode.TypeExpr()->GetSpan());
            }
        }
        else
        {
            throw Exception("exception variable must be of class type equal to System.Exception class or derive from it", catchNode.TypeExpr()->GetSpan());
        }
    }
    catchNode.CatchBlock()->Accept(*this);
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(TryStatementNode& tryStatementNode)
{
    tryStatementNode.TryBlock()->Accept(*this);
    int n = tryStatementNode.Catches().Count();
    for (int i = 0; i < n; ++i)
    {
        CatchNode* catchNode = tryStatementNode.Catches()[i];
        catchNode->Accept(*this);
    }
}

void TypeBinder::Visit(TypedefNode& typedefNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&typedefNode);
    Assert(symbol->GetSymbolType() == SymbolType::typedefSymbol, "typedef symbol expected");
    TypedefSymbol* typedefSymbol = static_cast<TypedefSymbol*>(symbol);
    typedefSymbol->SetSpecifiers(typedefNode.GetSpecifiers());
    TypeSymbol* typeSymbol = ResolveType(typedefNode.TypeExpr(), boundCompileUnit, containerScope);
    typedefSymbol->SetType(typeSymbol);
}

void TypeBinder::Visit(ConstantNode& constantNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&constantNode);
    Assert(symbol->GetSymbolType() == SymbolType::constantSymbol, "constant symbol expected");
    ConstantSymbol* constantSymbol = static_cast<ConstantSymbol*>(symbol);
    constantSymbol->SetSpecifiers(constantNode.GetSpecifiers());
    TypeSymbol* typeSymbol = ResolveType(constantNode.TypeExpr(), boundCompileUnit, containerScope);
    constantSymbol->SetType(typeSymbol);
    constantSymbol->SetEvaluating();
    // todo: evaluate value
    // constantSymbol->SetValue(value);
    constantSymbol->ResetEvaluating();
}

void TypeBinder::Visit(EnumTypeNode& enumTypeNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&enumTypeNode);
    Assert(symbol->GetSymbolType() == SymbolType::enumTypeSymbol, "enumerated type symbol expected");
    EnumTypeSymbol* enumTypeSymbol = static_cast<EnumTypeSymbol*>(symbol);
    if (enumTypeSymbol->IsBound()) return;
    enumTypeSymbol->SetBound();
    EnumTypeSymbol* prevEnumType = enumType;
    enumType = enumTypeSymbol;
    enumTypeSymbol->SetSpecifiers(enumTypeNode.GetSpecifiers());
    TypeSymbol* underlyingType = symbolTable.GetTypeByName(U"int");
    if (enumTypeNode.GetUnderlyingType())
    {
        underlyingType = ResolveType(enumTypeNode.GetUnderlyingType(), boundCompileUnit, containerScope);
    }
    enumTypeSymbol->SetUnderlyingType(underlyingType);
    ContainerScope* prevContainerScope = containerScope;
    containerScope = enumTypeSymbol->GetContainerScope();
    int n = enumTypeNode.Constants().Count();
    for (int i = 0; i < n; ++i)
    {
        EnumConstantNode* enumConstantNode = enumTypeNode.Constants()[i];
        enumConstantNode->Accept(*this);
    }
    containerScope = prevContainerScope;
    enumType = prevEnumType;
}

void TypeBinder::Visit(EnumConstantNode& enumConstantNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&enumConstantNode);
    Assert(symbol->GetSymbolType() == SymbolType::enumConstantSymbol, "enumeration constant symbol expected");
    EnumConstantSymbol* enumConstantSymbol = static_cast<EnumConstantSymbol*>(symbol);
    enumConstantSymbol->SetEvaluating();
    // todo: evaluate value
    // enumConstantSymbol->SetValue(value);
    enumConstantSymbol->ResetEvaluating();
}

} } // namespace cmajor::binder
