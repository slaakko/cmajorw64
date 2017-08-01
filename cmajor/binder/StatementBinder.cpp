// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/StatementBinder.hpp>
#include <cmajor/binder/BoundClass.hpp>
#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/binder/BoundStatement.hpp>
#include <cmajor/binder/BoundExpression.hpp>
#include <cmajor/binder/OverloadResolution.hpp>
#include <cmajor/binder/ExpressionBinder.hpp>
#include <cmajor/binder/Access.hpp>
#include <cmajor/binder/OperationRepository.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/ast/Literal.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::unicode;

bool IsAlwaysTrue(const Node* node, BoundCompileUnit& boundCompileUnit, ContainerScope* containerScope)
{
    // todo
    return false;
}

bool TerminatesFunction(StatementNode* statement, bool inForEverLoop, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit)
{
    switch (statement->GetNodeType())
    {
        case NodeType::compoundStatementNode:
        {
            CompoundStatementNode* compoundStatement = static_cast<CompoundStatementNode*>(statement);
            int n = compoundStatement->Statements().Count();
            for (int i = 0; i < n; ++i)
            {
                StatementNode* statement = compoundStatement->Statements()[i];
                if (TerminatesFunction(statement, inForEverLoop, containerScope, boundCompileUnit)) return true;
            }
            break;
        }
        case NodeType::ifStatementNode:
        {
            IfStatementNode* ifStatement = static_cast<IfStatementNode*>(statement);
            if (inForEverLoop || ifStatement->ElseS())
            {
                if (TerminatesFunction(ifStatement->ThenS(), inForEverLoop, containerScope, boundCompileUnit) &&
                    inForEverLoop || (ifStatement->ElseS() && TerminatesFunction(ifStatement->ElseS(), inForEverLoop, containerScope, boundCompileUnit)))
                {
                    return true;
                }
            }
            break;
        }
        case NodeType::whileStatementNode:
        {
            WhileStatementNode* whileStatement = static_cast<WhileStatementNode*>(statement);
            if (IsAlwaysTrue(whileStatement->Condition(), boundCompileUnit, containerScope))
            {
                if (TerminatesFunction(whileStatement->Statement(), true, containerScope, boundCompileUnit)) return true;
            }
            break;
        }
        case NodeType::doStatementNode:
        {
            DoStatementNode* doStatement = static_cast<DoStatementNode*>(statement);
            if (IsAlwaysTrue(doStatement->Condition(), boundCompileUnit, containerScope))
            {
                if (TerminatesFunction(doStatement->Statement(), true, containerScope, boundCompileUnit)) return true;
            }
            break;
        }
        case NodeType::forStatementNode:
        {
            ForStatementNode* forStatement = static_cast<ForStatementNode*>(statement);
            if (!forStatement->Condition() || IsAlwaysTrue(forStatement->Condition(), boundCompileUnit, containerScope))
            {
                if (TerminatesFunction(forStatement->ActionS(), true, containerScope, boundCompileUnit)) return true;
            }
            break;
        }
        default:
        {
            if (statement->IsFunctionTerminatingNode())
            {
                return true;
            }
            break;
        }
    }
    return false;
}

void CheckFunctionReturnPaths(FunctionSymbol* functionSymbol, CompoundStatementNode* bodyNode, const Span& span, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit);

void CheckFunctionReturnPaths(FunctionSymbol* functionSymbol, FunctionNode& functionNode, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit)
{
    CheckFunctionReturnPaths(functionSymbol, functionNode.Body(), functionNode.GetSpan(), containerScope, boundCompileUnit);
}

void CheckFunctionReturnPaths(FunctionSymbol* functionSymbol, CompoundStatementNode* bodyNode, const Span& span, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit)
{
    TypeSymbol* returnType = functionSymbol->ReturnType();
    if (!returnType || returnType->GetSymbolType() == SymbolType::voidTypeSymbol) return;
    if (functionSymbol->IsExternal()) return;
    if (functionSymbol->IsAbstract()) return;
    CompoundStatementNode* body = bodyNode;
    int n = body->Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statement = body->Statements()[i];
        if (TerminatesFunction(statement, false, containerScope, boundCompileUnit)) return;
    }
    throw Exception("not all control paths terminate in return or throw statement", span);
}

StatementBinder::StatementBinder(BoundCompileUnit& boundCompileUnit_) :  
    boundCompileUnit(boundCompileUnit_), symbolTable(boundCompileUnit.GetSymbolTable()), containerScope(nullptr), statement(), currentClass(nullptr), currentFunction(nullptr), 
    currentConstructorSymbol(nullptr), currentConstructorNode(nullptr), currentDestructorSymbol(nullptr), currentDestructorNode(nullptr), currentMemberFunctionSymbol(nullptr), 
    currentMemberFunctionNode(nullptr), postfix(false)
{
}

void StatementBinder::Visit(CompileUnitNode& compileUnitNode)
{
    compileUnitNode.GlobalNs()->Accept(*this);
}

void StatementBinder::Visit(NamespaceNode& namespaceNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetSymbolTable().GetSymbol(&namespaceNode);
    containerScope = symbol->GetContainerScope();
    int n = namespaceNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* member = namespaceNode.Members()[i];
        member->Accept(*this);
    }
    containerScope = prevContainerScope;
}

void StatementBinder::Visit(ClassNode& classNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetSymbolTable().GetSymbol(&classNode);
    Assert(symbol->GetSymbolType() == SymbolType::classTypeSymbol || symbol->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol, "class type symbol expected");
    ClassTypeSymbol* classTypeSymbol = static_cast<ClassTypeSymbol*>(symbol);
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundClass> boundClass(new BoundClass(classTypeSymbol));
    currentClass = boundClass.get();
    int n = classNode.Members().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* classMember = classNode.Members()[i];
        classMember->Accept(*this);
    }
    boundCompileUnit.AddBoundNode(std::move(boundClass));
    if (classTypeSymbol->HasNontrivialDestructor())
    {
        classTypeSymbol->CreateDestructorSymbol();
    }
    DestructorSymbol* destructorSymbol = classTypeSymbol->Destructor();
    if (destructorSymbol)
    {
        Node* node = symbolTable.GetNodeNoThrow(destructorSymbol);
        if (!node)
        {
            GenerateDestructorImplementation(currentClass, destructorSymbol, boundCompileUnit, containerScope, classNode.GetSpan());
        }
    }
    containerScope = prevContainerScope;
}

void StatementBinder::Visit(FunctionNode& functionNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetSymbolTable().GetSymbol(&functionNode);
    Assert(symbol->GetSymbolType() == SymbolType::functionSymbol, "function symbol expected");
    FunctionSymbol* functionSymbol = static_cast<FunctionSymbol*>(symbol);
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(functionSymbol));
    BoundFunction* prevFunction = currentFunction;
    currentFunction = boundFunction.get();
    if (functionNode.Body())
    {
        functionNode.Body()->Accept(*this);
        BoundStatement* boundStatement = statement.release();
        Assert(boundStatement->GetBoundNodeType() == BoundNodeType::boundCompoundStatement, "bound compound statement expected");
        BoundCompoundStatement* compoundStatement = static_cast<BoundCompoundStatement*>(boundStatement);
        boundFunction->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    }
    CheckFunctionReturnPaths(functionSymbol, functionNode, containerScope, boundCompileUnit);
    boundCompileUnit.AddBoundNode(std::move(boundFunction));
    currentFunction = prevFunction;
    containerScope = prevContainerScope;
}

void StatementBinder::Visit(StaticConstructorNode& staticConstructorNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetSymbolTable().GetSymbol(&staticConstructorNode);
    Assert(symbol->GetSymbolType() == SymbolType::staticConstructorSymbol , "static constructor symbol expected");
    StaticConstructorSymbol* staticConstructorSymbol = static_cast<StaticConstructorSymbol*>(symbol);
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(staticConstructorSymbol));
    BoundFunction* prevFunction = currentFunction;
    currentFunction = boundFunction.get();
    if (staticConstructorNode.Body())
    {
        staticConstructorNode.Body()->Accept(*this);
        BoundStatement* boundStatement = statement.release();
        Assert(boundStatement->GetBoundNodeType() == BoundNodeType::boundCompoundStatement, "bound compound statement expected");
        BoundCompoundStatement* compoundStatement = static_cast<BoundCompoundStatement*>(boundStatement);
        boundFunction->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    }
    CheckFunctionReturnPaths(staticConstructorSymbol, staticConstructorNode, containerScope, boundCompileUnit);
    currentClass->AddMember(std::move(boundFunction));
    currentFunction = prevFunction;
    containerScope = prevContainerScope;
}

void StatementBinder::Visit(ConstructorNode& constructorNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetSymbolTable().GetSymbol(&constructorNode);
    Assert(symbol->GetSymbolType() == SymbolType::constructorSymbol, "constructor symbol expected");
    ConstructorSymbol* constructorSymbol = static_cast<ConstructorSymbol*>(symbol);
    ConstructorSymbol* prevConstructorSymbol = currentConstructorSymbol;
    currentConstructorSymbol = constructorSymbol;
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(constructorSymbol));
    BoundFunction* prevFunction = currentFunction;
    currentFunction = boundFunction.get();
    if (constructorNode.Body())
    {
        ConstructorNode* prevConstructorNode = currentConstructorNode;
        currentConstructorNode = &constructorNode;
        constructorNode.Body()->Accept(*this);
        currentConstructorNode = prevConstructorNode;
        BoundStatement* boundStatement = statement.release();
        Assert(boundStatement->GetBoundNodeType() == BoundNodeType::boundCompoundStatement, "bound compound statement expected");
        BoundCompoundStatement* compoundStatement = static_cast<BoundCompoundStatement*>(boundStatement);
        boundFunction->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    }
    else if (constructorSymbol->IsDefault())
    {
        ConstructorNode* prevConstructorNode = currentConstructorNode;
        currentConstructorNode = &constructorNode;
        std::unique_ptr<BoundCompoundStatement> boundCompoundStatement(new BoundCompoundStatement(constructorNode.GetSpan()));
        GenerateClassInitialization(currentConstructorSymbol, currentConstructorNode, boundCompoundStatement.get(), currentFunction, boundCompileUnit, containerScope, this, true);
        currentConstructorNode = prevConstructorNode;
        boundFunction->SetBody(std::move(boundCompoundStatement));
    }
    CheckFunctionReturnPaths(constructorSymbol, constructorNode, containerScope, boundCompileUnit);
    currentClass->AddMember(std::move(boundFunction));
    currentFunction = prevFunction;
    containerScope = prevContainerScope;
    currentConstructorSymbol = prevConstructorSymbol;
}

void StatementBinder::Visit(DestructorNode& destructorNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetSymbolTable().GetSymbol(&destructorNode);
    Assert(symbol->GetSymbolType() == SymbolType::destructorSymbol, "destructor symbol expected");
    DestructorSymbol* destructorSymbol = static_cast<DestructorSymbol*>(symbol);
    DestructorSymbol* prevDestructorSymbol = currentDestructorSymbol;
    currentDestructorSymbol = destructorSymbol;
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(destructorSymbol));
    BoundFunction* prevFunction = currentFunction;
    currentFunction = boundFunction.get();
    if (destructorNode.Body())
    {
        DestructorNode* prevDestructorNode = currentDestructorNode;
        currentDestructorNode = &destructorNode;
        destructorNode.Body()->Accept(*this);
        currentDestructorNode = prevDestructorNode;
        BoundStatement* boundStatement = statement.release();
        Assert(boundStatement->GetBoundNodeType() == BoundNodeType::boundCompoundStatement, "bound compound statement expected");
        BoundCompoundStatement* compoundStatement = static_cast<BoundCompoundStatement*>(boundStatement);
        boundFunction->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    }
    else if (destructorSymbol->IsDefault())
    {
        DestructorNode* prevDestructorNode = currentDestructorNode;
        currentDestructorNode = &destructorNode;
        std::unique_ptr<BoundCompoundStatement> boundCompoundStatement(new BoundCompoundStatement(destructorNode.GetSpan()));
        GenerateClassTermination(currentDestructorSymbol, currentDestructorNode, boundCompoundStatement.get(), currentFunction, boundCompileUnit, containerScope, this);
        currentDestructorNode = prevDestructorNode;
        boundFunction->SetBody(std::move(boundCompoundStatement));
    }
    CheckFunctionReturnPaths(destructorSymbol, destructorNode, containerScope, boundCompileUnit);
    currentClass->AddMember(std::move(boundFunction));
    currentFunction = prevFunction;
    containerScope = prevContainerScope;
    currentDestructorSymbol = prevDestructorSymbol;
}

void StatementBinder::Visit(MemberFunctionNode& memberFunctionNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetSymbolTable().GetSymbol(&memberFunctionNode);
    Assert(symbol->GetSymbolType() == SymbolType::memberFunctionSymbol, "member function symbol expected");
    MemberFunctionSymbol* memberFunctionSymbol = static_cast<MemberFunctionSymbol*>(symbol);
    MemberFunctionSymbol* prevMemberFunctionSymbol = memberFunctionSymbol;
    currentMemberFunctionSymbol = memberFunctionSymbol;
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(memberFunctionSymbol));
    BoundFunction* prevFunction = currentFunction;
    currentFunction = boundFunction.get();
    if (memberFunctionNode.Body())
    {
        MemberFunctionNode* prevMemberFunctionNode = currentMemberFunctionNode;
        currentMemberFunctionNode = &memberFunctionNode;
        memberFunctionNode.Body()->Accept(*this);
        currentMemberFunctionNode = prevMemberFunctionNode;
        BoundStatement* boundStatement = statement.release();
        Assert(boundStatement->GetBoundNodeType() == BoundNodeType::boundCompoundStatement, "bound compound statement expected");
        BoundCompoundStatement* compoundStatement = static_cast<BoundCompoundStatement*>(boundStatement);
        boundFunction->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
    }
    else if (memberFunctionSymbol->IsDefault())
    {
        Assert(memberFunctionSymbol->GroupName() == U"operator=", "operator= expected");
        MemberFunctionNode* prevMemberFunctionNode = currentMemberFunctionNode;
        currentMemberFunctionNode = &memberFunctionNode;
        std::unique_ptr<BoundCompoundStatement> boundCompoundStatement(new BoundCompoundStatement(memberFunctionNode.GetSpan()));
        GenerateClassAssignment(currentMemberFunctionSymbol, currentMemberFunctionNode, boundCompoundStatement.get(), currentFunction, boundCompileUnit, containerScope, this, true);
        currentMemberFunctionNode = prevMemberFunctionNode;
        boundFunction->SetBody(std::move(boundCompoundStatement));
    }
    CheckFunctionReturnPaths(memberFunctionSymbol, memberFunctionNode, containerScope, boundCompileUnit);
    currentClass->AddMember(std::move(boundFunction));
    currentFunction = prevFunction;
    containerScope = prevContainerScope;
    currentMemberFunctionSymbol = prevMemberFunctionSymbol;
}

void StatementBinder::Visit(CompoundStatementNode& compoundStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = symbolTable.GetSymbol(&compoundStatementNode);
    Assert(symbol->GetSymbolType() == SymbolType::declarationBlock, "declaration block expected");
    DeclarationBlock* declarationBlock = static_cast<DeclarationBlock*>(symbol);
    containerScope = declarationBlock->GetContainerScope();
    std::unique_ptr<BoundCompoundStatement> boundCompoundStatement(new BoundCompoundStatement(compoundStatementNode.GetSpan()));
    if (currentConstructorSymbol && currentConstructorNode)
    {
        GenerateClassInitialization(currentConstructorSymbol, currentConstructorNode, boundCompoundStatement.get(), currentFunction, boundCompileUnit, containerScope, this, false);
    }
    else if (currentMemberFunctionSymbol && currentMemberFunctionSymbol->GroupName() == U"operator=" && currentMemberFunctionNode)
    {
        GenerateClassAssignment(currentMemberFunctionSymbol, currentMemberFunctionNode, boundCompoundStatement.get(), currentFunction, boundCompileUnit, containerScope, this, false);
    }
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
        boundCompoundStatement->AddStatement(std::move(statement));
    }
    if (currentDestructorSymbol && currentDestructorNode)
    {
        GenerateClassTermination(currentDestructorSymbol, currentDestructorNode, boundCompoundStatement.get(), currentFunction, boundCompileUnit, containerScope, this);
    }
    AddStatement(boundCompoundStatement.release());
    if (compoundStatementNode.Label())
    {
        statement->SetLabel(compoundStatementNode.Label()->Label());
    }
    containerScope = prevContainerScope;
}

void StatementBinder::Visit(ReturnStatementNode& returnStatementNode) 
{
    if (returnStatementNode.Expression())
    {
        TypeSymbol* returnType = currentFunction->GetFunctionSymbol()->ReturnType();
        bool returnDelegateType = false;
        bool returnClassDelegateType = false;
        if (returnType)
        {
            returnDelegateType = returnType->GetSymbolType() == SymbolType::delegateTypeSymbol;
            returnClassDelegateType = returnType->GetSymbolType() == SymbolType::classDelegateTypeSymbol;
        }
        if (returnType && returnType->GetSymbolType() != SymbolType::voidTypeSymbol)
        {
            std::vector<std::unique_ptr<BoundExpression>> returnTypeArgs;
            BoundTypeExpression* boundTypeExpression = new BoundTypeExpression(returnStatementNode.GetSpan(), returnType);
            returnTypeArgs.push_back(std::unique_ptr<BoundTypeExpression>(boundTypeExpression));
            std::vector<FunctionScopeLookup> functionScopeLookups;
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, returnType->ClassInterfaceOrNsScope()));
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
            std::unique_ptr<BoundFunctionCall> returnFunctionCall = ResolveOverload(U"@return", containerScope, functionScopeLookups, returnTypeArgs, boundCompileUnit, currentFunction, 
                returnStatementNode.GetSpan());
            std::unique_ptr<BoundExpression> expression = BindExpression(returnStatementNode.Expression(), boundCompileUnit, currentFunction, containerScope, this, false,
                returnDelegateType || returnClassDelegateType, returnClassDelegateType);
            std::vector<std::unique_ptr<BoundExpression>> returnValueArguments;
            returnValueArguments.push_back(std::move(expression));
            FunctionMatch functionMatch(returnFunctionCall->GetFunctionSymbol());
            bool conversionFound = FindConversions(boundCompileUnit, returnFunctionCall->GetFunctionSymbol(), returnValueArguments, functionMatch, ConversionType::implicit_, 
                returnStatementNode.GetSpan());
            if (conversionFound)
            {
                Assert(!functionMatch.argumentMatches.empty(), "argument match expected");
                ArgumentMatch argumentMatch = functionMatch.argumentMatches[0];
                FunctionSymbol* conversionFun = argumentMatch.conversionFun;
                if (conversionFun)
                {
                    BoundConversion* boundConversion = new BoundConversion(std::unique_ptr<BoundExpression>(returnValueArguments[0].release()), conversionFun);
                    returnValueArguments[0].reset(boundConversion);
                }
                if (argumentMatch.referenceConversionFlags != OperationFlags::none)
                {
                    if (argumentMatch.referenceConversionFlags == OperationFlags::addr)
                    {
                        TypeSymbol* type = returnValueArguments[0]->GetType()->AddLvalueReference(returnStatementNode.GetSpan());
                        BoundAddressOfExpression* addressOfExpression = new BoundAddressOfExpression(std::move(returnValueArguments[0]), type);
                        returnValueArguments[0].reset(addressOfExpression);
                    }
                    else if (argumentMatch.referenceConversionFlags == OperationFlags::deref)
                    {
                        TypeSymbol* type = returnValueArguments[0]->GetType()->RemoveReference(returnStatementNode.GetSpan());
                        BoundDereferenceExpression* dereferenceExpression = new BoundDereferenceExpression(std::move(returnValueArguments[0]), type);
                        returnValueArguments[0].reset(dereferenceExpression);
                    }
                }
                returnFunctionCall->SetArguments(std::move(returnValueArguments));
            }
            else
            {
                throw Exception("no implicit conversion from '" + ToUtf8(returnValueArguments[0]->GetType()->FullName()) + "' to '" + ToUtf8(returnType->FullName()) + "' exists",
                    returnStatementNode.GetSpan(), currentFunction->GetFunctionSymbol()->GetSpan());
            }
            CheckAccess(currentFunction->GetFunctionSymbol(), returnFunctionCall->GetFunctionSymbol());
            AddStatement(new BoundReturnStatement(std::move(returnFunctionCall)));
        }
        else
        {
            if (returnType)
            {
                throw Exception("void function cannot return a value", returnStatementNode.Expression()->GetSpan(), currentFunction->GetFunctionSymbol()->GetSpan());
            }
            else
            {
                throw Exception("constructor or assignment function cannot return a value", returnStatementNode.Expression()->GetSpan(), currentFunction->GetFunctionSymbol()->GetSpan());
            }
        }
    }
    else
    {
        TypeSymbol* returnType = currentFunction->GetFunctionSymbol()->ReturnType();
        if (!returnType || returnType->GetSymbolType() == SymbolType::voidTypeSymbol)
        {
            std::unique_ptr<BoundFunctionCall> returnFunctionCall;
            AddStatement(new BoundReturnStatement(std::move(returnFunctionCall)));
        }
        else
        {
            throw Exception("nonvoid function must return a value", returnStatementNode.GetSpan(), currentFunction->GetFunctionSymbol()->GetSpan());
        }
    }
    if (returnStatementNode.Label())
    {
        statement->SetLabel(returnStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(IfStatementNode& ifStatementNode)
{
    std::unique_ptr<BoundExpression> condition = BindExpression(ifStatementNode.Condition(), boundCompileUnit, currentFunction, containerScope, this);
    if (!TypesEqual(symbolTable.GetTypeByName(U"bool"), condition->GetType()->PlainType(ifStatementNode.GetSpan())))
    {
        throw Exception("condition of an if statement must be a Boolean expression", ifStatementNode.Condition()->GetSpan());
    }
    ifStatementNode.ThenS()->Accept(*this);
    BoundStatement* thenS = statement.release();
    BoundStatement* elseS = nullptr;
    if (ifStatementNode.ElseS())
    {
        ifStatementNode.ElseS()->Accept(*this);
        elseS = statement.release();
    }
    AddStatement(new BoundIfStatement(ifStatementNode.GetSpan(), std::move(condition), std::unique_ptr<BoundStatement>(thenS), std::unique_ptr<BoundStatement>(elseS)));
    if (ifStatementNode.Label())
    {
        statement->SetLabel(ifStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(WhileStatementNode& whileStatementNode)
{
    std::unique_ptr<BoundExpression> condition = BindExpression(whileStatementNode.Condition(), boundCompileUnit, currentFunction, containerScope, this);
    if (!TypesEqual(symbolTable.GetTypeByName(U"bool"), condition->GetType()->PlainType(whileStatementNode.GetSpan())))
    {
        throw Exception("condition of a while statement must be a Boolean expression", whileStatementNode.Condition()->GetSpan());
    }
    whileStatementNode.Statement()->Accept(*this);
    AddStatement(new BoundWhileStatement(whileStatementNode.GetSpan(), std::move(condition), std::unique_ptr<BoundStatement>(statement.release())));
    if (whileStatementNode.Label())
    {
        statement->SetLabel(whileStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(DoStatementNode& doStatementNode)
{
    std::unique_ptr<BoundExpression> condition = BindExpression(doStatementNode.Condition(), boundCompileUnit, currentFunction, containerScope, this);
    if (!TypesEqual(symbolTable.GetTypeByName(U"bool"), condition->GetType()->PlainType(doStatementNode.GetSpan())))
    {
        throw Exception("condition of a do statement must be a Boolean expression", doStatementNode.Condition()->GetSpan());
    }
    doStatementNode.Statement()->Accept(*this);
    AddStatement(new BoundDoStatement(doStatementNode.GetSpan(), std::unique_ptr<BoundStatement>(statement.release()), std::move(condition)));
    if (doStatementNode.Label())
    {
        statement->SetLabel(doStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(ForStatementNode& forStatementNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetSymbolTable().GetSymbol(&forStatementNode);
    Assert(symbol->GetSymbolType() == SymbolType::declarationBlock, "declaration block expected");
    DeclarationBlock* declarationBlock = static_cast<DeclarationBlock*>(symbol);
    containerScope = declarationBlock->GetContainerScope();
    std::unique_ptr<BoundExpression> condition;
    if (forStatementNode.Condition())
    {
        condition = BindExpression(forStatementNode.Condition(), boundCompileUnit, currentFunction, containerScope, this);
    }
    else
    {
        BooleanLiteralNode trueNode(forStatementNode.GetSpan(), true);
        condition = BindExpression(&trueNode, boundCompileUnit, currentFunction, containerScope, this);
    }
    if (!TypesEqual(symbolTable.GetTypeByName(U"bool"), condition->GetType()->PlainType(forStatementNode.GetSpan())))
    {
        throw Exception("condition of a for statement must be a Boolean expression", forStatementNode.Condition()->GetSpan());
    }
    forStatementNode.InitS()->Accept(*this);
    BoundStatement* initS = statement.release();
    forStatementNode.LoopS()->Accept(*this);
    BoundStatement* loopS = statement.release();
    forStatementNode.ActionS()->Accept(*this);
    BoundStatement* actionS = statement.release();
    AddStatement(new BoundForStatement(forStatementNode.GetSpan(), std::unique_ptr<BoundStatement>(initS), std::move(condition), std::unique_ptr<BoundStatement>(loopS), 
        std::unique_ptr<BoundStatement>(actionS)));
    if (forStatementNode.Label())
    {
        statement->SetLabel(forStatementNode.Label()->Label());
    }
    containerScope = prevContainerScope;
}

void StatementBinder::Visit(BreakStatementNode& breakStatementNode)
{
    const Node* parent = breakStatementNode.Parent();
    const StatementNode* parentStatement = nullptr;
    if (parent && parent->IsStatementNode())
    {
        parentStatement = static_cast<const StatementNode*>(parent);
    }
    while (parentStatement && !parentStatement->IsBreakEnclosingStatementNode())
    {
        parent = parentStatement->Parent();
        if (parent && parent->IsStatementNode())
        {
            parentStatement = static_cast<const StatementNode*>(parent);
        }
        else
        {
            parentStatement = nullptr;
        }
    }
    if (!parentStatement)
    {
        throw Exception("break statement must be enclosed in a while, do, for or switch statement", breakStatementNode.GetSpan());
    }
    AddStatement(new BoundBreakStatement(breakStatementNode.GetSpan()));
    if (breakStatementNode.Label())
    {
        statement->SetLabel(breakStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(ContinueStatementNode& continueStatementNode)
{
    const Node* parent = continueStatementNode.Parent();
    const StatementNode* parentStatement = nullptr;
    if (parent && parent->IsStatementNode())
    {
        parentStatement = static_cast<const StatementNode*>(parent);
    }
    while (parentStatement && !parentStatement->IsContinueEnclosingStatementNode())
    {
        parent = parentStatement->Parent();
        if (parent && parent->IsStatementNode())
        {
            parentStatement = static_cast<const StatementNode*>(parent);
        }
        else
        {
            parentStatement = nullptr;
        }
    }
    if (!parentStatement)
    {
        throw Exception("continue statement must be enclosed in a while, do or for statement", continueStatementNode.GetSpan());
    }
    AddStatement(new BoundContinueStatement(continueStatementNode.GetSpan()));
    if (continueStatementNode.Label())
    {
        statement->SetLabel(continueStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(GotoStatementNode& gotoStatementNode)
{
    currentFunction->SetHasGotos();
    boundCompileUnit.SetHasGotos();
    AddStatement(new BoundGotoStatement(gotoStatementNode.GetSpan(), gotoStatementNode.Target()));
    if (gotoStatementNode.Label())
    {
        statement->SetLabel(gotoStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(ConstructionStatementNode& constructionStatementNode)
{
    Symbol* symbol = boundCompileUnit.GetSymbolTable().GetSymbol(&constructionStatementNode);
    Assert(symbol->GetSymbolType() == SymbolType::localVariableSymbol, "local variable symbol expected");
    LocalVariableSymbol* localVariableSymbol = static_cast<LocalVariableSymbol*>(symbol);
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    BoundExpression* localVariable = new BoundLocalVariable(localVariableSymbol);
    arguments.push_back(std::unique_ptr<BoundExpression>(new BoundAddressOfExpression(
        std::unique_ptr<BoundExpression>(localVariable), localVariable->GetType()->AddPointer(constructionStatementNode.GetSpan()))));
    bool constructDelegateType = localVariableSymbol->GetType()->GetSymbolType() == SymbolType::delegateTypeSymbol;
    bool constructClassDelegateType = localVariableSymbol->GetType()->GetSymbolType() == SymbolType::classDelegateTypeSymbol;
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, localVariableSymbol->GetType()->ClassInterfaceOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    int n = constructionStatementNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argumentNode = constructionStatementNode.Arguments()[i];
        std::unique_ptr<BoundExpression> argument = BindExpression(argumentNode, boundCompileUnit, currentFunction, containerScope, this, false, constructDelegateType || constructClassDelegateType, 
            constructClassDelegateType);
        arguments.push_back(std::move(argument));
    }
    std::unique_ptr<BoundFunctionCall> constructorCall = ResolveOverload(U"@constructor", containerScope, functionScopeLookups, arguments, boundCompileUnit, currentFunction, 
        constructionStatementNode.GetSpan());
    CheckAccess(currentFunction->GetFunctionSymbol(), constructorCall->GetFunctionSymbol());
    AddStatement(new BoundConstructionStatement(std::move(constructorCall)));
    if (constructionStatementNode.Label())
    {
        statement->SetLabel(constructionStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(DeleteStatementNode& deleteStatementNode)
{

}

void StatementBinder::Visit(DestroyStatementNode& destroyStatementNode)
{

}

void StatementBinder::Visit(AssignmentStatementNode& assignmentStatementNode)
{
    std::unique_ptr<BoundExpression> target = BindExpression(assignmentStatementNode.TargetExpr(), boundCompileUnit, currentFunction, containerScope, this, true);
    target.reset(new BoundAddressOfExpression(std::move(target), target->GetType()->AddPointer(assignmentStatementNode.GetSpan())));
    TypeSymbol* targetType = target->GetType();
    bool assignDelegateType = targetType->GetSymbolType() == SymbolType::delegateTypeSymbol;
    bool assignClassDelegateType = targetType->GetSymbolType() == SymbolType::classDelegateTypeSymbol;
    std::unique_ptr<BoundExpression> source = BindExpression(assignmentStatementNode.SourceExpr(), boundCompileUnit, currentFunction, containerScope, this, false, 
        assignDelegateType || assignClassDelegateType, assignClassDelegateType);
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::move(target));
    arguments.push_back(std::move(source));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, targetType->ClassInterfaceOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    std::unique_ptr<BoundFunctionCall> assignmentCall = ResolveOverload(U"operator=", containerScope, functionScopeLookups, arguments, boundCompileUnit, currentFunction, 
        assignmentStatementNode.GetSpan());
    CheckAccess(currentFunction->GetFunctionSymbol(), assignmentCall->GetFunctionSymbol());
    AddStatement(new BoundAssignmentStatement(std::move(assignmentCall)));
    if (assignmentStatementNode.Label())
    {
        statement->SetLabel(assignmentStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(ExpressionStatementNode& expressionStatementNode)
{
    std::unique_ptr<BoundExpression> expression = BindExpression(expressionStatementNode.Expression(), boundCompileUnit, currentFunction, containerScope, this);
    AddStatement(new BoundExpressionStatement(std::move(expression)));
    if (expressionStatementNode.Label())
    {
        statement->SetLabel(expressionStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(EmptyStatementNode& emptyStatementNode)
{
    AddStatement(new BoundEmptyStatement(emptyStatementNode.GetSpan()));
    if (emptyStatementNode.Label())
    {
        statement->SetLabel(emptyStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(RangeForStatementNode& rangeForStatementNode)
{

}

void StatementBinder::Visit(SwitchStatementNode& switchStatementNode)
{

}

void StatementBinder::Visit(CaseStatementNode& caseStatementNode)
{

}

void StatementBinder::Visit(DefaultStatementNode& defaultStatementNode)
{

}

void StatementBinder::Visit(GotoCaseStatementNode& gotoCaseStatementNode)
{

}

void StatementBinder::Visit(GotoDefaultStatementNode& gotoDefaultStatementNode)
{

}

void StatementBinder::Visit(ThrowStatementNode& throwStatementNode)
{

}

void StatementBinder::Visit(CatchNode& catchNode)
{

}

void StatementBinder::Visit(TryStatementNode& tryStatementNode)
{

}

void StatementBinder::Visit(AssertStatementNode& assertStatementNode)
{

}

void StatementBinder::CompileStatement(Node* statementNode, bool setPostfix)
{
    bool prevPostfix = postfix;
    postfix = setPostfix;
    statementNode->Accept(*this);
    postfix = prevPostfix;
}

void StatementBinder::AddStatement(BoundStatement* boundStatement)
{
    if (postfix)
    {
        boundStatement->SetPostfix();
    }
    if (statement)
    {
        if (statement->Postfix())
        {
            BoundSequenceStatement* sequenceStatement = new BoundSequenceStatement(boundStatement->GetSpan(), std::unique_ptr<BoundStatement>(boundStatement), std::move(statement));
            boundStatement = sequenceStatement;
        }
        else
        {
            BoundSequenceStatement* sequenceStatement = new BoundSequenceStatement(boundStatement->GetSpan(), std::move(statement), std::unique_ptr<BoundStatement>(boundStatement));
            boundStatement = sequenceStatement;
        }
        if (postfix)
        {
            boundStatement->SetPostfix();
        }
    }
    statement.reset(boundStatement);
}

} } // namespace cmajor::binder
