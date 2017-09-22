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
#include <cmajor/binder/Evaluator.hpp>
#include <cmajor/binder/TypeBinder.hpp>
#include <cmajor/binder/TypeResolver.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/symbols/SymbolCreatorVisitor.hpp>
#include <cmajor/parser/TypeExpr.hpp>
#include <cmajor/parser/FileRegistry.hpp>
#include <cmajor/ast/Literal.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/ast/Expression.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::unicode;
using namespace cmajor::parser;

bool IsAlwaysTrue(Node* node, BoundCompileUnit& boundCompileUnit, ContainerScope* containerScope)
{
    std::unique_ptr<Value> value = Evaluate(node, ValueType::boolValue, containerScope, boundCompileUnit, true);
    if (value)
    {
        if (value->GetValueType() == ValueType::boolValue)
        {
            BoolValue* boolValue = static_cast<BoolValue*>(value.get());
            return boolValue->GetValue() == true;
        }
    }
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

bool TerminatesCase(StatementNode* statementNode)
{
    if (statementNode->GetNodeType() == NodeType::ifStatementNode)
    {
        IfStatementNode* ifStatementNode = static_cast<IfStatementNode*>(statementNode);
        if (ifStatementNode->ElseS())
        {
            if (TerminatesCase(ifStatementNode->ThenS()) && TerminatesCase(ifStatementNode->ElseS()))
            {
                return true;
            }
        }
    }
    else if (statementNode->GetNodeType() == NodeType::compoundStatementNode)
    {
        CompoundStatementNode* compoundStatement = static_cast<CompoundStatementNode*>(statementNode);
        int n = compoundStatement->Statements().Count();
        for (int i = 0; i < n; ++i)
        {
            StatementNode* statementNode = compoundStatement->Statements()[i];
            if (TerminatesCase(statementNode))
            {
                return true;
            }
        }
    }
    else
    {
        return statementNode->IsCaseTerminatingNode();
    }
    return false;
}

bool TerminatesDefault(StatementNode* statementNode)
{
    if (statementNode->GetNodeType() == NodeType::ifStatementNode)
    {
        IfStatementNode* ifStatementNode = static_cast<IfStatementNode*>(statementNode);
        if (ifStatementNode->ElseS())
        {
            if (TerminatesDefault(ifStatementNode->ThenS()) && TerminatesDefault(ifStatementNode->ElseS()))
            {
                return true;
            }
        }
    }
    else if (statementNode->GetNodeType() == NodeType::compoundStatementNode)
    {
        CompoundStatementNode* compoundStatement = static_cast<CompoundStatementNode*>(statementNode);
        int n = compoundStatement->Statements().Count();
        for (int i = 0; i < n; ++i)
        {
            StatementNode* statementNode = compoundStatement->Statements()[i];
            if (TerminatesDefault(statementNode))
            {
                return true;
            }
        }
    }
    else
    {
        return statementNode->IsDefaultTerminatingNode();
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
    if (body)
    {
        int n = body->Statements().Count();
        for (int i = 0; i < n; ++i)
        {
            StatementNode* statement = body->Statements()[i];
            if (TerminatesFunction(statement, false, containerScope, boundCompileUnit)) return;
        }
        throw Exception("not all control paths terminate in return or throw statement", span);
    }
}

StatementBinder::StatementBinder(BoundCompileUnit& boundCompileUnit_) :  
    boundCompileUnit(boundCompileUnit_), symbolTable(boundCompileUnit.GetSymbolTable()), containerScope(nullptr), statement(), compoundLevel(0), insideCatch(false), 
    currentClass(nullptr), currentFunction(nullptr), currentStaticConstructorSymbol(nullptr), currentStaticConstructorNode(nullptr), currentConstructorSymbol(nullptr), 
    currentConstructorNode(nullptr), currentDestructorSymbol(nullptr), currentDestructorNode(nullptr), currentMemberFunctionSymbol(nullptr), currentMemberFunctionNode(nullptr), 
    switchConditionType(nullptr), currentCaseValueMap(nullptr), currentGotoCaseStatements(nullptr), currentGotoDefaultStatements(nullptr), postfix(false)
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
    if (classTypeSymbol->IsClassTemplate())
    {
        return;
    }
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
    if (functionSymbol->IsFunctionTemplate())
    {
        return;
    }
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(functionSymbol));
    BoundFunction* prevFunction = currentFunction;
    currentFunction = boundFunction.get();
    if (functionNode.Body())
    {
        compoundLevel = 0;
        functionNode.Body()->Accept(*this);
        BoundStatement* boundStatement = statement.release();
        Assert(boundStatement->GetBoundNodeType() == BoundNodeType::boundCompoundStatement, "bound compound statement expected");
        BoundCompoundStatement* compoundStatement = static_cast<BoundCompoundStatement*>(boundStatement);
        boundFunction->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
        CheckFunctionReturnPaths(functionSymbol, functionNode, containerScope, boundCompileUnit);
        boundCompileUnit.AddBoundNode(std::move(boundFunction));
    }
    currentFunction = prevFunction;
    containerScope = prevContainerScope;
}

void StatementBinder::Visit(StaticConstructorNode& staticConstructorNode)
{
    ContainerScope* prevContainerScope = containerScope;
    Symbol* symbol = boundCompileUnit.GetSymbolTable().GetSymbol(&staticConstructorNode);
    Assert(symbol->GetSymbolType() == SymbolType::staticConstructorSymbol , "static constructor symbol expected");
    StaticConstructorSymbol* staticConstructorSymbol = static_cast<StaticConstructorSymbol*>(symbol);
    StaticConstructorSymbol* prevStaticConstructorSymbol = currentStaticConstructorSymbol;
    currentStaticConstructorSymbol = staticConstructorSymbol;
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(staticConstructorSymbol));
    BoundFunction* prevFunction = currentFunction;
    currentFunction = boundFunction.get();
    if (staticConstructorNode.Body())
    {
        StaticConstructorNode* prevStaticConstructorNode = currentStaticConstructorNode;
        currentStaticConstructorNode = &staticConstructorNode;
        compoundLevel = 0;
        staticConstructorNode.Body()->Accept(*this);
        currentStaticConstructorNode = prevStaticConstructorNode;
        BoundStatement* boundStatement = statement.release();
        Assert(boundStatement->GetBoundNodeType() == BoundNodeType::boundCompoundStatement, "bound compound statement expected");
        BoundCompoundStatement* compoundStatement = static_cast<BoundCompoundStatement*>(boundStatement);
        boundFunction->SetBody(std::unique_ptr<BoundCompoundStatement>(compoundStatement));
        CheckFunctionReturnPaths(staticConstructorSymbol, staticConstructorNode, containerScope, boundCompileUnit);
        currentClass->AddMember(std::move(boundFunction));
    }
    currentFunction = prevFunction;
    containerScope = prevContainerScope;
    currentStaticConstructorSymbol = prevStaticConstructorSymbol;
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
        compoundLevel = 0;
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
    if (boundFunction->Body())
    {
        CheckFunctionReturnPaths(constructorSymbol, constructorNode, containerScope, boundCompileUnit);
        currentClass->AddMember(std::move(boundFunction));
    }
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
        compoundLevel = 0;
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
    if (boundFunction->Body())
    {
        CheckFunctionReturnPaths(destructorSymbol, destructorNode, containerScope, boundCompileUnit);
        currentClass->AddMember(std::move(boundFunction));
    }
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
    MemberFunctionSymbol* prevMemberFunctionSymbol = currentMemberFunctionSymbol;
    currentMemberFunctionSymbol = memberFunctionSymbol;
    containerScope = symbol->GetContainerScope();
    std::unique_ptr<BoundFunction> boundFunction(new BoundFunction(memberFunctionSymbol));
    BoundFunction* prevFunction = currentFunction;
    currentFunction = boundFunction.get();
    if (memberFunctionNode.Body())
    {
        MemberFunctionNode* prevMemberFunctionNode = currentMemberFunctionNode;
        currentMemberFunctionNode = &memberFunctionNode;
        compoundLevel = 0;
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
    if (boundFunction->Body())
    {
        CheckFunctionReturnPaths(memberFunctionSymbol, memberFunctionNode, containerScope, boundCompileUnit);
        currentClass->AddMember(std::move(boundFunction));
    }
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
    if (compoundLevel == 0)
    {
        if (currentStaticConstructorSymbol && currentStaticConstructorNode)
        {
            GenerateStaticClassInitialization(currentStaticConstructorSymbol, currentStaticConstructorNode, boundCompileUnit, boundCompoundStatement.get(), currentFunction, containerScope, this);
        }
        else if (currentConstructorSymbol && currentConstructorNode)
        {
            GenerateClassInitialization(currentConstructorSymbol, currentConstructorNode, boundCompoundStatement.get(), currentFunction, boundCompileUnit, containerScope, this, false);
        }
        else if (currentMemberFunctionSymbol && currentMemberFunctionSymbol->GroupName() == U"operator=" && currentMemberFunctionNode)
        {
            GenerateClassAssignment(currentMemberFunctionSymbol, currentMemberFunctionNode, boundCompoundStatement.get(), currentFunction, boundCompileUnit, containerScope, this, false);
        }
        else if (currentMemberFunctionSymbol && currentMemberFunctionSymbol->IsStatic() && currentMemberFunctionNode)
        {
            if (currentClass->GetClassTypeSymbol()->StaticConstructor())
            {
                boundCompoundStatement->AddStatement(std::unique_ptr<BoundStatement>(new BoundExpressionStatement(std::unique_ptr<BoundExpression>(
                    new BoundFunctionCall(currentMemberFunctionNode->GetSpan(), currentClass->GetClassTypeSymbol()->StaticConstructor())))));
            }
        }
    }
    ++compoundLevel;
    int n = compoundStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = compoundStatementNode.Statements()[i];
        statementNode->Accept(*this);
        boundCompoundStatement->AddStatement(std::move(statement));
    }
    --compoundLevel;
    if (compoundLevel == 0 && currentDestructorSymbol && currentDestructorNode)
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
        if (currentFunction->GetFunctionSymbol()->ReturnsClassByValue())
        {
            std::vector<FunctionScopeLookup> classReturnLookups;
            classReturnLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
            classReturnLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, currentFunction->GetFunctionSymbol()->ReturnType()->ClassInterfaceOrNsScope()));
            classReturnLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
            std::vector<std::unique_ptr<BoundExpression>> classReturnArgs;
            classReturnArgs.push_back(std::unique_ptr<BoundExpression>(new BoundParameter(currentFunction->GetFunctionSymbol()->ReturnParam())));
            std::unique_ptr<BoundExpression> expression = BindExpression(returnStatementNode.Expression(), boundCompileUnit, currentFunction, containerScope, this, false, false, false);
            std::vector<FunctionScopeLookup> rvalueLookups;
            rvalueLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
            rvalueLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
            std::vector<std::unique_ptr<BoundExpression>> rvalueArguments;
            rvalueArguments.push_back(std::move(expression));
            std::unique_ptr<BoundExpression> rvalueExpr = ResolveOverload(U"System.Rvalue", containerScope, rvalueLookups, rvalueArguments, boundCompileUnit, currentFunction,
                returnStatementNode.GetSpan());
            classReturnArgs.push_back(std::move(rvalueExpr));
            std::unique_ptr<BoundFunctionCall> constructorCall = ResolveOverload(U"@constructor", containerScope, classReturnLookups, classReturnArgs, boundCompileUnit, currentFunction,
                returnStatementNode.GetSpan());
            std::unique_ptr<BoundStatement> constructStatement(new BoundExpressionStatement(std::move(constructorCall)));
            AddStatement(constructStatement.release());
            std::unique_ptr<BoundFunctionCall> returnFunctionCall;
            std::unique_ptr<BoundStatement> returnStatement(new BoundReturnStatement(std::move(returnFunctionCall), returnStatementNode.GetSpan()));
            AddStatement(returnStatement.release());
        }
        else
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
                functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, returnType->BaseType()->ClassInterfaceEnumOrNsScope()));
                functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
                std::unique_ptr<BoundFunctionCall> returnFunctionCall = ResolveOverload(U"@return", containerScope, functionScopeLookups, returnTypeArgs, boundCompileUnit, currentFunction,
                    returnStatementNode.GetSpan());
                std::unique_ptr<BoundExpression> expression = BindExpression(returnStatementNode.Expression(), boundCompileUnit, currentFunction, containerScope, this, false,
                    returnDelegateType || returnClassDelegateType, returnClassDelegateType);
                std::vector<std::unique_ptr<BoundExpression>> returnValueArguments;
                returnValueArguments.push_back(std::move(expression));
                FunctionMatch functionMatch(returnFunctionCall->GetFunctionSymbol());
                bool conversionFound = FindConversions(boundCompileUnit, returnFunctionCall->GetFunctionSymbol(), returnValueArguments, functionMatch, ConversionType::implicit_, 
                    containerScope, returnStatementNode.GetSpan());
                if (conversionFound)
                {
                    Assert(!functionMatch.argumentMatches.empty(), "argument match expected");
                    ArgumentMatch argumentMatch = functionMatch.argumentMatches[0];
                    FunctionSymbol* conversionFun = argumentMatch.conversionFun;
                    if (conversionFun)
                    {
                        if (conversionFun->GetSymbolType() == SymbolType::constructorSymbol)
                        {
                            BoundFunctionCall* constructorCall = new BoundFunctionCall(returnStatementNode.GetSpan(), conversionFun);
                            LocalVariableSymbol* temporary = currentFunction->GetFunctionSymbol()->CreateTemporary(conversionFun->ConversionTargetType(), returnStatementNode.GetSpan());
                            constructorCall->AddArgument(std::unique_ptr<BoundExpression>(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)),
                                conversionFun->ConversionTargetType()->AddPointer(returnStatementNode.GetSpan()))));
                            constructorCall->AddArgument(std::move(returnValueArguments[0]));
                            BoundConstructAndReturnTemporaryExpression* conversion = new BoundConstructAndReturnTemporaryExpression(std::unique_ptr<BoundExpression>(constructorCall),
                                std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)));
                            returnValueArguments[0].reset(conversion);
                        }
                        else
                        {
                            BoundConversion* boundConversion = new BoundConversion(std::unique_ptr<BoundExpression>(returnValueArguments[0].release()), conversionFun);
                            returnValueArguments[0].reset(boundConversion);
                        }
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
                AddStatement(new BoundReturnStatement(std::move(returnFunctionCall), returnStatementNode.GetSpan()));
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
    }
    else
    {
        TypeSymbol* returnType = currentFunction->GetFunctionSymbol()->ReturnType();
        if (!returnType || returnType->GetSymbolType() == SymbolType::voidTypeSymbol)
        {
            std::unique_ptr<BoundFunctionCall> returnFunctionCall;
            AddStatement(new BoundReturnStatement(std::move(returnFunctionCall), returnStatementNode.GetSpan()));
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
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, localVariableSymbol->GetType()->ClassInterfaceEnumOrNsScope()));
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
    std::unique_ptr<BoundExpression> ptr = BindExpression(deleteStatementNode.Expression(), boundCompileUnit, currentFunction, containerScope, this);
    std::unique_ptr<BoundExpression> memFreePtr;
    TypeSymbol* baseType = ptr->GetType()->BaseType();
    if (baseType->HasNontrivialDestructor())
    {
        Assert(baseType->GetSymbolType() == SymbolType::classTypeSymbol || baseType->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol, "class type expected");
        ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(baseType);
        std::vector<FunctionScopeLookup> lookups;
        lookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
        lookups.push_back(FunctionScopeLookup(ScopeLookup::this_, classType->ClassInterfaceOrNsScope()));
        lookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
        std::vector<std::unique_ptr<BoundExpression>> arguments;
        arguments.push_back(std::move(ptr));
        std::unique_ptr<BoundFunctionCall> destructorCall = ResolveOverload(U"@destructor", containerScope, lookups, arguments, boundCompileUnit, currentFunction, deleteStatementNode.GetSpan());
        CheckAccess(currentFunction->GetFunctionSymbol(), destructorCall->GetFunctionSymbol());
        if (destructorCall->GetFunctionSymbol()->IsVirtualAbstractOrOverride())
        {
            destructorCall->SetFlag(BoundExpressionFlags::virtualCall);
        }
        AddStatement(new BoundExpressionStatement(std::move(destructorCall)));
        memFreePtr = BindExpression(deleteStatementNode.Expression(), boundCompileUnit, currentFunction, containerScope, this);
    }
    else
    {
        memFreePtr = std::move(ptr);
    }
    std::vector<FunctionScopeLookup> lookups;
    lookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    lookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::move(memFreePtr));
    std::unique_ptr<BoundFunctionCall> memFreeCall = ResolveOverload(U"RtMemFree", containerScope, lookups, arguments, boundCompileUnit, currentFunction, deleteStatementNode.GetSpan());
    CheckAccess(currentFunction->GetFunctionSymbol(), memFreeCall->GetFunctionSymbol());
    AddStatement(new BoundExpressionStatement(std::move(memFreeCall)));
    if (deleteStatementNode.Label())
    {
        statement->SetLabel(deleteStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(DestroyStatementNode& destroyStatementNode)
{
    std::unique_ptr<BoundExpression> ptr = BindExpression(destroyStatementNode.Expression(), boundCompileUnit, currentFunction, containerScope, this);
    TypeSymbol* baseType = ptr->GetType()->BaseType();
    if (baseType->HasNontrivialDestructor())
    {
        Assert(baseType->GetSymbolType() == SymbolType::classTypeSymbol || baseType->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol, "class type expected");
        ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(baseType);
        std::vector<FunctionScopeLookup> lookups;
        lookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
        lookups.push_back(FunctionScopeLookup(ScopeLookup::this_, classType->ClassInterfaceOrNsScope()));
        lookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
        std::vector<std::unique_ptr<BoundExpression>> arguments;
        arguments.push_back(std::move(ptr));
        std::unique_ptr<BoundFunctionCall> destructorCall = ResolveOverload(U"@destructor", containerScope, lookups, arguments, boundCompileUnit, currentFunction, destroyStatementNode.GetSpan());
        CheckAccess(currentFunction->GetFunctionSymbol(), destructorCall->GetFunctionSymbol());
        if (destructorCall->GetFunctionSymbol()->IsVirtualAbstractOrOverride())
        {
            destructorCall->SetFlag(BoundExpressionFlags::virtualCall);
        }
        AddStatement(new BoundExpressionStatement(std::move(destructorCall)));
    }
    else
    {
        AddStatement(new BoundEmptyStatement(destroyStatementNode.GetSpan()));
    }
}

void StatementBinder::Visit(AssignmentStatementNode& assignmentStatementNode)
{
    std::unique_ptr<BoundExpression> target = BindExpression(assignmentStatementNode.TargetExpr(), boundCompileUnit, currentFunction, containerScope, this, true);
    TypeSymbol* targetPlainType = target->GetType()->PlainType(assignmentStatementNode.GetSpan());
    if (targetPlainType->IsClassTypeSymbol() && target->GetType()->IsReferenceType())
    {
        TypeSymbol* type = target->GetType()->RemoveReference(assignmentStatementNode.GetSpan())->AddPointer(assignmentStatementNode.GetSpan());
        target.reset(new BoundReferenceToPointerExpression(std::unique_ptr<BoundExpression>(target.release()), type));
    }
    else
    {
        target.reset(new BoundAddressOfExpression(std::move(target), target->GetType()->AddPointer(assignmentStatementNode.GetSpan())));
    }
    TypeSymbol* targetType = target->GetType()->BaseType();
    bool assignDelegateType = targetType->GetSymbolType() == SymbolType::delegateTypeSymbol;
    bool assignClassDelegateType = targetType->GetSymbolType() == SymbolType::classDelegateTypeSymbol;
    std::unique_ptr<BoundExpression> source = BindExpression(assignmentStatementNode.SourceExpr(), boundCompileUnit, currentFunction, containerScope, this, false, 
        assignDelegateType || assignClassDelegateType, assignClassDelegateType);
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::move(target));
    arguments.push_back(std::move(source));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, targetType->ClassInterfaceEnumOrNsScope()));
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

TypeExprGrammar* typeExprGrammar = nullptr;

void StatementBinder::Visit(RangeForStatementNode& rangeForStatementNode)
{
    const Span& span = rangeForStatementNode.GetSpan();
    std::unique_ptr<BoundExpression> container = BindExpression(rangeForStatementNode.Container(), boundCompileUnit, currentFunction, containerScope, this);
    TypeSymbol* plainContainerType = container->GetType()->PlainType(span);
    std::u32string plainContainerTypeFullName = plainContainerType->FullName();
    if (!typeExprGrammar)
    {
        typeExprGrammar = TypeExprGrammar::Create();
    }
    ParsingContext parsingContext;
    std::unique_ptr<Node> containerTypeNode(typeExprGrammar->Parse(&plainContainerTypeFullName[0], &plainContainerTypeFullName[plainContainerTypeFullName.length()], 0, "", &parsingContext));
    std::unique_ptr<IdentifierNode> iteratorTypeNode = nullptr;
    if (container->GetType()->IsConstType())
    {
        iteratorTypeNode.reset(new IdentifierNode(span, U"ConstIterator"));
    }
    else
    {
        iteratorTypeNode.reset(new IdentifierNode(span, U"Iterator"));
    }
    CloneContext cloneContext;
    std::unique_ptr<CompoundStatementNode> compoundStatementNode(new CompoundStatementNode(span));
    compoundStatementNode->SetParent(rangeForStatementNode.Parent());
    ConstructionStatementNode* constructEndIteratorStatement = new ConstructionStatementNode(span, 
        new DotNode(span, containerTypeNode->Clone(cloneContext), static_cast<IdentifierNode*>(iteratorTypeNode->Clone(cloneContext))), new IdentifierNode(span, U"@end"));
    if (container->GetType()->IsConstType())
    {
        constructEndIteratorStatement->AddArgument(new InvokeNode(span, new DotNode(span, rangeForStatementNode.Container()->Clone(cloneContext), new IdentifierNode(span, U"CEnd"))));
    }
    else
    {
        constructEndIteratorStatement->AddArgument(new InvokeNode(span, new DotNode(span, rangeForStatementNode.Container()->Clone(cloneContext), new IdentifierNode(span, U"End"))));
    }
    compoundStatementNode->AddStatement(constructEndIteratorStatement);
    ConstructionStatementNode* constructIteratorStatement = new ConstructionStatementNode(span, 
        new DotNode(span, containerTypeNode->Clone(cloneContext), static_cast<IdentifierNode*>(iteratorTypeNode->Clone(cloneContext))), new IdentifierNode(span, U"@it"));
    if (container->GetType()->IsConstType())
    {
        constructIteratorStatement->AddArgument(new InvokeNode(span, new DotNode(span, rangeForStatementNode.Container()->Clone(cloneContext), new IdentifierNode(span, U"CBegin"))));
    }
    else
    {
        constructIteratorStatement->AddArgument(new InvokeNode(span, new DotNode(span, rangeForStatementNode.Container()->Clone(cloneContext), new IdentifierNode(span, U"Begin"))));
    }
    Node* itNotEndCond = new NotEqualNode(span, new IdentifierNode(span, U"@it"), new IdentifierNode(span, U"@end"));
    StatementNode* incrementItStatement = new ExpressionStatementNode(span, new PrefixIncrementNode(span, new IdentifierNode(span, U"@it")));
    CompoundStatementNode* actionStatement = new CompoundStatementNode(span);
    ConstructionStatementNode* constructLoopVarStatement = new ConstructionStatementNode(span,
        rangeForStatementNode.TypeExpr()->Clone(cloneContext), static_cast<IdentifierNode*>(rangeForStatementNode.Id()->Clone(cloneContext)));
    constructLoopVarStatement->AddArgument(new DerefNode(span, new IdentifierNode(span, U"@it")));
    actionStatement->AddStatement(constructLoopVarStatement);
    actionStatement->AddStatement(static_cast<StatementNode*>(rangeForStatementNode.Action()->Clone(cloneContext)));
    ForStatementNode* forStatement = new ForStatementNode(span, constructIteratorStatement, itNotEndCond, incrementItStatement, actionStatement);
    compoundStatementNode->AddStatement(forStatement);

    symbolTable.BeginContainer(containerScope->Container());
    SymbolCreatorVisitor symbolCreatorVisitor(symbolTable);
    compoundStatementNode->Accept(symbolCreatorVisitor);
    symbolTable.EndContainer();
    TypeBinder typeBinder(boundCompileUnit);
    typeBinder.SetContainerScope(containerScope);
    typeBinder.SetCurrentFunctionSymbol(currentFunction->GetFunctionSymbol());
    compoundStatementNode->Accept(typeBinder);
    compoundStatementNode->Accept(*this);
}

void StatementBinder::Visit(SwitchStatementNode& switchStatementNode)
{
    std::unique_ptr<BoundExpression> condition = BindExpression(switchStatementNode.Condition(), boundCompileUnit, currentFunction, containerScope, this);
    TypeSymbol* conditionType = condition->GetType();
    if (conditionType->IsSwitchConditionType())
    {
        if (conditionType->GetSymbolType() == SymbolType::enumTypeSymbol)
        {
            EnumTypeSymbol* enumType = static_cast<EnumTypeSymbol*>(conditionType);
            conditionType = enumType->UnderlyingType();
        }
        TypeSymbol* prevSwitchConditionType = switchConditionType;
        switchConditionType = conditionType;
        std::unordered_map<IntegralValue, CaseStatementNode*, IntegralValueHash>* prevCaseValueMap = currentCaseValueMap;
        std::unordered_map<IntegralValue, CaseStatementNode*, IntegralValueHash> caseValueMap;
        currentCaseValueMap = &caseValueMap;
        std::vector<std::pair<BoundGotoCaseStatement*, IntegralValue>>* prevGotoCaseStatements = currentGotoCaseStatements;
        std::vector<std::pair<BoundGotoCaseStatement*, IntegralValue>> gotoCaseStatements;
        currentGotoCaseStatements = &gotoCaseStatements;
        std::vector<BoundGotoDefaultStatement*>* prevGotoDefaultStatements = currentGotoDefaultStatements;
        std::vector<BoundGotoDefaultStatement*> gotoDefaultStatements;
        currentGotoDefaultStatements = &gotoDefaultStatements;
        std::unique_ptr<BoundSwitchStatement> boundSwitchStatement(new BoundSwitchStatement(switchStatementNode.GetSpan(), std::move(condition)));
        int n = switchStatementNode.Cases().Count();
        for (int i = 0; i < n; ++i)
        {
            CaseStatementNode* caseS = switchStatementNode.Cases()[i];
            caseS->Accept(*this);
            Assert(statement->GetBoundNodeType() == BoundNodeType::boundCaseStatement, "case statement expected");
            boundSwitchStatement->AddCaseStatement(std::unique_ptr<BoundCaseStatement>(static_cast<BoundCaseStatement*>(statement.release())));
        }
        if (switchStatementNode.Default())
        {
            switchStatementNode.Default()->Accept(*this);
            Assert(statement->GetBoundNodeType() == BoundNodeType::boundDefaultStatement, "default statement expected");
            boundSwitchStatement->SetDefaultStatement(std::unique_ptr<BoundDefaultStatement>(static_cast<BoundDefaultStatement*>(statement.release())));
        }
        for (const std::pair<BoundGotoCaseStatement*, IntegralValue>& p : gotoCaseStatements)
        {
            BoundGotoCaseStatement* gotoCaseStatement = p.first;
            IntegralValue integralCaseValue = p.second;
            auto it = caseValueMap.find(integralCaseValue);
            if (it == caseValueMap.cend())
            {
                throw Exception("case not found", gotoCaseStatement->GetSpan());
            }
        }
        if (!gotoDefaultStatements.empty() && !switchStatementNode.Default())
        {
            throw Exception("switch does not have a default statement", gotoDefaultStatements.front()->GetSpan());
        }
        currentGotoCaseStatements = prevGotoCaseStatements;
        currentGotoDefaultStatements = prevGotoDefaultStatements;
        currentCaseValueMap = prevCaseValueMap;
        AddStatement(boundSwitchStatement.release());
        if (switchStatementNode.Label())
        {
            statement->SetLabel(switchStatementNode.Label()->Label());
        }
        switchConditionType = prevSwitchConditionType;
    }
    else
    {
        throw Exception("switch statement condition must be of integer, character, enumerated or Boolean type", switchStatementNode.Condition()->GetSpan());
    }
}

void StatementBinder::Visit(CaseStatementNode& caseStatementNode)
{
    std::unique_ptr<BoundCaseStatement> boundCaseStatement(new BoundCaseStatement(caseStatementNode.GetSpan()));
    bool terminated = false;
    int n = caseStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = caseStatementNode.Statements()[i];
        if (TerminatesCase(statementNode))
        {
            terminated = true;
        }
        statementNode->Accept(*this);
        boundCaseStatement->AddStatement(std::move(statement));
    }
    if (!terminated)
    {
        throw Exception("case must end in break, continue, return, throw, goto, goto case or goto default statement", caseStatementNode.GetSpan());
    }
    int ne = caseStatementNode.CaseExprs().Count();
    for (int i = 0; i < ne; ++i)
    {
        Node* caseExprNode = caseStatementNode.CaseExprs()[i];
        std::unique_ptr<Value> caseValue = Evaluate(caseExprNode, GetValueTypeFor(switchConditionType->GetSymbolType()), containerScope, boundCompileUnit, false);
        IntegralValue integralCaseValue(caseValue.get());
        Assert(currentCaseValueMap, "current case value map not set");
        auto it = currentCaseValueMap->find(integralCaseValue);
        if (it != currentCaseValueMap->cend())
        {
            throw Exception("case value already used", caseExprNode->GetSpan());
        }
        (*currentCaseValueMap)[integralCaseValue] = &caseStatementNode;
        boundCaseStatement->AddCaseValue(std::move(caseValue));
    }
    AddStatement(boundCaseStatement.release());
    if (caseStatementNode.Label())
    {
        statement->SetLabel(caseStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(DefaultStatementNode& defaultStatementNode)
{
    std::unique_ptr<BoundDefaultStatement> boundDefaultStatement(new BoundDefaultStatement(defaultStatementNode.GetSpan()));
    bool terminated = false;
    int n = defaultStatementNode.Statements().Count();
    for (int i = 0; i < n; ++i)
    {
        StatementNode* statementNode = defaultStatementNode.Statements()[i];
        if (TerminatesDefault(statementNode))
        {
            terminated = true;
        }
        statementNode->Accept(*this);
        boundDefaultStatement->AddStatement(std::move(statement));
    }
    if (!terminated)
    {
        throw Exception("default must end in break, continue, return, throw, goto, or goto case statement", defaultStatementNode.GetSpan());
    }
    AddStatement(boundDefaultStatement.release());
    if (defaultStatementNode.Label())
    {
        statement->SetLabel(defaultStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(GotoCaseStatementNode& gotoCaseStatementNode)
{
    const Node* parent = gotoCaseStatementNode.Parent();
    while (parent && parent->GetNodeType() != NodeType::caseStatementNode && parent->GetNodeType() != NodeType::defaultStatementNode)
    {
        parent = parent->Parent();
    }
    if (!parent)
    {
        throw Exception("goto case statement must be enclosed in a case or default statement", gotoCaseStatementNode.GetSpan());
    }
    Node* caseExprNode = gotoCaseStatementNode.CaseExpr();
    std::unique_ptr<Value> caseValue = Evaluate(caseExprNode, GetValueTypeFor(switchConditionType->GetSymbolType()), containerScope, boundCompileUnit, false);
    Value* caseValuePtr = caseValue.get();
    BoundGotoCaseStatement* boundGotoCaseStatement = new BoundGotoCaseStatement(gotoCaseStatementNode.GetSpan(), std::move(caseValue));
    Assert(currentGotoCaseStatements, "current goto case statement vector not set");
    currentGotoCaseStatements->push_back(std::make_pair(boundGotoCaseStatement, IntegralValue(caseValuePtr)));
    AddStatement(boundGotoCaseStatement);
    if (gotoCaseStatementNode.Label())
    {
        statement->SetLabel(gotoCaseStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(GotoDefaultStatementNode& gotoDefaultStatementNode)
{
    const Node* parent = gotoDefaultStatementNode.Parent();
    while (parent && parent->GetNodeType() != NodeType::caseStatementNode)
    {
        parent = parent->Parent();
    }
    if (!parent)
    {
        throw Exception("goto default statement must be enclosed in a case statement", gotoDefaultStatementNode.GetSpan());
    }
    BoundGotoDefaultStatement* boundGotoDefaultStatement = new BoundGotoDefaultStatement(gotoDefaultStatementNode.GetSpan());
    Assert(currentGotoDefaultStatements, "current goto default statement vector not set");
    currentGotoDefaultStatements->push_back(boundGotoDefaultStatement);
    AddStatement(boundGotoDefaultStatement);
    if (gotoDefaultStatementNode.Label())
    {
        statement->SetLabel(gotoDefaultStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(ThrowStatementNode& throwStatementNode)
{
    if (currentFunction->GetFunctionSymbol()->DontThrow() && !currentFunction->GetFunctionSymbol()->HasTry())
    {
        throw Exception("a nothrow function cannot contain a throw statement unless it handles exceptions", throwStatementNode.GetSpan(), currentFunction->GetFunctionSymbol()->GetSpan());
    }
    Span span = throwStatementNode.GetSpan();
    Node* exceptionExprNode = throwStatementNode.Expression();
    if (exceptionExprNode)
    {
        std::unique_ptr<BoundExpression> boundExceptionExpr = BindExpression(exceptionExprNode, boundCompileUnit, currentFunction, containerScope, this);
        if (boundExceptionExpr->GetType()->PlainType(span)->IsClassTypeSymbol())
        {
            ClassTypeSymbol* exceptionClassType = static_cast<ClassTypeSymbol*>(boundExceptionExpr->GetType()->PlainType(span));
            IdentifierNode systemExceptionNode(throwStatementNode.GetSpan(), U"System.Exception");
            TypeSymbol* systemExceptionType = ResolveType(&systemExceptionNode, boundCompileUnit, containerScope);
            Assert(systemExceptionType->IsClassTypeSymbol(), "System.Exception not of class type");
            ClassTypeSymbol* systemExceptionClassType = static_cast<ClassTypeSymbol*>(systemExceptionType);
            if (exceptionClassType == systemExceptionClassType || exceptionClassType->HasBaseClass(systemExceptionClassType))
            {
                uint32_t exceptionTypeId = exceptionClassType->TypeId();
                NewNode* newNode = new NewNode(span, new IdentifierNode(span, exceptionClassType->FullName()));
                CloneContext cloneContext;
                newNode->AddArgument(throwStatementNode.Expression()->Clone(cloneContext));
                InvokeNode invokeNode(span, new IdentifierNode(span, U"RtThrowException"));
                invokeNode.AddArgument(newNode);
                invokeNode.AddArgument(new UIntLiteralNode(span, exceptionTypeId));
                std::unique_ptr<BoundExpression> throwCall = BindExpression(&invokeNode, boundCompileUnit, currentFunction, containerScope, this);
                AddStatement(new BoundThrowStatement(span, std::move(throwCall)));
            }
            else
            {
                throw Exception("exception class must be derived from System.Exception class", throwStatementNode.GetSpan());
            }
        }
        else
        {
            throw Exception("exception not of class type", throwStatementNode.GetSpan());
        }
    }
    else
    {
        if (insideCatch)
        {
            InvokeNode invokeNode(span, new DotNode(span, new IdentifierNode(span, U"@exPtr"), new IdentifierNode(span, U"Release")));
            std::unique_ptr<BoundExpression> releaseCall = BindExpression(&invokeNode, boundCompileUnit, currentFunction, containerScope, this);
            AddStatement(new BoundRethrowStatement(span, std::move(releaseCall)));
        }
        else
        {
            throw Exception("rethrow must occur inside a catch clause", throwStatementNode.GetSpan());
        }
    }
    if (throwStatementNode.Label())
    {
        statement->SetLabel(throwStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(TryStatementNode& tryStatementNode)
{
    BoundTryStatement* boundTryStatement = new BoundTryStatement(tryStatementNode.GetSpan());
    tryStatementNode.TryBlock()->Accept(*this);
    boundTryStatement->SetTryBlock(std::move(statement));
    int n = tryStatementNode.Catches().Count();
    for (int i = 0; i < n; ++i)
    {
        CatchNode* catchNode = tryStatementNode.Catches()[i];
        catchNode->Accept(*this);
        BoundStatement* s = statement.release();
        Assert(s->GetBoundNodeType() == BoundNodeType::boundCatchStatement, "catch statement expected");
        BoundCatchStatement* catchStatement = static_cast<BoundCatchStatement*>(s);
        boundTryStatement->AddCatch(std::unique_ptr<BoundCatchStatement>(catchStatement));
    }
    AddStatement(boundTryStatement);
    if (tryStatementNode.Label())
    {
        statement->SetLabel(tryStatementNode.Label()->Label());
    }
}

void StatementBinder::Visit(CatchNode& catchNode)
{
    bool prevInsideCatch = insideCatch;
    insideCatch = true;
    Span span = catchNode.GetSpan();
    std::unique_ptr<BoundCatchStatement> boundCatchStatement(new BoundCatchStatement(catchNode.GetSpan()));;
    TypeSymbol* catchedType = ResolveType(catchNode.TypeExpr(), boundCompileUnit, containerScope);
    boundCatchStatement->SetCatchedType(catchedType);
    LocalVariableSymbol* catchVar = nullptr;
    if (catchNode.Id())
    {
        Symbol* symbol = symbolTable.GetSymbol(catchNode.Id());
        Assert(symbol->GetSymbolType() == SymbolType::localVariableSymbol, "local variable symbol expected");
        catchVar = static_cast<LocalVariableSymbol*>(symbol);
        boundCatchStatement->SetCatchVar(catchVar);
        currentFunction->GetFunctionSymbol()->AddLocalVariable(catchVar);
    }
    CompoundStatementNode handlerBlock(span);
    handlerBlock.SetParent(catchNode.Parent());
    ConstructionStatementNode* getExceptionAddr = new ConstructionStatementNode(span, new PointerNode(span, new IdentifierNode(span, U"void")), new IdentifierNode(span, U"@exceptionAddr"));
    getExceptionAddr->AddArgument(new InvokeNode(span, new IdentifierNode(span, U"RtGetException")));
    handlerBlock.AddStatement(getExceptionAddr);
    PointerNode exceptionPtrTypeNode(span, new IdentifierNode(span, catchedType->BaseType()->FullName()));
    CloneContext cloneContext;
    ConstructionStatementNode* constructExceptionPtr = new ConstructionStatementNode(span, exceptionPtrTypeNode.Clone(cloneContext), new IdentifierNode(span, U"@exceptionPtr"));
    constructExceptionPtr->AddArgument(new CastNode(span, exceptionPtrTypeNode.Clone(cloneContext), new IdentifierNode(span, U"@exceptionAddr")));
    handlerBlock.AddStatement(constructExceptionPtr);
    TemplateIdNode* uniquePtrNode = new TemplateIdNode(span, new IdentifierNode(span, U"UniquePtr"));
    uniquePtrNode->AddTemplateArgument(new IdentifierNode(span, catchedType->BaseType()->FullName()));
    ConstructionStatementNode* constructUniquePtrException = new ConstructionStatementNode(span, uniquePtrNode, new IdentifierNode(span, U"@exPtr"));
    constructUniquePtrException->AddArgument(new IdentifierNode(span, U"@exceptionPtr"));
    handlerBlock.AddStatement(constructUniquePtrException);
    if (catchVar)
    {
        ConstructionStatementNode* setExceptionVar = new ConstructionStatementNode(span, catchNode.TypeExpr()->Clone(cloneContext), static_cast<IdentifierNode*>(catchNode.Id()->Clone(cloneContext)));
        setExceptionVar->AddArgument(new DerefNode(span, new IdentifierNode(span, U"@exPtr")));
        handlerBlock.AddStatement(setExceptionVar);
    }
    handlerBlock.AddStatement(static_cast<StatementNode*>(catchNode.CatchBlock()->Clone(cloneContext)));
    symbolTable.BeginContainer(containerScope->Container());
    SymbolCreatorVisitor symbolCreatorVisitor(symbolTable);
    handlerBlock.Accept(symbolCreatorVisitor);
    symbolTable.EndContainer();
    TypeBinder typeBinder(boundCompileUnit);
    typeBinder.SetContainerScope(containerScope);
    typeBinder.SetCurrentFunctionSymbol(currentFunction->GetFunctionSymbol());
    handlerBlock.Accept(typeBinder);
    handlerBlock.Accept(*this);
    boundCatchStatement->SetCatchBlock(std::move(statement));
    AddStatement(boundCatchStatement.release());
    insideCatch = prevInsideCatch;
}

void StatementBinder::Visit(AssertStatementNode& assertStatementNode)
{
    if (GetGlobalFlag(GlobalFlags::release))
    {
        AddStatement(new BoundEmptyStatement(assertStatementNode.GetSpan()));
    }
    else
    {
        std::vector<FunctionScopeLookup> lookups;
        lookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, symbolTable.GlobalNs().GetContainerScope()));
        std::vector<std::unique_ptr<BoundExpression>> arguments;
        TypeSymbol* constCharPtrType = symbolTable.GetTypeByName(U"char")->AddConst(assertStatementNode.GetSpan())->AddPointer(assertStatementNode.GetSpan());
        arguments.push_back(std::unique_ptr<BoundExpression>(new BoundLiteral(std::unique_ptr<Value>(new StringValue(assertStatementNode.GetSpan(),
            boundCompileUnit.Install(assertStatementNode.AssertExpr()->ToString()))), constCharPtrType)));
        arguments.push_back(std::unique_ptr<BoundExpression>(new BoundLiteral(std::unique_ptr<Value>(new StringValue(assertStatementNode.GetSpan(),
            boundCompileUnit.Install(ToUtf8(currentFunction->GetFunctionSymbol()->FullName())))), constCharPtrType)));
        arguments.push_back(std::unique_ptr<BoundExpression>(new BoundLiteral(std::unique_ptr<Value>(new StringValue(assertStatementNode.GetSpan(),
            boundCompileUnit.Install(FileRegistry::Instance().GetFilePath(assertStatementNode.GetSpan().FileIndex())))), constCharPtrType)));
        arguments.push_back(std::unique_ptr<BoundExpression>(new BoundLiteral(std::unique_ptr<Value>(new IntValue(assertStatementNode.GetSpan(), 
            assertStatementNode.GetSpan().LineNumber())), symbolTable.GetTypeByName(U"int"))));
        std::unique_ptr<BoundExpression> assertExpression = BindExpression(assertStatementNode.AssertExpr(), boundCompileUnit, currentFunction, containerScope, this);
        std::unique_ptr<BoundStatement> ifStatement(new BoundIfStatement(assertStatementNode.GetSpan(), std::move(assertExpression),
            std::unique_ptr<BoundStatement>(new BoundEmptyStatement(assertStatementNode.GetSpan())),
            std::unique_ptr<BoundStatement>(new BoundExpressionStatement(ResolveOverload(U"RtFailAssertion", containerScope, lookups, arguments, boundCompileUnit, currentFunction,
                assertStatementNode.GetSpan())))));
        AddStatement(ifStatement.release());
    }
}

void StatementBinder::CompileStatement(Node* statementNode, bool setPostfix)
{
    bool prevPostfix = postfix;
    postfix = setPostfix;
    statementNode->Accept(*this);
    postfix = prevPostfix;
}

void StatementBinder::SetCurrentConstructor(ConstructorSymbol* currentConstructorSymbol_, ConstructorNode* currentConstructorNode_)
{
    currentConstructorSymbol = currentConstructorSymbol_;
    currentConstructorNode = currentConstructorNode_;
}

void StatementBinder::SetCurrentDestructor(DestructorSymbol* currentDestructorSymbol_, DestructorNode* currentDestructorNode_)
{
    currentDestructorSymbol = currentDestructorSymbol_;
    currentDestructorNode = currentDestructorNode_;
}

void StatementBinder::SetCurrentMemberFunction(MemberFunctionSymbol* currentMemberFunctionSymbol_, MemberFunctionNode* currentMemberFunctionNode_)
{
    currentMemberFunctionSymbol = currentMemberFunctionSymbol_;
    currentMemberFunctionNode = currentMemberFunctionNode_;
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
