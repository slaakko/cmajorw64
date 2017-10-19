// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/TypeBinder.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/TypeResolver.hpp>
#include <cmajor/binder/Concept.hpp>
#include <cmajor/binder/Evaluator.hpp>
#include <cmajor/ast/CompileUnit.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/DelegateSymbol.hpp>
#include <cmajor/symbols/TypedefSymbol.hpp>
#include <cmajor/symbols/ConstantSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/TemplateSymbol.hpp>
#include <cmajor/symbols/ConceptSymbol.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::unicode;

class UsingNodeAdder : public Visitor
{
public:
    UsingNodeAdder(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerscope);
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(AliasNode& aliasNode) override;
    void Visit(NamespaceImportNode& namespaceImportNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    SymbolTable& symbolTable;
    ContainerScope* containerScope;
};

UsingNodeAdder::UsingNodeAdder(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_) :
    boundCompileUnit(boundCompileUnit_), symbolTable(boundCompileUnit.GetSymbolTable()), containerScope(containerScope_)
{
}

void UsingNodeAdder::Visit(NamespaceNode& namespaceNode)
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

void UsingNodeAdder::Visit(AliasNode& aliasNode)
{
    boundCompileUnit.FirstFileScope()->InstallAlias(containerScope, &aliasNode);
}

void UsingNodeAdder::Visit(NamespaceImportNode& namespaceImportNode)
{
    boundCompileUnit.FirstFileScope()->InstallNamespaceImport(containerScope, &namespaceImportNode);
}

TypeBinder::TypeBinder(BoundCompileUnit& boundCompileUnit_) : 
    boundCompileUnit(boundCompileUnit_), symbolTable(boundCompileUnit.GetSymbolTable()), containerScope(), enumType(nullptr), currentFunctionSymbol(nullptr)
{
}

void TypeBinder::AddUsingNodesToCurrentCompileUnit(Node* node)
{
    NamespaceNode* namespaceNode = nullptr;
    Node* parent = node->Parent();
    while (parent)
    {
        if (parent->GetNodeType() == NodeType::namespaceNode)
        {
            namespaceNode = static_cast<NamespaceNode*>(parent);
        }
        parent = parent->Parent();
    }
    if (!namespaceNode)
    {
        throw Exception("global namespace parent not found for node", node->GetSpan());
    }
    UsingNodeAdder usingNodeAdder(boundCompileUnit, containerScope);
    namespaceNode->Accept(usingNodeAdder);
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
    FunctionSymbol* prevFunctionSymbol = currentFunctionSymbol;
    currentFunctionSymbol = functionSymbol;
    if (functionSymbol->IsFunctionTemplate())
    {
        functionSymbol->CloneUsingNodes(usingNodes);
        if (functionNode.WhereConstraint())
        {
            CloneContext cloneContext;
            functionSymbol->SetConstraint(static_cast<WhereConstraintNode*>(functionNode.WhereConstraint()->Clone(cloneContext)));
        }
    }
    containerScope = functionSymbol->GetContainerScope();
    Specifiers specifiers = functionNode.GetSpecifiers();
    functionSymbol->SetSpecifiers(specifiers);
    if (GetGlobalFlag(GlobalFlags::release) && functionSymbol->IsInline())
    {
        functionSymbol->CloneUsingNodes(usingNodes);
    }
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
    if (!functionSymbol->Constraint() && functionNode.WhereConstraint())
    {
        CloneContext cloneContext;
        functionSymbol->SetConstraint(static_cast<WhereConstraintNode*>(functionNode.WhereConstraint()->Clone(cloneContext)));
    }
    functionSymbol->ComputeName();
    if (functionSymbol->ReturnsClassOrClassDelegateByValue())
    {
        ParameterSymbol* returnParam = new ParameterSymbol(functionNode.ReturnTypeExpr()->GetSpan(), U"@return");
        returnParam->SetParent(functionSymbol);
        returnParam->SetType(returnType->AddPointer(functionNode.GetSpan()));
        functionSymbol->SetReturnParam(returnParam);
    }
    if (functionNode.Body() && !functionSymbol->IsFunctionTemplate())
    {
        functionNode.Body()->Accept(*this);
    }
    else
    {
        if (!functionSymbol->IsExternal() && !functionSymbol->IsFunctionTemplate())
        {
            throw Exception("function has no body", functionSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
    currentFunctionSymbol = prevFunctionSymbol;
}

void TypeBinder::Visit(ClassNode& classNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&classNode);
    Assert(symbol->GetSymbolType() == SymbolType::classTypeSymbol || symbol->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol, "class type symbol expected");
    ClassTypeSymbol* classTypeSymbol = static_cast<ClassTypeSymbol*>(symbol);
    BindClass(classTypeSymbol, &classNode, true);
}

void TypeBinder::BindClass(ClassTypeSymbol* classTypeSymbol, ClassNode* classNode, bool fromOwnCompileUnit)
{
    if (classTypeSymbol->IsBound()) return;
    classTypeSymbol->SetBound();
    if (!fromOwnCompileUnit)
    {
        AddUsingNodesToCurrentCompileUnit(classNode);
    }
    if (classTypeSymbol->IsClassTemplate())
    {
        classTypeSymbol->CloneUsingNodes(usingNodes);
        if (classNode->WhereConstraint())
        {
            CloneContext cloneContext;
            classTypeSymbol->SetConstraint(static_cast<ConstraintNode*>(classNode->WhereConstraint()->Clone(cloneContext)));
        }
        classTypeSymbol->SetAccess(classNode->GetSpecifiers() & Specifiers::access_);
        classTypeSymbol->ComputeName();
        return;
    }
    ContainerScope* prevContainerScope = containerScope;
    containerScope = classTypeSymbol->GetContainerScope();
    classTypeSymbol->SetSpecifiers(classNode->GetSpecifiers());
    if (classNode->WhereConstraint())
    {
        CloneContext cloneContext;
        classTypeSymbol->SetConstraint(static_cast<ConstraintNode*>(classNode->WhereConstraint()->Clone(cloneContext)));
    }
    classTypeSymbol->ComputeName();
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
                BindClass(baseClassSymbol, baseClassNode, false);
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
                BindInterface(interfaceTypeSymbol, interfaceNode, false);
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
    classTypeSymbol->InitVmt();
    classTypeSymbol->InitImts();
    classTypeSymbol->CreateLayouts();
    if (classTypeSymbol->IsPolymorphic() && !classTypeSymbol->IsPrototypeTemplateSpecialization())
    {
        symbolTable.AddPolymorphicClass(classTypeSymbol);
    }
    if (classTypeSymbol->StaticConstructor())
    {
        symbolTable.AddClassHavingStaticConstructor(classTypeSymbol);
    }
    containerScope = prevContainerScope;
}

void TypeBinder::Visit(StaticConstructorNode& staticConstructorNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&staticConstructorNode);
    Assert(symbol->GetSymbolType() == SymbolType::staticConstructorSymbol, "static constructor symbol expected");
    StaticConstructorSymbol* staticConstructorSymbol = static_cast<StaticConstructorSymbol*>(symbol);
    FunctionSymbol* prevFunctionSymbol = currentFunctionSymbol;
    currentFunctionSymbol = staticConstructorSymbol;
    ContainerScope* prevContainerScope = containerScope;
    containerScope = staticConstructorSymbol->GetContainerScope();
    staticConstructorSymbol->SetSpecifiers(staticConstructorNode.GetSpecifiers());
    if (staticConstructorSymbol->Parent()->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        staticConstructorSymbol->SetTemplateSpecialization();
        staticConstructorSymbol->SetLinkOnceOdrLinkage();
    }
    if (!staticConstructorSymbol->Constraint() && staticConstructorNode.WhereConstraint())
    {
        CloneContext cloneContext;
        staticConstructorSymbol->SetConstraint(static_cast<WhereConstraintNode*>(staticConstructorNode.WhereConstraint()->Clone(cloneContext)));
    }
    staticConstructorSymbol->ComputeName();
    if (staticConstructorNode.Body())
    {
        staticConstructorNode.Body()->Accept(*this);
    }
    else
    {
        if (!staticConstructorSymbol->IsTemplateSpecialization())
        {
            throw Exception("static constructor has no body", staticConstructorSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
    currentFunctionSymbol = prevFunctionSymbol;
}

void TypeBinder::Visit(ConstructorNode& constructorNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&constructorNode);
    Assert(symbol->GetSymbolType() == SymbolType::constructorSymbol, "constructor symbol expected");
    ConstructorSymbol* constructorSymbol = static_cast<ConstructorSymbol*>(symbol);
    FunctionSymbol* prevFunctionSymbol = currentFunctionSymbol;
    currentFunctionSymbol = constructorSymbol;
    ContainerScope* prevContainerScope = containerScope;
    containerScope = constructorSymbol->GetContainerScope();
    constructorSymbol->SetSpecifiers(constructorNode.GetSpecifiers());
    if (GetGlobalFlag(GlobalFlags::release) && constructorSymbol->IsInline())
    {
        constructorSymbol->CloneUsingNodes(usingNodes);
    }
    Symbol* parent = constructorSymbol->Parent();
    if (parent->IsStatic())
    {
        throw Exception("static class cannot contain instance constructors", constructorSymbol->GetSpan(), parent->GetSpan());
    }
    if (parent->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        constructorSymbol->SetTemplateSpecialization();
        constructorSymbol->SetLinkOnceOdrLinkage();
    }
    Assert(parent->IsClassTypeSymbol(), "class type symbol expected");
    ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(parent);
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
    if (!constructorSymbol->Constraint() && constructorNode.WhereConstraint())
    {
        CloneContext cloneContext;
        constructorSymbol->SetConstraint(static_cast<WhereConstraintNode*>(constructorNode.WhereConstraint()->Clone(cloneContext)));
    }
    constructorSymbol->ComputeName();
    if (constructorSymbol->IsDefaultConstructor())
    {
        classType->SetDefaultConstructor(constructorSymbol);
    }
    else if (constructorSymbol->IsCopyConstructor())
    {
        classType->SetCopyConstructor(constructorSymbol);
    }
    else if (constructorSymbol->IsMoveConstructor())
    {
        classType->SetMoveConstructor(constructorSymbol);
    }
    else if (constructorSymbol->Arity() == 2 && !constructorSymbol->IsExplicit())
    {
        constructorSymbol->SetConversion();
        symbolTable.AddConversion(constructorSymbol);
    }
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
        if (!constructorSymbol->IsDefault() && !constructorSymbol->IsSuppressed() && !constructorSymbol->IsTemplateSpecialization())
        {
            throw Exception("constructor has no body", constructorSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
    currentFunctionSymbol = prevFunctionSymbol;
}

void TypeBinder::Visit(DestructorNode& destructorNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&destructorNode);
    Assert(symbol->GetSymbolType() == SymbolType::destructorSymbol, "destructor symbol expected");
    DestructorSymbol* destructorSymbol = static_cast<DestructorSymbol*>(symbol);
    FunctionSymbol* prevFunctionSymbol = currentFunctionSymbol;
    currentFunctionSymbol = destructorSymbol;
    ContainerScope* prevContainerScope = containerScope;
    containerScope = destructorSymbol->GetContainerScope();
    destructorSymbol->SetSpecifiers(destructorNode.GetSpecifiers());
    const Symbol* parent = destructorSymbol->Parent();
    if (parent->IsStatic())
    {
        throw Exception("static class cannot contain a destructor", destructorSymbol->GetSpan(), parent->GetSpan());
    }
    if (parent->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        destructorSymbol->SetTemplateSpecialization();
        destructorSymbol->SetLinkOnceOdrLinkage();
    }
    if (!destructorSymbol->Constraint() && destructorNode.WhereConstraint())
    {
        CloneContext cloneContext;
        destructorSymbol->SetConstraint(static_cast<WhereConstraintNode*>(destructorNode.WhereConstraint()->Clone(cloneContext)));
    }
    destructorSymbol->ComputeName();
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
        if (!destructorSymbol->IsDefault() && !destructorSymbol->IsTemplateSpecialization())
        {
            throw Exception("destructor has no body", destructorSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
    currentFunctionSymbol = prevFunctionSymbol;
}

void TypeBinder::Visit(MemberFunctionNode& memberFunctionNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&memberFunctionNode);
    Assert(symbol->GetSymbolType() == SymbolType::memberFunctionSymbol, "member function symbol expected");
    MemberFunctionSymbol* memberFunctionSymbol = static_cast<MemberFunctionSymbol*>(symbol);
    FunctionSymbol* prevFunctionSymbol = currentFunctionSymbol;
    currentFunctionSymbol = memberFunctionSymbol;
    ContainerScope* prevContainerScope = containerScope;
    containerScope = memberFunctionSymbol->GetContainerScope();
    memberFunctionSymbol->SetSpecifiers(memberFunctionNode.GetSpecifiers());
    if (GetGlobalFlag(GlobalFlags::release) && memberFunctionSymbol->IsInline())
    {
        memberFunctionSymbol->CloneUsingNodes(usingNodes);
    }
    const Symbol* parent = memberFunctionSymbol->Parent();
    if (parent->IsStatic() && !memberFunctionSymbol->IsStatic())
    {
        throw Exception("static class cannot contain nonstatic member functions", memberFunctionSymbol->GetSpan(), parent->GetSpan());
    }
    if (parent->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        memberFunctionSymbol->SetTemplateSpecialization();
        memberFunctionSymbol->SetLinkOnceOdrLinkage();
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
    if (!memberFunctionSymbol->Constraint() && memberFunctionNode.WhereConstraint())
    {
        CloneContext cloneContext;
        memberFunctionSymbol->SetConstraint(static_cast<WhereConstraintNode*>(memberFunctionNode.WhereConstraint()->Clone(cloneContext)));
    }
    memberFunctionSymbol->ComputeName();
    if (memberFunctionSymbol->ReturnsClassOrClassDelegateByValue())
    {
        ParameterSymbol* returnParam = new ParameterSymbol(memberFunctionNode.ReturnTypeExpr()->GetSpan(), U"@return");
        returnParam->SetParent(memberFunctionSymbol);
        returnParam->SetType(returnType->AddPointer(memberFunctionNode.GetSpan()));
        memberFunctionSymbol->SetReturnParam(returnParam);
    }
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
        if (!memberFunctionSymbol->IsAbstract() && !memberFunctionSymbol->IsDefault() && !memberFunctionSymbol->IsSuppressed() && !memberFunctionSymbol->IsTemplateSpecialization())
        {
            throw Exception("member function has no body", memberFunctionSymbol->GetSpan());
        }
    }
    containerScope = prevContainerScope;
    currentFunctionSymbol = prevFunctionSymbol;
}

void TypeBinder::Visit(ConversionFunctionNode& conversionFunctionNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&conversionFunctionNode);
    Assert(symbol->GetSymbolType() == SymbolType::conversionFunctionSymbol, "conversion function symbol expected");
    ConversionFunctionSymbol* conversionFunctionSymbol = static_cast<ConversionFunctionSymbol*>(symbol);
    FunctionSymbol* prevFunctionSymbol = currentFunctionSymbol;
    currentFunctionSymbol = conversionFunctionSymbol;
    ContainerScope* prevContainerScope = containerScope;
    containerScope = conversionFunctionSymbol->GetContainerScope();
    conversionFunctionSymbol->SetSpecifiers(conversionFunctionNode.GetSpecifiers());
    if (GetGlobalFlag(GlobalFlags::release) && conversionFunctionSymbol->IsInline())
    {
        conversionFunctionSymbol->CloneUsingNodes(usingNodes);
    }
    const Symbol* parent = conversionFunctionSymbol->Parent();
    if (parent->IsStatic())
    {
        throw Exception("static class cannot contain conversion functions", conversionFunctionSymbol->GetSpan(), parent->GetSpan());
    }
    if (parent->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        conversionFunctionSymbol->SetTemplateSpecialization();
        conversionFunctionSymbol->SetLinkOnceOdrLinkage();
    }
    TypeSymbol* returnType = ResolveType(conversionFunctionNode.ReturnTypeExpr(), boundCompileUnit, containerScope);
    conversionFunctionSymbol->SetReturnType(returnType);
    if (!conversionFunctionSymbol->Constraint() && conversionFunctionNode.WhereConstraint())
    {
        CloneContext cloneContext;
        conversionFunctionSymbol->SetConstraint(static_cast<WhereConstraintNode*>(conversionFunctionNode.WhereConstraint()->Clone(cloneContext)));
    }
    conversionFunctionSymbol->ComputeName();
    if (conversionFunctionSymbol->ReturnsClassOrClassDelegateByValue())
    {
        ParameterSymbol* returnParam = new ParameterSymbol(conversionFunctionNode.ReturnTypeExpr()->GetSpan(), U"@return");
        returnParam->SetParent(conversionFunctionSymbol);
        returnParam->SetType(returnType->AddPointer(conversionFunctionNode.GetSpan()));
        conversionFunctionSymbol->SetReturnParam(returnParam);
    }
    symbolTable.AddConversion(conversionFunctionSymbol);
    if (conversionFunctionNode.Body())
    {
        conversionFunctionNode.Body()->Accept(*this);
    }
    else
    {
        throw Exception("conversion function has no body", conversionFunctionSymbol->GetSpan());
    }
    containerScope = prevContainerScope;
    currentFunctionSymbol = prevFunctionSymbol;
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
    BindInterface(interfaceTypeSymbol, &interfaceNode, true);
}

void TypeBinder::BindInterface(InterfaceTypeSymbol* interfaceTypeSymbol, InterfaceNode* interfaceNode, bool fromOwnCompileUnit)
{
    if (interfaceTypeSymbol->IsBound()) return;
    interfaceTypeSymbol->SetBound();
    interfaceTypeSymbol->SetSpecifiers(interfaceNode->GetSpecifiers());
    if (!fromOwnCompileUnit)
    {
        AddUsingNodesToCurrentCompileUnit(interfaceNode);
    }
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
    if (delegateTypeSymbol->ReturnsClassOrClassDelegateByValue())
    {
        ParameterSymbol* returnParam = new ParameterSymbol(delegateNode.ReturnTypeExpr()->GetSpan(), U"@return");
        returnParam->SetParent(delegateTypeSymbol);
        returnParam->SetType(returnType->AddPointer(delegateNode.GetSpan()));
        delegateTypeSymbol->SetReturnParam(returnParam);
    }
    DelegateTypeDefaultConstructor* defaultConstructor = new DelegateTypeDefaultConstructor(delegateTypeSymbol);
    symbolTable.SetFunctionIdFor(defaultConstructor);
    delegateTypeSymbol->AddMember(defaultConstructor);
    DelegateTypeCopyConstructor* copyConstructor = new DelegateTypeCopyConstructor(delegateTypeSymbol);
    symbolTable.SetFunctionIdFor(copyConstructor);
    delegateTypeSymbol->AddMember(copyConstructor);
    DelegateTypeMoveConstructor* moveConstructor = new DelegateTypeMoveConstructor(delegateTypeSymbol);
    symbolTable.SetFunctionIdFor(moveConstructor);
    delegateTypeSymbol->AddMember(moveConstructor);
    DelegateTypeCopyAssignment* copyAssignment = new DelegateTypeCopyAssignment(delegateTypeSymbol, symbolTable.GetTypeByName(U"void"));
    symbolTable.SetFunctionIdFor(copyAssignment);
    delegateTypeSymbol->AddMember(copyAssignment);
    DelegateTypeMoveAssignment* moveAssignment = new DelegateTypeMoveAssignment(delegateTypeSymbol, symbolTable.GetTypeByName(U"void"));
    symbolTable.SetFunctionIdFor(moveAssignment);
    delegateTypeSymbol->AddMember(moveAssignment);
    DelegateTypeReturn* returnFun = new DelegateTypeReturn(delegateTypeSymbol);
    symbolTable.SetFunctionIdFor(returnFun);
    delegateTypeSymbol->AddMember(returnFun); 
    DelegateTypeEquality* equality = new DelegateTypeEquality(delegateTypeSymbol, symbolTable.GetTypeByName(U"bool"));
    symbolTable.SetFunctionIdFor(equality);
    delegateTypeSymbol->Ns()->AddMember(equality);
}

void TypeBinder::Visit(ClassDelegateNode& classDelegateNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&classDelegateNode);
    Assert(symbol->GetSymbolType() == SymbolType::classDelegateTypeSymbol, "class delegate type symbol expected");
    ClassDelegateTypeSymbol* classDelegateTypeSymbol = static_cast<ClassDelegateTypeSymbol*>(symbol);
    classDelegateTypeSymbol->SetSpecifiers(classDelegateNode.GetSpecifiers());
    DelegateTypeSymbol* memberDelegateType = new DelegateTypeSymbol(classDelegateNode.GetSpan(), U"@dlg_type");
    symbolTable.SetTypeIdFor(memberDelegateType);
    ParameterSymbol* objectParam = new ParameterSymbol(classDelegateNode.GetSpan(), U"@obj");
    TypeSymbol* voidPtrType = symbolTable.GetTypeByName(U"void")->AddPointer(classDelegateNode.GetSpan());
    objectParam->SetType(voidPtrType);
    memberDelegateType->AddMember(objectParam);
    int n = classDelegateNode.Parameters().Count();
    for (int i = 0; i < n; ++i)
    {
        ParameterNode* parameterNode = classDelegateNode.Parameters()[i];
        TypeSymbol* parameterType = ResolveType(parameterNode->TypeExpr(), boundCompileUnit, containerScope);
        Symbol* symbol = symbolTable.GetSymbol(parameterNode);
        Assert(symbol->GetSymbolType() == SymbolType::parameterSymbol, "parameter symbol expected");
        ParameterSymbol* parameterSymbol = static_cast<ParameterSymbol*>(symbol);
        parameterSymbol->SetType(parameterType);
        ParameterSymbol* memberParam = new ParameterSymbol(classDelegateNode.GetSpan(), ToUtf32("@p" + std::to_string(i)));
        memberParam->SetType(parameterType);
        memberDelegateType->AddMember(memberParam);
    }
    TypeSymbol* returnType = ResolveType(classDelegateNode.ReturnTypeExpr(), boundCompileUnit, containerScope);
    classDelegateTypeSymbol->SetReturnType(returnType);
    if (classDelegateTypeSymbol->ReturnsClassOrClassDelegateByValue())
    {
        ParameterSymbol* returnParam = new ParameterSymbol(classDelegateNode.ReturnTypeExpr()->GetSpan(), U"@return");
        returnParam->SetParent(classDelegateTypeSymbol);
        returnParam->SetType(returnType->AddPointer(classDelegateNode.GetSpan()));
        classDelegateTypeSymbol->SetReturnParam(returnParam);
        ParameterSymbol* memberReturnParam = new ParameterSymbol(classDelegateNode.ReturnTypeExpr()->GetSpan(), U"@return");
        memberReturnParam->SetParent(memberDelegateType);
        memberReturnParam->SetType(returnType->AddPointer(classDelegateNode.GetSpan()));
        memberDelegateType->SetReturnParam(memberReturnParam);
    }
    memberDelegateType->SetReturnType(returnType);
    classDelegateTypeSymbol->AddMember(memberDelegateType);
    ClassTypeSymbol* objectDelegatePairType = new ClassTypeSymbol(classDelegateNode.GetSpan(), U"@objectDelegatePairType");
    MemberVariableSymbol* objVar = new MemberVariableSymbol(classDelegateNode.GetSpan(), U"@obj");
    objVar->SetType(voidPtrType);
    MemberVariableSymbol* dlgVar = new MemberVariableSymbol(classDelegateNode.GetSpan(), U"@dlg");
    dlgVar->SetType(memberDelegateType);
    objectDelegatePairType->AddMember(objVar);
    objectDelegatePairType->AddMember(dlgVar);
    symbolTable.SetTypeIdFor(objectDelegatePairType);
    objectDelegatePairType->CreateLayouts();
    classDelegateTypeSymbol->AddMember(objectDelegatePairType);
    ClassDelegateTypeDefaultConstructor* defaultConstructor = new ClassDelegateTypeDefaultConstructor(classDelegateTypeSymbol);
    symbolTable.SetFunctionIdFor(defaultConstructor);
    classDelegateTypeSymbol->AddMember(defaultConstructor);
    ClassDelegateTypeCopyConstructor* copyConstructor = new ClassDelegateTypeCopyConstructor(classDelegateTypeSymbol);
    symbolTable.SetFunctionIdFor(copyConstructor);
    classDelegateTypeSymbol->AddMember(copyConstructor);
    ClassDelegateTypeMoveConstructor* moveConstructor = new ClassDelegateTypeMoveConstructor(classDelegateTypeSymbol);
    symbolTable.SetFunctionIdFor(moveConstructor);
    classDelegateTypeSymbol->AddMember(moveConstructor);
    ClassDelegateTypeCopyAssignment* copyAssignment = new ClassDelegateTypeCopyAssignment(classDelegateTypeSymbol, symbolTable.GetTypeByName(U"void"));
    symbolTable.SetFunctionIdFor(copyAssignment);
    classDelegateTypeSymbol->AddMember(copyAssignment);
    ClassDelegateTypeMoveAssignment* moveAssignment = new ClassDelegateTypeMoveAssignment(classDelegateTypeSymbol, symbolTable.GetTypeByName(U"void"));
    symbolTable.SetFunctionIdFor(moveAssignment);
    classDelegateTypeSymbol->AddMember(moveAssignment);
    ClassDelegateTypeEquality* equality = new ClassDelegateTypeEquality(classDelegateTypeSymbol, symbolTable.GetTypeByName(U"bool"));
    symbolTable.SetFunctionIdFor(equality);
    classDelegateTypeSymbol->Ns()->AddMember(equality);
}

void TypeBinder::Visit(ConceptNode& conceptNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&conceptNode);
    Assert(symbol->GetSymbolType() == SymbolType::conceptSymbol, "concept symbol expected");
    ConceptSymbol* conceptSymbol = static_cast<ConceptSymbol*>(symbol);
    BindConcept(conceptSymbol, &conceptNode);
}

void TypeBinder::BindConcept(ConceptSymbol* conceptSymbol, ConceptNode* conceptNode)
{
    if (conceptSymbol->IsBound()) return;
    conceptSymbol->SetBound();
    conceptSymbol->SetSpecifiers(conceptNode->GetSpecifiers());
    conceptSymbol->ComputeName();
    if (conceptNode->Refinement())
    {
        ConceptIdNode* refinedConceptIdNode = conceptNode->Refinement();
        ConceptSymbol* refinedConceptSymbol = ResolveConceptId(refinedConceptIdNode, boundCompileUnit, containerScope);
        if (refinedConceptSymbol->IsProject())
        {
            Node* node = symbolTable.GetNode(refinedConceptSymbol);
            Assert(node->GetNodeType() == NodeType::conceptNode, "concept node expected");
            ConceptNode* refinedConceptNode = static_cast<ConceptNode*>(node);
            BindConcept(refinedConceptSymbol, refinedConceptNode);
        }
        conceptSymbol->SetRefinedConcept(refinedConceptSymbol);
    }
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

void TypeBinder::Visit(TryStatementNode& tryStatementNode)
{
    currentFunctionSymbol->SetHasTry();
    tryStatementNode.TryBlock()->Accept(*this);
    int n = tryStatementNode.Catches().Count();
    for (int i = 0; i < n; ++i)
    {
        CatchNode* catchNode = tryStatementNode.Catches()[i];
        catchNode->Accept(*this);
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
        if (type->BaseType()->IsClassTypeSymbol())
        {
            ClassTypeSymbol* exceptionVarClassType = static_cast<ClassTypeSymbol*>(type->BaseType());
            IdentifierNode systemExceptionNode(catchNode.GetSpan(), U"System.Exception");
            TypeSymbol* systemExceptionType = ResolveType(&systemExceptionNode, boundCompileUnit, containerScope);
            Assert(systemExceptionType->IsClassTypeSymbol(), "System.Exception not of class type");
            ClassTypeSymbol* systemExceptionClassType = static_cast<ClassTypeSymbol*>(systemExceptionType);
            if (exceptionVarClassType->IsProject())
            {
                Node* exceptionVarNode = symbolTable.GetNode(exceptionVarClassType);
                Assert(exceptionVarNode->GetNodeType() == NodeType::classNode, "class node expected");
                ClassNode* exceptionVarClassNode = static_cast<ClassNode*>(exceptionVarNode);
                BindClass(exceptionVarClassType, exceptionVarClassNode, false);
            }
            if (exceptionVarClassType == systemExceptionClassType || exceptionVarClassType->HasBaseClass(systemExceptionClassType))
            {
                exceptionVarSymbol->SetType(type);
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

void TypeBinder::Visit(TypedefNode& typedefNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&typedefNode);
    Assert(symbol->GetSymbolType() == SymbolType::typedefSymbol, "typedef symbol expected");
    TypedefSymbol* typedefSymbol = static_cast<TypedefSymbol*>(symbol);
    BindTypedef(typedefSymbol, &typedefNode, true);
}

void TypeBinder::BindTypedef(TypedefSymbol* typedefSymbol, TypedefNode* typedefNode, bool fromOwnCompileUnit)
{
    if (typedefSymbol->IsBound()) return;
    typedefSymbol->SetBound();
    typedefSymbol->SetSpecifiers(typedefNode->GetSpecifiers());
    if (!fromOwnCompileUnit)
    {
        AddUsingNodesToCurrentCompileUnit(typedefNode);
    }
    TypeSymbol* typeSymbol = ResolveType(typedefNode->TypeExpr(), boundCompileUnit, containerScope);
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
    std::unique_ptr<Value> value = Evaluate(constantNode.Value(), GetValueTypeFor(typeSymbol->GetSymbolType()), containerScope, boundCompileUnit, false);
    constantSymbol->SetValue(value.release());
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
    EnumTypeDefaultConstructor* defaultConstructor = new EnumTypeDefaultConstructor(enumTypeSymbol);
    symbolTable.SetFunctionIdFor(defaultConstructor);
    enumTypeSymbol->AddMember(defaultConstructor);
    EnumTypeCopyConstructor* copyConstructor = new EnumTypeCopyConstructor(enumTypeSymbol);
    symbolTable.SetFunctionIdFor(copyConstructor);
    enumTypeSymbol->AddMember(copyConstructor);
    EnumTypeMoveConstructor* moveConstructor = new EnumTypeMoveConstructor(enumTypeSymbol);
    symbolTable.SetFunctionIdFor(moveConstructor);
    enumTypeSymbol->AddMember(moveConstructor);
    EnumTypeCopyAssignment* copyAssignment = new EnumTypeCopyAssignment(enumTypeSymbol, symbolTable.GetTypeByName(U"void"));
    symbolTable.SetFunctionIdFor(copyAssignment);
    enumTypeSymbol->AddMember(copyAssignment);
    EnumTypeMoveAssignment* moveAssignment = new EnumTypeMoveAssignment(enumTypeSymbol, symbolTable.GetTypeByName(U"void"));
    symbolTable.SetFunctionIdFor(moveAssignment);
    enumTypeSymbol->AddMember(moveAssignment);
    EnumTypeReturn* returnFun = new EnumTypeReturn(enumTypeSymbol);
    symbolTable.SetFunctionIdFor(returnFun);
    enumTypeSymbol->AddMember(returnFun);
    EnumTypeEqualityOp* equality = new EnumTypeEqualityOp(enumTypeSymbol, symbolTable.GetTypeByName(U"bool"));
    symbolTable.SetFunctionIdFor(equality);
    enumTypeSymbol->Ns()->AddMember(equality);
    EnumTypeToUnderlyingTypeConversion* enum2underlying = new EnumTypeToUnderlyingTypeConversion(enumTypeNode.GetSpan(), U"enum2underlying", enumTypeSymbol, underlyingType);
    symbolTable.SetFunctionIdFor(enum2underlying);
    symbolTable.AddConversion(enum2underlying);
    enumTypeSymbol->AddMember(enum2underlying);
    UnderlyingTypeToEnumTypeConversion* underlying2enum = new UnderlyingTypeToEnumTypeConversion(enumTypeNode.GetSpan(), U"underlying2enum", underlyingType, enumTypeSymbol);
    symbolTable.SetFunctionIdFor(underlying2enum);
    symbolTable.AddConversion(underlying2enum);
    enumTypeSymbol->AddMember(underlying2enum);
    containerScope = prevContainerScope;
    enumType = prevEnumType;
}

void TypeBinder::Visit(EnumConstantNode& enumConstantNode)
{
    Symbol* symbol = symbolTable.GetSymbol(&enumConstantNode);
    Assert(symbol->GetSymbolType() == SymbolType::enumConstantSymbol, "enumeration constant symbol expected");
    EnumConstantSymbol* enumConstantSymbol = static_cast<EnumConstantSymbol*>(symbol);
    enumConstantSymbol->SetEvaluating();
    std::unique_ptr<Value> value = Evaluate(enumConstantNode.GetValue(), GetValueTypeFor(enumType->UnderlyingType()->GetSymbolType()), containerScope, boundCompileUnit, false);
    enumConstantSymbol->SetValue(value.release());
    enumConstantSymbol->ResetEvaluating();
}

} } // namespace cmajor::binder
