// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/ExpressionBinder.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/binder/BoundExpression.hpp>
#include <cmajor/binder/OverloadResolution.hpp>
#include <cmajor/binder/StatementBinder.hpp>
#include <cmajor/binder/TypeResolver.hpp>
#include <cmajor/binder/Access.hpp>
#include <cmajor/symbols/ClassTypeSymbol.hpp>
#include <cmajor/symbols/ConstantSymbol.hpp>
#include <cmajor/symbols/EnumSymbol.hpp>
#include <cmajor/ast/BasicType.hpp>
#include <cmajor/ast/Literal.hpp>
#include <cmajor/ast/Expression.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/ast/Visitor.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace binder {

using cmajor::parsing::Span;
using namespace cmajor::unicode;

class ExpressionBinder : public cmajor::ast::Visitor
{
public:
    ExpressionBinder(const Span& span_, BoundCompileUnit& boundCompileUnit_, BoundFunction* boundFunction_, ContainerScope* containerScope_, StatementBinder* statementBinder_, bool lvalue_);
    std::unique_ptr<BoundExpression> GetExpression() { return std::move(expression); }

    void Visit(BoolNode& boolNode) override;
    void Visit(SByteNode& sbyteNode) override;
    void Visit(ByteNode& byteNode) override;
    void Visit(ShortNode& shortNode) override;
    void Visit(UShortNode& ushortNode) override;
    void Visit(IntNode& intNode) override;
    void Visit(UIntNode& uintNode) override;
    void Visit(LongNode& longNode) override;
    void Visit(ULongNode& ulongNode) override;
    void Visit(FloatNode& floatNode) override;
    void Visit(DoubleNode& doubleNode) override;
    void Visit(CharNode& charNode) override;
    void Visit(WCharNode& wcharNode) override;
    void Visit(UCharNode& ucharNode) override;
    void Visit(VoidNode& voidNode) override;

    void Visit(BooleanLiteralNode& booleanLiteralNode) override;
    void Visit(SByteLiteralNode& sbyteLiteralBode) override;
    void Visit(ByteLiteralNode& byteLiteralNode) override;
    void Visit(ShortLiteralNode& shortLiteralNode) override;
    void Visit(UShortLiteralNode& ushortLiteralNode) override;
    void Visit(IntLiteralNode& intLiteralNode) override;
    void Visit(UIntLiteralNode& uintLiteralNode) override;
    void Visit(LongLiteralNode& longLiteralNode) override;
    void Visit(ULongLiteralNode& ulongLiteralNode) override;
    void Visit(FloatLiteralNode& floatLiteralNode) override;
    void Visit(DoubleLiteralNode& doubleLiteralNode) override;
    void Visit(CharLiteralNode& charLiteralNode) override;
    void Visit(WCharLiteralNode& wcharLiteralNode) override;
    void Visit(UCharLiteralNode& ucharLiteralNode) override;
    void Visit(StringLiteralNode& stringLiteralNode) override;
    void Visit(WStringLiteralNode& wstringLiteralNode) override;
    void Visit(UStringLiteralNode& ustringLiteralNode) override;
    void Visit(NullLiteralNode& nullLiteralNode) override;

    void Visit(IdentifierNode& identifierNode) override;
    void Visit(DotNode& dotNode) override;
    void Visit(ArrowNode& arrowNode) override;
    void Visit(DisjunctionNode& disjunctionNode) override;
    void Visit(ConjunctionNode& conjunctionNode) override;
    void Visit(BitOrNode& bitOrNode) override;
    void Visit(BitXorNode& bitXorNode) override;
    void Visit(BitAndNode& bitAndNode) override;
    void Visit(EqualNode& equalNode) override;
    void Visit(NotEqualNode& notEqualNode) override;
    void Visit(LessNode& lessNode) override;
    void Visit(GreaterNode& greaterNode) override;
    void Visit(LessOrEqualNode& lessOrEqualNode) override;
    void Visit(GreaterOrEqualNode& greaterOrEqualNode) override;
    void Visit(ShiftLeftNode& shiftLeftNode) override;
    void Visit(ShiftRightNode& shiftRightNode) override;
    void Visit(AddNode& addNode) override;
    void Visit(SubNode& subNode) override;
    void Visit(MulNode& mulNode) override;
    void Visit(DivNode& divNode) override;
    void Visit(RemNode& remNode) override;
    void Visit(NotNode& notNode) override;
    void Visit(UnaryPlusNode& unaryPlusNode) override;
    void Visit(UnaryMinusNode& unaryMinusNode) override;
    void Visit(PrefixIncrementNode& prefixIncrementNode) override;
    void Visit(PrefixDecrementNode& prefixDecrementNode) override;
    void Visit(DerefNode& derefNode) override;
    void Visit(AddrOfNode& addrOfNode) override;
    void Visit(ComplementNode& complementNode) override;
    void Visit(IsNode& isNode) override;
    void Visit(AsNode& asNode) override;
    void Visit(IndexingNode& indexingNode) override;
    void Visit(InvokeNode& invokeNode) override;
    void Visit(PostfixIncrementNode& postfixIncrementNode) override;
    void Visit(PostfixDecrementNode& postfixDecrementNode) override;
    void Visit(SizeOfNode& sizeOfNode) override;
    void Visit(TypeNameNode& typeNameNode) override;
    void Visit(CastNode& castNode) override;
    void Visit(ConstructNode& constructNode) override;
    void Visit(NewNode& newNode) override;
    void Visit(ThisNode& thisNode) override;
    void Visit(BaseNode& baseNode) override;
private:
    Span span;
    BoundCompileUnit& boundCompileUnit;
    SymbolTable& symbolTable;
    BoundFunction* boundFunction;
    ContainerScope* containerScope;
    StatementBinder* statementBinder;
    std::unique_ptr<BoundExpression> expression;
    bool lvalue;
    bool inhibitCompile;
    void BindUnaryOp(BoundExpression* operand, Node& node, const std::u32string& groupName);
    void BindUnaryOp(UnaryNode& unaryNode, const std::u32string& groupName);
    void BindBinaryOp(BinaryNode& binaryNode, const std::u32string& groupName);
    void BindBinaryOp(BoundExpression* left, BoundExpression* right, Node& node, const std::u32string& groupName);
    void BindDerefExpr(Node& node);
    void BindSymbol(Symbol* symbol);
};

ExpressionBinder::ExpressionBinder(const Span& span_, BoundCompileUnit& boundCompileUnit_, BoundFunction* boundFunction_, ContainerScope* containerScope_, StatementBinder* statementBinder_, bool lvalue_) :
    span(span_), boundCompileUnit(boundCompileUnit_), symbolTable(boundCompileUnit.GetSymbolTable()), boundFunction(boundFunction_), containerScope(containerScope_), statementBinder(statementBinder_), 
    lvalue(lvalue_), inhibitCompile(false)
{
}

void ExpressionBinder::BindUnaryOp(BoundExpression* operand, Node& node, const std::u32string& groupName)
{
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::unique_ptr<BoundExpression>(operand));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, operand->GetType()->ClassOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    std::unique_ptr<BoundFunctionCall> operatorFunCall = ResolveOverload(groupName, containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, node.GetSpan());
    CheckAccess(boundFunction->GetFunctionSymbol(), operatorFunCall->GetFunctionSymbol());
    expression.reset(operatorFunCall.release());
}

void ExpressionBinder::BindUnaryOp(UnaryNode& unaryNode, const std::u32string& groupName)
{
    unaryNode.Subject()->Accept(*this);
    BoundExpression* operand = expression.release();
    BindUnaryOp(operand, unaryNode, groupName);
}

void ExpressionBinder::BindBinaryOp(BinaryNode& binaryNode, const std::u32string& groupName)
{
    binaryNode.Left()->Accept(*this);
    BoundExpression* left = expression.release();
    binaryNode.Right()->Accept(*this);
    BoundExpression* right = expression.release();
    BindBinaryOp(left, right, binaryNode, groupName);
}

void ExpressionBinder::BindBinaryOp(BoundExpression* left, BoundExpression* right, Node& node, const std::u32string& groupName)
{
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    arguments.push_back(std::unique_ptr<BoundExpression>(left));
    arguments.push_back(std::unique_ptr<BoundExpression>(right));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, left->GetType()->ClassOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, right->GetType()->ClassOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    std::unique_ptr<BoundFunctionCall> operatorFunCall = ResolveOverload(groupName, containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, node.GetSpan());
    CheckAccess(boundFunction->GetFunctionSymbol(), operatorFunCall->GetFunctionSymbol());
    expression.reset(operatorFunCall.release());
}

void ExpressionBinder::BindSymbol(Symbol* symbol)
{
    switch (symbol->GetSymbolType())
    {
        case SymbolType::functionGroupSymbol:
        {
            FunctionGroupSymbol* functionGroupSymbol = static_cast<FunctionGroupSymbol*>(symbol);
            BoundFunctionGroupExpression* boundFunctionGroupExpression = new BoundFunctionGroupExpression(span, functionGroupSymbol);
            ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
            if (thisParam)
            {
                boundFunctionGroupExpression->SetClassObject(std::unique_ptr<BoundExpression>(new BoundParameter(thisParam)));
            }
            expression.reset(boundFunctionGroupExpression);
            break;
        }
        case SymbolType::classTypeSymbol:
        {
            ClassTypeSymbol* classTypeSymbol = static_cast<ClassTypeSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), classTypeSymbol);
            expression.reset(new BoundTypeExpression(span, classTypeSymbol));
            break;
        }
        case SymbolType::parameterSymbol:
        {
            ParameterSymbol* parameterSymbol = static_cast<ParameterSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), parameterSymbol);
            expression.reset(new BoundParameter(parameterSymbol));
            break;
        }
        case SymbolType::localVariableSymbol:
        {
            LocalVariableSymbol* localVariableSymbol = static_cast<LocalVariableSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), localVariableSymbol);
            expression.reset(new BoundLocalVariable(localVariableSymbol));
            break;
        }
        case SymbolType::memberVariableSymbol:
        {
            MemberVariableSymbol* memberVariableSymbol = static_cast<MemberVariableSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), memberVariableSymbol);
            BoundMemberVariable* bmv = new BoundMemberVariable(memberVariableSymbol);
            bool accessFromOwnScope = false;
            Symbol* currentFuctionSymbol = boundFunction->GetFunctionSymbol();
            ClassTypeSymbol* currentClass = currentFuctionSymbol->ContainingClassNoThrow();
            if (currentClass)
            {
                ClassTypeSymbol* cp = memberVariableSymbol->ContainingClassNoThrow();
                Assert(cp, "class type symbol expected");
                if (cp == currentClass)
                {
                    accessFromOwnScope = true;
                }
            }
            if (memberVariableSymbol->IsStatic())
            {
                if (!accessFromOwnScope)
                {
                    bmv->SetStaticInitNeeded();
                }
            }
            else
            {
                ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
                if (accessFromOwnScope)
                {
                    if (thisParam)
                    {
                        TypeSymbol* thisPointerType = thisParam->GetType()->BaseType()->AddPointer(span);
                        if (thisParam->GetType()->IsConstType())
                        {
                            thisPointerType = thisPointerType->AddConst(span);
                        }
                        bmv->SetClassPtr(std::unique_ptr<BoundExpression>(new BoundParameter(thisParam)));
                            
                    }
                    else
                    {
                        Assert(false, "this param expected");
                    }
                }
                else if (thisParam)
                {
                    ClassTypeSymbol* containingClass = memberVariableSymbol->ContainingClassNoThrow();
                    TypeSymbol* containingClassPointerType = containingClass->AddPointer(span);
                    TypeSymbol* thisPointerType = thisParam->GetType()->AddPointer(span);
                    if (thisParam->GetType()->IsConstType())
                    {
                        thisPointerType = thisPointerType->AddConst(span);
                        containingClassPointerType->AddConst(span);
                    }
                    FunctionSymbol* conversionFun = boundCompileUnit.GetConversion(thisPointerType, containingClassPointerType, span);
                    if (conversionFun)
                    {
                        bmv->SetClassPtr(std::unique_ptr<BoundExpression>(new BoundConversion(std::unique_ptr<BoundExpression>(new BoundParameter(thisParam)), conversionFun)));
                    }
                    else
                    {
                        throw Exception("cannot convert from '" + ToUtf8(thisParam->GetType()->FullName()) + "' to '" + ToUtf8(containingClassPointerType->FullName()) + "'", span);
                    }
                }
            }
            expression.reset(bmv);
            break;
        }
        case SymbolType::constantSymbol:
        {
            ConstantSymbol* constantSymbol = static_cast<ConstantSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), constantSymbol);
            expression.reset(new BoundConstant(constantSymbol));
            break;
        }
        case SymbolType::enumTypeSymbol:
        {
            EnumTypeSymbol* enumTypeSymbol = static_cast<EnumTypeSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), enumTypeSymbol);
            expression.reset(new BoundTypeExpression(span, enumTypeSymbol));
            break;
        }
        case SymbolType::enumConstantSymbol:
        {
            EnumConstantSymbol* enumConstantSymbol = static_cast<EnumConstantSymbol*>(symbol);
            expression.reset(new BoundEnumConstant(enumConstantSymbol));
            break;
        }
        case SymbolType::namespaceSymbol:
        {
            NamespaceSymbol* ns = static_cast<NamespaceSymbol*>(symbol);
            expression.reset(new BoundNamespaceExpression(span, ns));
            break;
        }
        default:
        {
            throw Exception("could not bind '" + ToUtf8(symbol->FullName()) + "'", span, symbol->GetSpan());
        }
    }
}

void ExpressionBinder::Visit(BoolNode& boolNode)
{
    expression.reset(new BoundTypeExpression(boolNode.GetSpan(), symbolTable.GetTypeByName(U"bool")));
}

void ExpressionBinder::Visit(SByteNode& sbyteNode)
{
    expression.reset(new BoundTypeExpression(sbyteNode.GetSpan(), symbolTable.GetTypeByName(U"sbyte")));
}

void ExpressionBinder::Visit(ByteNode& byteNode)
{
    expression.reset(new BoundTypeExpression(byteNode.GetSpan(), symbolTable.GetTypeByName(U"byte")));
}

void ExpressionBinder::Visit(ShortNode& shortNode)
{
    expression.reset(new BoundTypeExpression(shortNode.GetSpan(), symbolTable.GetTypeByName(U"short")));
}

void ExpressionBinder::Visit(UShortNode& ushortNode)
{
    expression.reset(new BoundTypeExpression(ushortNode.GetSpan(), symbolTable.GetTypeByName(U"ushort")));
}

void ExpressionBinder::Visit(IntNode& intNode)
{
    expression.reset(new BoundTypeExpression(intNode.GetSpan(), symbolTable.GetTypeByName(U"int")));
}

void ExpressionBinder::Visit(UIntNode& uintNode)
{
    expression.reset(new BoundTypeExpression(uintNode.GetSpan(), symbolTable.GetTypeByName(U"uint")));
}

void ExpressionBinder::Visit(LongNode& longNode)
{
    expression.reset(new BoundTypeExpression(longNode.GetSpan(), symbolTable.GetTypeByName(U"long")));
}

void ExpressionBinder::Visit(ULongNode& ulongNode)
{
    expression.reset(new BoundTypeExpression(ulongNode.GetSpan(), symbolTable.GetTypeByName(U"ulong")));
}

void ExpressionBinder::Visit(FloatNode& floatNode)
{
    expression.reset(new BoundTypeExpression(floatNode.GetSpan(), symbolTable.GetTypeByName(U"float")));
}

void ExpressionBinder::Visit(DoubleNode& doubleNode)
{
    expression.reset(new BoundTypeExpression(doubleNode.GetSpan(), symbolTable.GetTypeByName(U"double")));
}

void ExpressionBinder::Visit(CharNode& charNode)
{
    expression.reset(new BoundTypeExpression(charNode.GetSpan(), symbolTable.GetTypeByName(U"char")));
}

void ExpressionBinder::Visit(WCharNode& wcharNode)
{
    expression.reset(new BoundTypeExpression(wcharNode.GetSpan(), symbolTable.GetTypeByName(U"wchar")));
}

void ExpressionBinder::Visit(UCharNode& ucharNode)
{
    expression.reset(new BoundTypeExpression(ucharNode.GetSpan(), symbolTable.GetTypeByName(U"uchar")));
}

void ExpressionBinder::Visit(VoidNode& voidNode)
{
    expression.reset(new BoundTypeExpression(voidNode.GetSpan(), symbolTable.GetTypeByName(U"void")));
}

void ExpressionBinder::Visit(BooleanLiteralNode& booleanLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new BoolValue(booleanLiteralNode.GetSpan(), booleanLiteralNode.Value())), symbolTable.GetTypeByName(U"bool")));
}

void ExpressionBinder::Visit(SByteLiteralNode& sbyteLiteralBode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new SByteValue(sbyteLiteralBode.GetSpan(), sbyteLiteralBode.Value())), symbolTable.GetTypeByName(U"sbyte")));
}

void ExpressionBinder::Visit(ByteLiteralNode& byteLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new ByteValue(byteLiteralNode.GetSpan(), byteLiteralNode.Value())), symbolTable.GetTypeByName(U"byte")));
}

void ExpressionBinder::Visit(ShortLiteralNode& shortLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new ShortValue(shortLiteralNode.GetSpan(), shortLiteralNode.Value())), symbolTable.GetTypeByName(U"short")));
}

void ExpressionBinder::Visit(UShortLiteralNode& ushortLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new UShortValue(ushortLiteralNode.GetSpan(), ushortLiteralNode.Value())), symbolTable.GetTypeByName(U"ushort")));
}

void ExpressionBinder::Visit(IntLiteralNode& intLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new IntValue(intLiteralNode.GetSpan(), intLiteralNode.Value())), symbolTable.GetTypeByName(U"int")));
}

void ExpressionBinder::Visit(UIntLiteralNode& uintLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new UIntValue(uintLiteralNode.GetSpan(), uintLiteralNode.Value())), symbolTable.GetTypeByName(U"uint")));
}

void ExpressionBinder::Visit(LongLiteralNode& longLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new LongValue(longLiteralNode.GetSpan(), longLiteralNode.Value())), symbolTable.GetTypeByName(U"long")));
}

void ExpressionBinder::Visit(ULongLiteralNode& ulongLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new ULongValue(ulongLiteralNode.GetSpan(), ulongLiteralNode.Value())), symbolTable.GetTypeByName(U"ulong")));
}

void ExpressionBinder::Visit(FloatLiteralNode& floatLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new FloatValue(floatLiteralNode.GetSpan(), floatLiteralNode.Value())), symbolTable.GetTypeByName(U"float")));
}

void ExpressionBinder::Visit(DoubleLiteralNode& doubleLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new DoubleValue(doubleLiteralNode.GetSpan(), doubleLiteralNode.Value())), symbolTable.GetTypeByName(U"double")));
}

void ExpressionBinder::Visit(CharLiteralNode& charLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new CharValue(charLiteralNode.GetSpan(), charLiteralNode.Value())), symbolTable.GetTypeByName(U"char")));
}

void ExpressionBinder::Visit(WCharLiteralNode& wcharLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new WCharValue(wcharLiteralNode.GetSpan(), wcharLiteralNode.Value())), symbolTable.GetTypeByName(U"wchar")));
}

void ExpressionBinder::Visit(UCharLiteralNode& ucharLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new UCharValue(ucharLiteralNode.GetSpan(), ucharLiteralNode.Value())), symbolTable.GetTypeByName(U"uchar")));
}

void ExpressionBinder::Visit(StringLiteralNode& stringLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new StringValue(stringLiteralNode.GetSpan(), stringLiteralNode.Value())),
        symbolTable.GetTypeByName(U"char")->AddConst(stringLiteralNode.GetSpan())->AddPointer(stringLiteralNode.GetSpan())));
}

void ExpressionBinder::Visit(WStringLiteralNode& wstringLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new StringValue(wstringLiteralNode.GetSpan(), ToUtf8(wstringLiteralNode.Value()))),
        symbolTable.GetTypeByName(U"char")->AddConst(wstringLiteralNode.GetSpan())->AddPointer(wstringLiteralNode.GetSpan())));
}

void ExpressionBinder::Visit(UStringLiteralNode& ustringLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new StringValue(ustringLiteralNode.GetSpan(), ToUtf8(ustringLiteralNode.Value()))),
        symbolTable.GetTypeByName(U"char")->AddConst(ustringLiteralNode.GetSpan())->AddPointer(ustringLiteralNode.GetSpan())));
}

void ExpressionBinder::Visit(NullLiteralNode& nullLiteralNode) 
{
    TypeSymbol* nullPtrType = symbolTable.GetTypeByName(U"@nullptr_type");
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new NullValue(nullLiteralNode.GetSpan(), nullPtrType)), nullPtrType));
}

void ExpressionBinder::Visit(IdentifierNode& identifierNode)
{
    std::u32string name = identifierNode.Str();
    Symbol* symbol = containerScope->Lookup(name, ScopeLookup::this_and_base_and_parent);
    if (!symbol)
    {
        for (const std::unique_ptr<FileScope>& fileScope : boundCompileUnit.FileScopes())
        {
            symbol = fileScope->Lookup(name);
            if (symbol)
            {
                break;
            }
        }
    }
    if (symbol)
    {
        BindSymbol(symbol);
    }
    else
    {
        throw Exception("symbol '" + ToUtf8(name) + "' not found", identifierNode.GetSpan());
    }
}

void ExpressionBinder::Visit(DotNode& dotNode)
{
    // todo
}

void ExpressionBinder::Visit(ArrowNode& arrowNode) 
{
    // todo
}

void ExpressionBinder::Visit(DisjunctionNode& disjunctionNode) 
{
    // todo
}

void ExpressionBinder::Visit(ConjunctionNode& conjunctionNode) 
{
    // todo
}

void ExpressionBinder::Visit(BitOrNode& bitOrNode) 
{
    BindBinaryOp(bitOrNode, U"operator|");
}

void ExpressionBinder::Visit(BitXorNode& bitXorNode) 
{
    BindBinaryOp(bitXorNode, U"operator^");
}

void ExpressionBinder::Visit(BitAndNode& bitAndNode) 
{
    BindBinaryOp(bitAndNode, U"operator&");
}

void ExpressionBinder::Visit(EqualNode& equalNode) 
{
    BindBinaryOp(equalNode, U"operator==");
}

void ExpressionBinder::Visit(NotEqualNode& notEqualNode) 
{
    BindBinaryOp(notEqualNode, U"operator==");
    BindUnaryOp(expression.release(), notEqualNode, U"operator!");
}

void ExpressionBinder::Visit(LessNode& lessNode) 
{
    BindBinaryOp(lessNode, U"operator<");
}

void ExpressionBinder::Visit(GreaterNode& greaterNode) 
{
    greaterNode.Left()->Accept(*this);
    BoundExpression* left = expression.release();
    greaterNode.Right()->Accept(*this);
    BoundExpression* right = expression.release();
    BindBinaryOp(right, left, greaterNode, U"operator<");
}

void ExpressionBinder::Visit(LessOrEqualNode& lessOrEqualNode) 
{
    lessOrEqualNode.Left()->Accept(*this);
    BoundExpression* left = expression.release();
    lessOrEqualNode.Right()->Accept(*this);
    BoundExpression* right = expression.release();
    BindBinaryOp(right, left, lessOrEqualNode, U"operator<");
    BindUnaryOp(expression.release(), lessOrEqualNode, U"operator!");
}

void ExpressionBinder::Visit(GreaterOrEqualNode& greaterOrEqualNode) 
{
    BindBinaryOp(greaterOrEqualNode, U"operator<");
    BindUnaryOp(expression.release(), greaterOrEqualNode, U"operator!");
}

void ExpressionBinder::Visit(ShiftLeftNode& shiftLeftNode) 
{
    BindBinaryOp(shiftLeftNode, U"operator<<");
}

void ExpressionBinder::Visit(ShiftRightNode& shiftRightNode) 
{
    BindBinaryOp(shiftRightNode, U"operator>>");
}

void ExpressionBinder::Visit(AddNode& addNode) 
{
    BindBinaryOp(addNode, U"operator+");
}

void ExpressionBinder::Visit(SubNode& subNode) 
{
    BindBinaryOp(subNode, U"operator-");
}

void ExpressionBinder::Visit(MulNode& mulNode) 
{
    BindBinaryOp(mulNode, U"operator*");
}

void ExpressionBinder::Visit(DivNode& divNode) 
{
    BindBinaryOp(divNode, U"operator/");
}

void ExpressionBinder::Visit(RemNode& remNode) 
{
    BindBinaryOp(remNode, U"operator%");
}

void ExpressionBinder::Visit(NotNode& notNode) 
{
    BindUnaryOp(notNode, U"operator!");
}

void ExpressionBinder::Visit(UnaryPlusNode& unaryPlusNode) 
{
    BindUnaryOp(unaryPlusNode, U"operator+");
}

void ExpressionBinder::Visit(UnaryMinusNode& unaryMinusNode) 
{
    BindUnaryOp(unaryMinusNode, U"operator-");
}

void ExpressionBinder::Visit(PrefixIncrementNode& prefixIncrementNode) 
{
    prefixIncrementNode.Subject()->Accept(*this);
    if (expression->GetType()->PlainType(prefixIncrementNode.GetSpan())->IsClassTypeSymbol())
    {
        BindUnaryOp(prefixIncrementNode, U"operator++");
    }
    else
    {
        if (!inhibitCompile)
        {
            if (expression->GetType()->IsUnsignedType())
            {
                CloneContext cloneContext;
                AssignmentStatementNode assignmentStatement(prefixIncrementNode.GetSpan(), prefixIncrementNode.Subject()->Clone(cloneContext),
                    new AddNode(prefixIncrementNode.GetSpan(), prefixIncrementNode.Subject()->Clone(cloneContext), new ByteLiteralNode(prefixIncrementNode.GetSpan(), 1u)));
                statementBinder->CompileStatement(&assignmentStatement, false);
            }
            else
            {
                CloneContext cloneContext;
                AssignmentStatementNode assignmentStatement(prefixIncrementNode.GetSpan(), prefixIncrementNode.Subject()->Clone(cloneContext),
                    new AddNode(prefixIncrementNode.GetSpan(), prefixIncrementNode.Subject()->Clone(cloneContext), new SByteLiteralNode(prefixIncrementNode.GetSpan(), 1)));
                statementBinder->CompileStatement(&assignmentStatement, false);
            }
        }
        bool prevInhibitCompile = inhibitCompile;
        inhibitCompile = true;
        prefixIncrementNode.Subject()->Accept(*this);
        inhibitCompile = prevInhibitCompile;
    }
}

void ExpressionBinder::Visit(PrefixDecrementNode& prefixDecrementNode) 
{
    prefixDecrementNode.Subject()->Accept(*this);
    if (expression->GetType()->PlainType(prefixDecrementNode.GetSpan())->IsClassTypeSymbol())
    {
        BindUnaryOp(prefixDecrementNode, U"operator--");
    }
    else
    {
        if (!inhibitCompile)
        {
            if (expression->GetType()->IsUnsignedType())
            {
                CloneContext cloneContext;
                AssignmentStatementNode assignmentStatement(prefixDecrementNode.GetSpan(), prefixDecrementNode.Subject()->Clone(cloneContext),
                    new SubNode(prefixDecrementNode.GetSpan(), prefixDecrementNode.Subject()->Clone(cloneContext), new ByteLiteralNode(prefixDecrementNode.GetSpan(), 1u)));
                statementBinder->CompileStatement(&assignmentStatement, false);
            }
            else
            {
                CloneContext cloneContext;
                AssignmentStatementNode assignmentStatement(prefixDecrementNode.GetSpan(), prefixDecrementNode.Subject()->Clone(cloneContext),
                    new SubNode(prefixDecrementNode.GetSpan(), prefixDecrementNode.Subject()->Clone(cloneContext), new SByteLiteralNode(prefixDecrementNode.GetSpan(), 1)));
                statementBinder->CompileStatement(&assignmentStatement, false);
            }
        }
        bool prevInhibitCompile = inhibitCompile;
        inhibitCompile = true;
        prefixDecrementNode.Subject()->Accept(*this);
        inhibitCompile = prevInhibitCompile;
    }
}

void ExpressionBinder::BindDerefExpr(Node& node)
{
    if (expression->GetType()->IsPointerType())
    {
        TypeSymbol* type = expression->GetType()->RemovePointer(node.GetSpan());
        expression.reset(new BoundDereferenceExpression(std::unique_ptr<BoundExpression>(expression.release()), type));
    }
    else 
    {
        TypeSymbol* plainSubjectType = expression->GetType()->PlainType(node.GetSpan());
        if (plainSubjectType->IsClassTypeSymbol())
        {
            BindUnaryOp(expression.release(), node, U"operator*");
        }
        else
        {
            throw Exception("dereference needs pointer or class type argument", node.GetSpan());
        }
    }
}

void ExpressionBinder::Visit(DerefNode& derefNode) 
{
    derefNode.Subject()->Accept(*this);
    BindDerefExpr(derefNode);
}

void ExpressionBinder::Visit(AddrOfNode& addrOfNode) 
{
    addrOfNode.Subject()->Accept(*this);
    if (expression->IsLvalueExpression())
    {
        TypeSymbol* type = expression->GetType()->AddPointer(addrOfNode.GetSpan());
        expression.reset(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(expression.release()), type));
    }
    else
    {
        throw Exception("cannot take address of " + expression->TypeString(), addrOfNode.GetSpan());
    }
}

void ExpressionBinder::Visit(ComplementNode& complementNode) 
{
    BindUnaryOp(complementNode, U"operator~");
}

void ExpressionBinder::Visit(IsNode& isNode) 
{
    // todo
}

void ExpressionBinder::Visit(AsNode& asNode) 
{
    // todo
}

void ExpressionBinder::Visit(IndexingNode& indexingNode) 
{
    indexingNode.Subject()->Accept(*this);
    std::unique_ptr<BoundExpression> subject = std::move(expression);
    indexingNode.Index()->Accept(*this);
    std::unique_ptr<BoundExpression> index = std::move(expression);
    TypeSymbol* plainSubjectType = subject->GetType()->PlainType(indexingNode.GetSpan());
    if (plainSubjectType->IsClassTypeSymbol())
    {
        BindBinaryOp(subject.release(), index.release(), indexingNode, U"operator[]");
    }
    else  if (plainSubjectType->IsPointerType())
    {
        BindBinaryOp(subject.release(), index.release(), indexingNode, U"operator+");
        BindDerefExpr(indexingNode);
    }
    else if (plainSubjectType->IsArrayType())
    {
        // todo
    }
    else
    {
        throw Exception("subscript operator can be applied only to pointer, array or class type subject", indexingNode.GetSpan());
    }
}

void ExpressionBinder::Visit(InvokeNode& invokeNode) 
{
    invokeNode.Subject()->Accept(*this);
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    FunctionGroupSymbol* functionGroupSymbol = nullptr;
    bool scopeQualified = false;
    if (expression->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExcpression)
    {
        BoundFunctionGroupExpression* bfge = static_cast<BoundFunctionGroupExpression*>(expression.get());
        functionGroupSymbol = bfge->FunctionGroup();
        if (bfge->IsScopeQualified())
        {
            functionScopeLookups.clear();
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, bfge->QualifiedScope()));
            scopeQualified = true;
        }
    }
    else
    {
        throw Exception("invoke cannot be applied to this type of expression", invokeNode.Subject()->GetSpan());
    }
    int n = invokeNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = invokeNode.Arguments()[i];
        argument->Accept(*this);
        if (expression->GetType()->GetSymbolType() != SymbolType::functionGroupTypeSymbol && !scopeQualified)
        {
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, expression->GetType()->ClassInterfaceOrNsScope()));
        }
        arguments.push_back(std::unique_ptr<BoundExpression>(expression.release()));
    }
    if (!scopeQualified)
    {
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    }
    std::unique_ptr<Exception> exception;
    std::unique_ptr<Exception> thisEx;
    std::unique_ptr<Exception> nsEx;
    std::unique_ptr<BoundFunctionCall> functionCall = ResolveOverload(functionGroupSymbol->Name(), containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, 
        invokeNode.GetSpan(), OverloadResolutionFlags::dontThrow, exception);
    if (!functionCall)
    {
        ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
        bool thisParamInserted = false;
        if (thisParam)
        {
            BoundParameter* boundThisParam = new BoundParameter(thisParam);
            arguments.insert(arguments.begin(), std::unique_ptr<BoundExpression>(boundThisParam));
            thisParamInserted = true;
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base, thisParam->GetType()->ClassInterfaceOrNsScope()));
            functionCall = std::move(ResolveOverload(functionGroupSymbol->Name(), containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, invokeNode.GetSpan(),
                OverloadResolutionFlags::dontThrow, thisEx));
        }
        if (!functionCall)
        {
            if (thisParamInserted)
            {
                arguments.erase(arguments.begin());
            }
            if (!arguments.empty())
            {
                arguments.erase(arguments.begin());
            }
            functionCall = std::move(ResolveOverload(functionGroupSymbol->Name(), containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, invokeNode.GetSpan(),
                OverloadResolutionFlags::dontThrow, nsEx));
        }
    }
    if (!functionCall)
    {
        if (CastOverloadException* castException = dynamic_cast<CastOverloadException*>(exception.get()))
        {
            throw *exception;
        }
        if (CastOverloadException* castException = dynamic_cast<CastOverloadException*>(thisEx.get()))
        {
            throw *thisEx;
        }
        if (CastOverloadException* castException = dynamic_cast<CastOverloadException*>(nsEx.get()))
        {
            throw *nsEx;
        }
        if (CannotBindConstToNonconstOverloadException* bindException = dynamic_cast<CannotBindConstToNonconstOverloadException*>(exception.get()))
        {
            throw *exception;
        }
        if (CannotBindConstToNonconstOverloadException* bindException = dynamic_cast<CannotBindConstToNonconstOverloadException*>(thisEx.get()))
        {
            throw *thisEx;
        }
        if (CannotBindConstToNonconstOverloadException* bindException = dynamic_cast<CannotBindConstToNonconstOverloadException*>(nsEx.get()))
        {
            throw *nsEx;
        }
        if (CannotAssignToConstOverloadException* assignmentException = dynamic_cast<CannotAssignToConstOverloadException*>(exception.get()))
        {
            throw *exception;
        }
        if (CannotAssignToConstOverloadException* assignmentException = dynamic_cast<CannotAssignToConstOverloadException*>(thisEx.get()))
        {
            throw *thisEx;
        }
        if (CannotAssignToConstOverloadException* assignmentException = dynamic_cast<CannotAssignToConstOverloadException*>(nsEx.get()))
        {
            throw *nsEx;
        }
        if (exception.get())
        {
            throw *exception;
        }
        else if (thisEx.get())
        {
            throw *thisEx;
        }
        else if (nsEx.get())
        {
            throw *nsEx;
        }
        else
        {
            throw Exception("overload resolution failed: overload not found", invokeNode.GetSpan());
        }
    }
    CheckAccess(boundFunction->GetFunctionSymbol(), functionCall->GetFunctionSymbol());
    expression.reset(functionCall.release());
}

void ExpressionBinder::Visit(PostfixIncrementNode& postfixIncrementNode)
{
    bool prevInhibitCompile = inhibitCompile;
    inhibitCompile = true;
    postfixIncrementNode.Subject()->Accept(*this);
    inhibitCompile = prevInhibitCompile;
    if (!inhibitCompile)
    {
        if (expression->GetType()->PlainType(postfixIncrementNode.GetSpan())->IsClassTypeSymbol())
        {
            CloneContext cloneContext;
            ExpressionStatementNode prefixIncrementExpression(postfixIncrementNode.GetSpan(), new PrefixIncrementNode(postfixIncrementNode.GetSpan(), 
                postfixIncrementNode.Subject()->Clone(cloneContext)));
            statementBinder->CompileStatement(&prefixIncrementExpression, true);
        }
        else
        {
            if (expression->GetType()->IsUnsignedType())
            {
                CloneContext cloneContext;
                AssignmentStatementNode assignmentStatement(postfixIncrementNode.GetSpan(), postfixIncrementNode.Subject()->Clone(cloneContext),
                    new AddNode(postfixIncrementNode.GetSpan(), postfixIncrementNode.Subject()->Clone(cloneContext), new ByteLiteralNode(postfixIncrementNode.GetSpan(), 1u)));
                statementBinder->CompileStatement(&assignmentStatement, true);
            }
            else
            {
                CloneContext cloneContext;
                AssignmentStatementNode assignmentStatement(postfixIncrementNode.GetSpan(), postfixIncrementNode.Subject()->Clone(cloneContext),
                    new AddNode(postfixIncrementNode.GetSpan(), postfixIncrementNode.Subject()->Clone(cloneContext), new SByteLiteralNode(postfixIncrementNode.GetSpan(), 1)));
                statementBinder->CompileStatement(&assignmentStatement, true);
            }
        }
    }
    postfixIncrementNode.Subject()->Accept(*this);
}

void ExpressionBinder::Visit(PostfixDecrementNode& postfixDecrementNode)
{
    bool prevInhibitCompile = inhibitCompile;
    inhibitCompile = true;
    postfixDecrementNode.Subject()->Accept(*this);
    inhibitCompile = prevInhibitCompile;
    if (!inhibitCompile)
    {
        if (expression->GetType()->PlainType(postfixDecrementNode.GetSpan())->IsClassTypeSymbol())
        {
            CloneContext cloneContext;
            ExpressionStatementNode prefixDecrementExpression(postfixDecrementNode.GetSpan(), new PrefixDecrementNode(postfixDecrementNode.GetSpan(),
                postfixDecrementNode.Subject()->Clone(cloneContext)));
            statementBinder->CompileStatement(&prefixDecrementExpression, true);
        }
        else
        {
            if (expression->GetType()->IsUnsignedType())
            {
                CloneContext cloneContext;
                AssignmentStatementNode assignmentStatement(postfixDecrementNode.GetSpan(), postfixDecrementNode.Subject()->Clone(cloneContext),
                    new AddNode(postfixDecrementNode.GetSpan(), postfixDecrementNode.Subject()->Clone(cloneContext), new ByteLiteralNode(postfixDecrementNode.GetSpan(), 1u)));
                statementBinder->CompileStatement(&assignmentStatement, true);
            }
            else
            {
                CloneContext cloneContext;
                AssignmentStatementNode assignmentStatement(postfixDecrementNode.GetSpan(), postfixDecrementNode.Subject()->Clone(cloneContext),
                    new AddNode(postfixDecrementNode.GetSpan(), postfixDecrementNode.Subject()->Clone(cloneContext), new SByteLiteralNode(postfixDecrementNode.GetSpan(), 1)));
                statementBinder->CompileStatement(&assignmentStatement, true);
            }
        }
    }
    postfixDecrementNode.Subject()->Accept(*this);
}

void ExpressionBinder::Visit(SizeOfNode& sizeOfNode) 
{
    sizeOfNode.Expression()->Accept(*this);
    expression.reset(new BoundSizeOfExpression(sizeOfNode.GetSpan(), symbolTable.GetTypeByName(U"long"), expression->GetType()->AddPointer(sizeOfNode.GetSpan())));
}

void ExpressionBinder::Visit(TypeNameNode& typeNameNode) 
{
    // todo
}

void ExpressionBinder::Visit(CastNode& castNode) 
{
    TypeSymbol* targetType = ResolveType(castNode.TargetTypeExpr(), boundCompileUnit, containerScope);
    castNode.SourceExpr()->Accept(*this);
    if (targetType != expression->GetType())
    {
        std::vector<std::unique_ptr<BoundExpression>> targetExprArgs;
        targetExprArgs.push_back(std::unique_ptr<BoundExpression>(new BoundTypeExpression(castNode.GetSpan(), targetType)));
        std::vector<FunctionScopeLookup> functionScopeLookups;
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, targetType->ClassInterfaceOrNsScope()));
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
        std::unique_ptr<BoundFunctionCall> castFunctionCall = ResolveOverload(U"@return", containerScope, functionScopeLookups, targetExprArgs, boundCompileUnit, boundFunction, castNode.GetSpan());
        std::vector<std::unique_ptr<BoundExpression>> castArguments;
        castArguments.push_back(std::move(expression));
        FunctionMatch functionMatch(castFunctionCall->GetFunctionSymbol());
        bool conversionFound = FindConversions(boundCompileUnit, castFunctionCall->GetFunctionSymbol(), castArguments, functionMatch, ConversionType::explicit_, castNode.GetSpan());
        if (conversionFound)
        {
            Assert(!functionMatch.argumentMatches.empty(), "argument match expected");
            FunctionSymbol* conversionFun = functionMatch.argumentMatches[0].conversionFun;
            if (conversionFun)
            {
                castArguments[0].reset(new BoundConversion(std::unique_ptr<BoundExpression>(castArguments[0].release()), conversionFun));
            }
            ArgumentMatch& argumentMatch = functionMatch.argumentMatches[0];
            if (argumentMatch.referenceConversionFlags != OperationFlags::none)
            {
                if (argumentMatch.referenceConversionFlags == OperationFlags::addr)
                {
                    TypeSymbol* type = castArguments[0]->GetType()->AddLvalueReference(span);
                    BoundAddressOfExpression* addressOfExpression = new BoundAddressOfExpression(std::move(castArguments[0]), type);
                    castArguments[0].reset(addressOfExpression);
                }
                else if (argumentMatch.referenceConversionFlags == OperationFlags::deref)
                {
                    TypeSymbol* type = castArguments[0]->GetType()->RemoveReference(span);
                    BoundDereferenceExpression* dereferenceExpression = new BoundDereferenceExpression(std::move(castArguments[0]), type);
                    castArguments[0].reset(dereferenceExpression);
                }
            }
            castFunctionCall->SetArguments(std::move(castArguments));
        }
        else
        {
            throw Exception("no explicit conversion from '" + ToUtf8(castArguments[0]->GetType()->FullName()) + "' to '" + ToUtf8(targetType->FullName()) + "' exists",
                castNode.GetSpan(), boundFunction->GetFunctionSymbol()->GetSpan());
        }
        CheckAccess(boundFunction->GetFunctionSymbol(), castFunctionCall->GetFunctionSymbol());
        expression.reset(castFunctionCall.release());
    }
}

void ExpressionBinder::Visit(ConstructNode& constructNode) 
{
    TypeSymbol* resultType = nullptr;
    int n = constructNode.Arguments().Count();
    if (n == 0)
    {
        throw Exception("must supply at least one argument to construct expression", constructNode.GetSpan());
    }
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    for (int i = 0; i < n; ++i)
    {
        Node* argumentNode = constructNode.Arguments()[i];
        if (i == 0)
        {
            CloneContext cloneContext;
            CastNode castNode(constructNode.GetSpan(), new PointerNode(constructNode.GetSpan(), constructNode.TypeExpr()->Clone(cloneContext)), argumentNode->Clone(cloneContext));
            castNode.Accept(*this);
            resultType = expression->GetType();
            BindDerefExpr(constructNode);
            if (!resultType->IsPointerType())
            {
                throw Exception("first argument of a construct expression must be of a pointer type", argumentNode->GetSpan());
            }
        }
        else
        {
            argumentNode->Accept(*this);
        }
        arguments.push_back(std::move(expression));
    }
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, resultType->RemovePointer(constructNode.GetSpan())->ClassInterfaceOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    expression = ResolveOverload(U"@constructor", containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, constructNode.GetSpan());
    expression.reset(new BoundConstructExpression(std::move(expression), resultType));
}

void ExpressionBinder::Visit(NewNode& newNode)
{
    CloneContext cloneContext;
    InvokeNode* invokeMemAlloc = new InvokeNode(newNode.GetSpan(), new IdentifierNode(newNode.GetSpan(), U"RtMemAlloc"));
    invokeMemAlloc->AddArgument(new SizeOfNode(newNode.GetSpan(), newNode.TypeExpr()->Clone(cloneContext)));
    CastNode castNode(newNode.GetSpan(), new PointerNode(newNode.GetSpan(), newNode.TypeExpr()->Clone(cloneContext)), invokeMemAlloc);
    castNode.Accept(*this);
    std::vector<std::unique_ptr<BoundExpression>> arguments;
    TypeSymbol* resultType = expression->GetType();
    BindDerefExpr(newNode);
    arguments.push_back(std::move(expression));
    int n = newNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        newNode.Arguments()[i]->Accept(*this);
        arguments.push_back(std::move(expression));
    }
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, resultType->RemovePointer(newNode.GetSpan())->ClassInterfaceOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    expression = ResolveOverload(U"@constructor", containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, newNode.GetSpan());
    expression.reset(new BoundConstructExpression(std::move(expression), resultType));
}

void ExpressionBinder::Visit(ThisNode& thisNode) 
{
    ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
    if (thisParam)
    {
        expression.reset(new BoundParameter(thisParam));
    }
    else
    {
        throw Exception("'this' can only be used in member function context", thisNode.GetSpan());
    }
}

void ExpressionBinder::Visit(BaseNode& baseNode) 
{
    ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
    if (thisParam)
    {
        TypeSymbol* thisType = thisParam->GetType()->BaseType();
        if (thisType->IsClassTypeSymbol())
        {
            ClassTypeSymbol* thisClassType = static_cast<ClassTypeSymbol*>(thisType);
            if (thisClassType->BaseClass())
            {
                TypeSymbol* basePointerType = thisClassType->BaseClass()->AddPointer(baseNode.GetSpan());
                if (thisParam->GetType()->IsConstType())
                {
                    basePointerType = basePointerType->AddConst(baseNode.GetSpan());
                }
                FunctionSymbol* thisAsBaseConversionFunction = boundCompileUnit.GetConversion(thisParam->GetType(), basePointerType, baseNode.GetSpan());
                if (thisAsBaseConversionFunction)
                {
                    expression.reset(new BoundConversion(std::unique_ptr<BoundExpression>(new BoundParameter(thisParam)), thisAsBaseConversionFunction));
                }
                else
                {
                    throw Exception("cannot convert from '" + ToUtf8(thisParam->GetType()->FullName()) + "' to '" + ToUtf8(basePointerType->FullName()) + "'", baseNode.GetSpan());
                }
            }
            else
            {
                throw Exception("class '" + ToUtf8(thisClassType->FullName()) + "' does not have a base class", baseNode.GetSpan());
            }
        }
        else
        {
            throw Exception("'base' can only be used in member function context", baseNode.GetSpan());
        }
    }
    else
    {
        throw Exception("'base' can only be used in member function context", baseNode.GetSpan());
    }
}

std::unique_ptr<BoundExpression> BindExpression(Node* node, BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, StatementBinder* statementBinder)
{
    return BindExpression(node, boundCompileUnit, boundFunction, containerScope, statementBinder, false);
}

std::unique_ptr<BoundExpression> BindExpression(Node* node, BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, StatementBinder* statementBinder, bool lvalue)
{
    return BindExpression(node, boundCompileUnit, boundFunction, containerScope, statementBinder, lvalue, false, false);
}

std::unique_ptr<BoundExpression> BindExpression(Node* node, BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, StatementBinder* statementBinder, bool lvalue,
    bool acceptFunctionGroup, bool acceptMemberExpression)
{
    return BindExpression(node, boundCompileUnit, boundFunction, containerScope, statementBinder, lvalue, acceptFunctionGroup, acceptMemberExpression, false);
}

std::unique_ptr<BoundExpression> BindExpression(Node* node, BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, StatementBinder* statementBinder, bool lvalue,
    bool acceptFunctionGroup, bool acceptMemberExpression, bool acceptIncomplete)
{
    ExpressionBinder expressionBinder(node->GetSpan(), boundCompileUnit, boundFunction, containerScope, statementBinder, lvalue);
    node->Accept(expressionBinder);
    std::unique_ptr<BoundExpression> expression = expressionBinder.GetExpression();
    if (acceptFunctionGroup && expression->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExcpression)
    { 
        return expression;
    }
    if (acceptMemberExpression && expression->GetBoundNodeType() == BoundNodeType::boundMemberExpression)
    {
        return expression;
    }
    if (!acceptIncomplete)
    {
        if (!expression->IsComplete())
        {
            throw Exception("incomplete expression", node->GetSpan());
        }
    }
    if (lvalue && !expression->IsLvalueExpression())
    {
        throw Exception("not an lvalue expression", node->GetSpan());
    }
    return expression;
}

} } // namespace cmajor::binder
