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
#include <cmajor/symbols/InterfaceTypeSymbol.hpp>
#include <cmajor/symbols/ConstantSymbol.hpp>
#include <cmajor/symbols/EnumSymbol.hpp>
#include <cmajor/symbols/TypedefSymbol.hpp>
#include <cmajor/symbols/TemplateSymbol.hpp>
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
    void Visit(TemplateIdNode& templateIdNode) override;
    void Visit(ParameterNode& parameterNode) override;
    void Visit(DotNode& dotNode) override;
    void Visit(ArrowNode& arrowNode) override;
    void BindArrow(Node& node, const std::u32string& name);
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
    void BindUnaryOp(BoundExpression* operand, Node& node, const std::u32string& groupName);
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
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, operand->GetType()->BaseType()->ClassOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    std::unique_ptr<BoundFunctionCall> operatorFunCall = ResolveOverload(groupName, containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, node.GetSpan());
    CheckAccess(boundFunction->GetFunctionSymbol(), operatorFunCall->GetFunctionSymbol());
    LocalVariableSymbol* temporary = nullptr;
    if (operatorFunCall->GetFunctionSymbol()->ReturnsClassInterfaceOrClassDelegateByValue())
    {
        TypeSymbol* type = operatorFunCall->GetFunctionSymbol()->ReturnType();
        temporary = boundFunction->GetFunctionSymbol()->CreateTemporary(type, node.GetSpan());
        operatorFunCall->AddArgument(std::unique_ptr<BoundExpression>(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)),
            type->AddPointer(node.GetSpan()))));
    }
    expression.reset(operatorFunCall.release());
    if (temporary)
    {
        expression.reset(new BoundConstructAndReturnTemporaryExpression(std::move(expression), std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary))));
        expression->SetFlag(BoundExpressionFlags::bindToRvalueReference);
    }
}

void ExpressionBinder::BindUnaryOp(UnaryNode& unaryNode, const std::u32string& groupName)
{
    unaryNode.Subject()->Accept(*this);
    if (expression->GetType()->IsReferenceType() && expression->GetType()->PlainType(unaryNode.GetSpan())->IsClassTypeSymbol())
    {
        TypeSymbol* type = expression->GetType()->RemoveReference(unaryNode.GetSpan())->AddPointer(unaryNode.GetSpan());
        expression.reset(new BoundReferenceToPointerExpression(std::move(expression), type));
    }
    else if (expression->GetType()->IsClassTypeSymbol())
    {
        TypeSymbol* type = expression->GetType()->AddPointer(unaryNode.GetSpan());
        expression.reset(new BoundAddressOfExpression(std::move(expression), type));
    }
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
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, left->GetType()->BaseType()->ClassOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, right->GetType()->BaseType()->ClassOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    std::vector<TypeSymbol*> templateArgumentTypes;
    std::unique_ptr<Exception> exception;
    std::unique_ptr<BoundFunctionCall> operatorFunCall = ResolveOverload(groupName, containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, node.GetSpan(),
        OverloadResolutionFlags::dontThrow, templateArgumentTypes, exception);
    if (!operatorFunCall)
    {
        if (arguments[0]->GetType()->PlainType(node.GetSpan())->IsClassTypeSymbol())
        {
            if (arguments[0]->GetType()->IsReferenceType())
            {
                TypeSymbol* type = arguments[0]->GetType()->RemoveReference(node.GetSpan())->AddPointer(node.GetSpan());
                arguments[0].reset(new BoundReferenceToPointerExpression(std::move(arguments[0]), type));
            }
            else
            {
                TypeSymbol* type = arguments[0]->GetType()->PlainType(node.GetSpan())->AddPointer(node.GetSpan());
                arguments[0].reset(new BoundAddressOfExpression(std::move(arguments[0]), type));
            }
            operatorFunCall = std::move(ResolveOverload(groupName, containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, node.GetSpan()));
        }
        else
        {
            throw *exception;
        }
    }
    CheckAccess(boundFunction->GetFunctionSymbol(), operatorFunCall->GetFunctionSymbol());
    LocalVariableSymbol* temporary = nullptr;
    if (operatorFunCall->GetFunctionSymbol()->ReturnsClassInterfaceOrClassDelegateByValue())
    {
        TypeSymbol* type = operatorFunCall->GetFunctionSymbol()->ReturnType();
        temporary = boundFunction->GetFunctionSymbol()->CreateTemporary(type, node.GetSpan());
        operatorFunCall->AddArgument(std::unique_ptr<BoundExpression>(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)),
            type->AddPointer(node.GetSpan()))));
    }
    expression.reset(operatorFunCall.release());
    if (temporary)
    {
        expression.reset(new BoundConstructAndReturnTemporaryExpression(std::move(expression), std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary))));
        expression->SetFlag(BoundExpressionFlags::bindToRvalueReference);
    }
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
                boundFunctionGroupExpression->SetClassPtr(std::unique_ptr<BoundExpression>(new BoundParameter(thisParam)));
            }
            expression.reset(boundFunctionGroupExpression);
            break;
        }
        case SymbolType::classTypeSymbol: case SymbolType::classTemplateSpecializationSymbol:
        {
            ClassTypeSymbol* classTypeSymbol = static_cast<ClassTypeSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), classTypeSymbol);
            expression.reset(new BoundTypeExpression(span, classTypeSymbol));
            break;
        }
        case SymbolType::classGroupTypeSymbol: 
        {
            ClassGroupTypeSymbol* classGroupTypeSymbol = static_cast<ClassGroupTypeSymbol*>(symbol);
            expression.reset(new BoundTypeExpression(span, classGroupTypeSymbol));
            break;
        }
        case SymbolType::interfaceTypeSymbol:
        {
            InterfaceTypeSymbol* interfaceTypeSymbol = static_cast<InterfaceTypeSymbol*>(symbol); 
            expression.reset(new BoundTypeExpression(span, interfaceTypeSymbol));
            break;
        }
        case SymbolType::delegateTypeSymbol:
        {
            DelegateTypeSymbol* delegateTypeSymbol = static_cast<DelegateTypeSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), delegateTypeSymbol);
            expression.reset(new BoundTypeExpression(span, delegateTypeSymbol));
            break;
        }
        case SymbolType::classDelegateTypeSymbol:
        {
            ClassDelegateTypeSymbol* classDelegateTypeSymbol = static_cast<ClassDelegateTypeSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), classDelegateTypeSymbol);
            expression.reset(new BoundTypeExpression(span, classDelegateTypeSymbol));
            break;
        }
        case SymbolType::typedefSymbol:
        {
            TypedefSymbol* typedefSymbol = static_cast<TypedefSymbol*>(symbol);
            CheckAccess(boundFunction->GetFunctionSymbol(), typedefSymbol);
            expression.reset(new BoundTypeExpression(span, typedefSymbol->GetType()));
            break;
        }
        case SymbolType::boundTemplateParameterSymbol:
        {
            BoundTemplateParameterSymbol* boundTemplateParameterSymbol = static_cast<BoundTemplateParameterSymbol*>(symbol);
            expression.reset(new BoundTypeExpression(span, boundTemplateParameterSymbol->GetType()));
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
            FunctionSymbol* currentFuctionSymbol = boundFunction->GetFunctionSymbol();
            CheckAccess(currentFuctionSymbol, memberVariableSymbol);
            BoundMemberVariable* bmv = new BoundMemberVariable(memberVariableSymbol);
            bool accessFromOwnScope = false;
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
                ParameterSymbol* thisParam = currentFuctionSymbol->GetThisParam();
                if (accessFromOwnScope && !currentFuctionSymbol->IsStatic())
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
                    ArgumentMatch argumentMatch;
                    FunctionSymbol* conversionFun = boundCompileUnit.GetConversion(thisPointerType, containingClassPointerType, containerScope, boundFunction, span, argumentMatch);
                    if (conversionFun)
                    {
                        bmv->SetClassPtr(std::unique_ptr<BoundExpression>(new BoundConversion(std::unique_ptr<BoundExpression>(new BoundParameter(thisParam)), conversionFun)));
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
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new StringValue(stringLiteralNode.GetSpan(), boundCompileUnit.Install(stringLiteralNode.Value()))),
        symbolTable.GetTypeByName(U"char")->AddConst(stringLiteralNode.GetSpan())->AddPointer(stringLiteralNode.GetSpan())));
}

void ExpressionBinder::Visit(WStringLiteralNode& wstringLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new WStringValue(wstringLiteralNode.GetSpan(), boundCompileUnit.Install(wstringLiteralNode.Value()))),
        symbolTable.GetTypeByName(U"wchar")->AddConst(wstringLiteralNode.GetSpan())->AddPointer(wstringLiteralNode.GetSpan())));
}

void ExpressionBinder::Visit(UStringLiteralNode& ustringLiteralNode)
{
    expression.reset(new BoundLiteral(std::unique_ptr<Value>(new UStringValue(ustringLiteralNode.GetSpan(), boundCompileUnit.Install(ustringLiteralNode.Value()))),
        symbolTable.GetTypeByName(U"uchar")->AddConst(ustringLiteralNode.GetSpan())->AddPointer(ustringLiteralNode.GetSpan())));
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

void ExpressionBinder::Visit(TemplateIdNode& templateIdNode)
{
    int arity = templateIdNode.TemplateArguments().Count();
    templateIdNode.Primary()->Accept(*this);
    if (expression->GetBoundNodeType() == BoundNodeType::boundTypeExpression)
    {
        TypeSymbol* typeSymbol = expression->GetType();
        if (typeSymbol->GetSymbolType() == SymbolType::classGroupTypeSymbol)
        {
            ClassGroupTypeSymbol* classGroup = static_cast<ClassGroupTypeSymbol*>(typeSymbol);
            typeSymbol = classGroup->GetClass(arity);
            expression.reset(new BoundTypeExpression(span, typeSymbol));
        }
    }
    std::vector<TypeSymbol*> templateArgumentTypes;
    int n = arity;
    for (int i = 0; i < n; ++i)
    {
        Node* templateArgumentNode = templateIdNode.TemplateArguments()[i];
        TypeSymbol* type = ResolveType(templateArgumentNode, boundCompileUnit, containerScope);
        templateArgumentTypes.push_back(type);
    }
    if (expression->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExpression)
    {
        BoundFunctionGroupExpression* bfge = static_cast<BoundFunctionGroupExpression*>(expression.get());
        bfge->SetTemplateArgumentTypes(templateArgumentTypes);
    }
    else if (expression->GetBoundNodeType() == BoundNodeType::boundMemberExpression)
    {
        BoundMemberExpression* bme = static_cast<BoundMemberExpression*>(expression.get());
        if (bme->Member()->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExpression)
        {
            BoundFunctionGroupExpression* bfge = static_cast<BoundFunctionGroupExpression*>(bme->Member());
            bfge->SetTemplateArgumentTypes(templateArgumentTypes);
        }
        else if (bme->Member()->GetBoundNodeType() == BoundNodeType::boundTypeExpression)
        {
            TypeSymbol* typeSymbol = bme->Member()->GetType();
            if (typeSymbol->IsClassTypeSymbol())
            {
                ClassTypeSymbol* classTypeSymbol = static_cast<ClassTypeSymbol*>(typeSymbol);
                if (classTypeSymbol->IsClassTemplate())
                {
                    int m = classTypeSymbol->TemplateParameters().size();
                    if (n < m)
                    {
                        boundCompileUnit.GetClassTemplateRepository().ResolveDefaultTemplateArguments(templateArgumentTypes, classTypeSymbol, containerScope, templateIdNode.GetSpan());
                    }
                    ClassTemplateSpecializationSymbol* classTemplateSpecialization = symbolTable.MakeClassTemplateSpecialization(classTypeSymbol, templateArgumentTypes, templateIdNode.GetSpan());
                    if (!classTemplateSpecialization->IsBound())
                    {
                        boundCompileUnit.GetClassTemplateRepository().BindClassTemplateSpecialization(classTemplateSpecialization, containerScope, templateIdNode.GetSpan());
                    }
                    expression.reset(new BoundTypeExpression(span, classTemplateSpecialization));
                }
            }
        }
        else
        {
            throw Exception("function group or class group expected", templateIdNode.GetSpan());
        }
    }
    else if (expression->GetBoundNodeType() == BoundNodeType::boundTypeExpression)
    {
        TypeSymbol* typeSymbol = expression->GetType();
        if (typeSymbol->IsClassTypeSymbol())
        {
            ClassTypeSymbol* classTypeSymbol = static_cast<ClassTypeSymbol*>(typeSymbol);
            if (classTypeSymbol->IsClassTemplate())
            {
                int m = classTypeSymbol->TemplateParameters().size();
                if (n < m)
                {
                    boundCompileUnit.GetClassTemplateRepository().ResolveDefaultTemplateArguments(templateArgumentTypes, classTypeSymbol, containerScope, templateIdNode.GetSpan());
                }
                ClassTemplateSpecializationSymbol* classTemplateSpecialization = symbolTable.MakeClassTemplateSpecialization(classTypeSymbol, templateArgumentTypes, templateIdNode.GetSpan());
                if (!classTemplateSpecialization->IsBound())
                {
                    boundCompileUnit.GetClassTemplateRepository().BindClassTemplateSpecialization(classTemplateSpecialization, containerScope, templateIdNode.GetSpan());
                }
                expression.reset(new BoundTypeExpression(span, classTemplateSpecialization));
            }
        }
    }
    else
    {
        throw Exception("function group or class group expected", templateIdNode.GetSpan());
    }
}

void ExpressionBinder::Visit(ParameterNode& parameterNode)
{
    if (!parameterNode.Id())
    {
        throw Exception("parameter not named", parameterNode.GetSpan());
    }
    std::u32string name = parameterNode.Id()->Str();
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
        if (symbol->GetSymbolType() == SymbolType::parameterSymbol)
        {
            ParameterSymbol* parameterSymbol = static_cast<ParameterSymbol*>(symbol);
            expression.reset(new BoundParameter(parameterSymbol));
        }
        else
        {
            throw Exception("symbol '" + ToUtf8(name) + "' does not denote a parameter", parameterNode.GetSpan());
        }
    }
    else
    {
        throw Exception("parameter symbol '" + ToUtf8(name) + "' not found", parameterNode.GetSpan());
    }
}

void ExpressionBinder::Visit(DotNode& dotNode)
{
    ContainerScope* prevContainerScope = containerScope;
    expression = std::move(BindExpression(dotNode.Subject(), boundCompileUnit, boundFunction, containerScope, statementBinder, false, true, true, true, false));
    if (expression->GetBoundNodeType() == BoundNodeType::boundTypeExpression)
    {
        TypeSymbol* typeSymbol = expression->GetType();
        if (typeSymbol->GetSymbolType() == SymbolType::classGroupTypeSymbol)
        {
            ClassGroupTypeSymbol* classGroupTypeSymbol = static_cast<ClassGroupTypeSymbol*>(typeSymbol);
            typeSymbol = classGroupTypeSymbol->GetClass(0);
            if (!typeSymbol)
            {
                throw Exception("ordinary class not found from class group '" + ToUtf8(classGroupTypeSymbol->FullName()) + "'", span, classGroupTypeSymbol->GetSpan());
            }
            else
            {
                expression.reset(new BoundTypeExpression(span, typeSymbol));
            }
        }
    }
    if (expression->GetBoundNodeType() == BoundNodeType::boundNamespaceExpression)
    {
        BoundNamespaceExpression* bns = static_cast<BoundNamespaceExpression*>(expression.get());
        containerScope = bns->Ns()->GetContainerScope();
        std::u32string name = dotNode.MemberId()->Str();
        Symbol* symbol = containerScope->Lookup(name, ScopeLookup::this_);
        if (symbol)
        {
            BindSymbol(symbol);
            if (expression->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExpression)
            {
                BoundFunctionGroupExpression* bfe = static_cast<BoundFunctionGroupExpression*>(expression.get());
                bfe->SetScopeQualified();
                bfe->SetQualifiedScope(containerScope);
            }
        }
        else
        {
            throw Exception("symbol '" + ToUtf8(name) + "' not found from namespace '" + ToUtf8(bns->Ns()->FullName()) + "'", dotNode.MemberId()->GetSpan());
        }
    }
    else
    {
        TypeSymbol* type = expression->GetType()->PlainType(dotNode.GetSpan());
        if (type->IsClassTypeSymbol())
        {
            ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(type->BaseType());
            ContainerScope* scope = classType->GetContainerScope();
            std::u32string name = dotNode.MemberId()->Str();
            Symbol* symbol = scope->Lookup(name, ScopeLookup::this_and_base);
            if (symbol)
            {
                std::unique_ptr<BoundExpression> classPtr;
                if (expression->GetType()->IsClassTypeSymbol())
                {
                    TypeSymbol* type = expression->GetType()->AddPointer(dotNode.GetSpan());
                    classPtr.reset(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(expression.release()), type));
                }
                else if (expression->GetType()->IsReferenceType())
                {
                    TypeSymbol* type = expression->GetType()->RemoveReference(dotNode.GetSpan())->AddPointer(dotNode.GetSpan());
                    classPtr.reset(new BoundReferenceToPointerExpression(std::unique_ptr<BoundExpression>(expression.release()), type));
                }
                else
                {
                    classPtr.reset(expression.release());
                }
                BindSymbol(symbol);
                if (expression->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExpression)
                {
                    BoundFunctionGroupExpression* bfg = static_cast<BoundFunctionGroupExpression*>(expression.get());
                    if (!classPtr->GetFlag(BoundExpressionFlags::argIsExplicitThisOrBasePtr))
                    {
                        Symbol* parent = symbol->Parent();
                        Assert(parent->GetSymbolType() == SymbolType::classTypeSymbol || parent->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol, "class type expected");
                        ClassTypeSymbol* owner = static_cast<ClassTypeSymbol*>(parent);
                        if (classType->HasBaseClass(owner))
                        {
                            if (classPtr->GetType()->IsConstType())
                            {
                                ArgumentMatch argumentMatch;
                                classPtr.reset(new BoundConversion(std::unique_ptr<BoundExpression>(classPtr.release()),
                                    boundCompileUnit.GetConversion(classType->AddConst(span)->AddPointer(span), owner->AddConst(span)->AddPointer(span), containerScope, boundFunction, dotNode.GetSpan(), argumentMatch)));
                            }
                            else
                            {
                                ArgumentMatch argumentMatch;
                                classPtr.reset(new BoundConversion(std::unique_ptr<BoundExpression>(classPtr.release()),
                                    boundCompileUnit.GetConversion(classType->AddPointer(span), owner->AddPointer(span), containerScope, boundFunction, dotNode.GetSpan(), argumentMatch)));
                            }
                        }
                    }
                    if (classPtr->GetBoundNodeType() == BoundNodeType::boundTypeExpression)
                    {
                        BoundTypeExpression* bte = static_cast<BoundTypeExpression*>(classPtr.get());
                        bfg->SetScopeQualified();
                        bfg->SetQualifiedScope(bte->GetType()->GetContainerScope());
                    }
                    BoundMemberExpression* bme = new BoundMemberExpression(dotNode.GetSpan(), std::unique_ptr<BoundExpression>(classPtr.release()), std::move(expression));
                    expression.reset(bme);
                }
                else if (expression->GetBoundNodeType() == BoundNodeType::boundMemberVariable)
                {
                    BoundMemberVariable* bmv = static_cast<BoundMemberVariable*>(expression.get());
                    if (!bmv->GetMemberVariableSymbol()->IsStatic())
                    {
                        Symbol* parent = symbol->Parent();
                        Assert(parent->GetSymbolType() == SymbolType::classTypeSymbol || parent->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol, "class type expected");
                        ClassTypeSymbol* owner = static_cast<ClassTypeSymbol*>(parent);
                        if (classType->HasBaseClass(owner))
                        {
                            if (classPtr->GetType()->IsConstType())
                            {
                                ArgumentMatch argumentMatch;
                                classPtr.reset(new BoundConversion(std::unique_ptr<BoundExpression>(classPtr.release()),
                                    boundCompileUnit.GetConversion(classType->AddConst(span)->AddPointer(span), owner->AddConst(span)->AddPointer(span), containerScope, boundFunction, dotNode.GetSpan(), argumentMatch)));
                            }
                            else
                            {
                                ArgumentMatch argumentMatch;
                                classPtr.reset(new BoundConversion(std::unique_ptr<BoundExpression>(classPtr.release()),
                                    boundCompileUnit.GetConversion(classType->AddPointer(span), owner->AddPointer(span), containerScope, boundFunction, dotNode.GetSpan(), argumentMatch)));
                            }
                        }
                        bmv->SetClassPtr(std::unique_ptr<BoundExpression>(classPtr.release()));
                    }
                }
                else if (expression->GetBoundNodeType() != BoundNodeType::boundTypeExpression)
                {
                    throw Exception("symbol '" + ToUtf8(name) + "' does not denote a function group, member variable, or type", dotNode.MemberId()->GetSpan());
                }
            }
            else
            {
                throw Exception("symbol '" + ToUtf8(name) + "' not found from class '" + ToUtf8(classType->FullName()) + "'", dotNode.MemberId()->GetSpan());
            }
        }
        else if (type->GetSymbolType() == SymbolType::interfaceTypeSymbol)
        {
            InterfaceTypeSymbol* interfaceType = static_cast<InterfaceTypeSymbol*>(type->BaseType());
            ContainerScope* scope = interfaceType->GetContainerScope();
            std::u32string name = dotNode.MemberId()->Str();
            Symbol* symbol = scope->Lookup(name, ScopeLookup::this_);
            if (symbol)
            {
                std::unique_ptr<BoundExpression> interfacePtr;
                interfacePtr.reset(expression.release());
                BindSymbol(symbol);
                if (expression->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExpression)
                {
                    BoundFunctionGroupExpression* bfg = static_cast<BoundFunctionGroupExpression*>(expression.get());
                    BoundMemberExpression* bme = new BoundMemberExpression(dotNode.GetSpan(), std::unique_ptr<BoundExpression>(interfacePtr.release()), std::move(expression));
                    expression.reset(bme);
                }
                else 
                {
                    throw Exception("symbol '" + ToUtf8(name) + "' does not denote a function group", dotNode.MemberId()->GetSpan());
                }
            }
            else
            {
                throw Exception("symbol '" + ToUtf8(name) + "' not found from interface '" + ToUtf8(interfaceType->FullName()) + "'", dotNode.MemberId()->GetSpan());
            }
        }
        else if (type->GetSymbolType() == SymbolType::enumTypeSymbol)
        {
            EnumTypeSymbol* enumType = static_cast<EnumTypeSymbol*>(type);
            ContainerScope* scope = enumType->GetContainerScope();
            std::u32string name = dotNode.MemberId()->Str();
            Symbol* symbol = scope->Lookup(name);
            if (symbol)
            {
                BindSymbol(symbol);
            }
            else
            {
                throw Exception("symbol '" + ToUtf8(name) + "' not found from enumerated type '" + ToUtf8(enumType->FullName()) + "'", dotNode.MemberId()->GetSpan());
            }
        }
        else
        {
            throw Exception("expression must denote a namespace, class type, interface type, or an enumerated type type object", dotNode.GetSpan());
        }
    }
    containerScope = prevContainerScope;
}

void ExpressionBinder::BindArrow(Node& node, const std::u32string& name)
{
    if (expression->GetType()->IsPointerType())
    {
        std::unique_ptr<BoundExpression> classPtr(std::move(expression));
        if (classPtr->GetType()->BaseType()->IsClassTypeSymbol())
        {
            ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(classPtr->GetType()->BaseType());
            ContainerScope* scope = classType->GetContainerScope();
            Symbol* symbol = scope->Lookup(name, ScopeLookup::this_and_base);
            if (symbol)
            {
                BindSymbol(symbol);
                if (expression->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExpression)
                {
                    BoundFunctionGroupExpression* bfg = static_cast<BoundFunctionGroupExpression*>(expression.get());
                    if (!classPtr->GetFlag(BoundExpressionFlags::argIsExplicitThisOrBasePtr))
                    {
                        Symbol* parent = symbol->Parent();
                        Assert(parent->GetSymbolType() == SymbolType::classTypeSymbol || parent->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol, "class type expected");
                        ClassTypeSymbol* owner = static_cast<ClassTypeSymbol*>(parent);
                        if (classType->HasBaseClass(owner))
                        {
                            if (classPtr->GetType()->IsConstType())
                            {
                                ArgumentMatch argumentMatch;
                                classPtr.reset(new BoundConversion(std::unique_ptr<BoundExpression>(classPtr.release()),
                                    boundCompileUnit.GetConversion(classType->AddConst(span)->AddPointer(span), owner->AddConst(span)->AddPointer(span), containerScope, boundFunction, node.GetSpan(), argumentMatch)));
                            }
                            else
                            {
                                ArgumentMatch argumentMatch;
                                classPtr.reset(new BoundConversion(std::unique_ptr<BoundExpression>(classPtr.release()),
                                    boundCompileUnit.GetConversion(classType->AddPointer(span), owner->AddPointer(span), containerScope, boundFunction, node.GetSpan(), argumentMatch)));
                            }
                        }
                    }
                    BoundMemberExpression* bme = new BoundMemberExpression(node.GetSpan(), std::unique_ptr<BoundExpression>(classPtr.release()), std::move(expression));
                    expression.reset(bme);
                }
                else if (expression->GetBoundNodeType() == BoundNodeType::boundMemberVariable)
                {
                    BoundMemberVariable* bmv = static_cast<BoundMemberVariable*>(expression.get());
                    if (!bmv->GetMemberVariableSymbol()->IsStatic())
                    {
                        Symbol* parent = symbol->Parent();
                        Assert(parent->GetSymbolType() == SymbolType::classTypeSymbol || parent->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol, "class type expected");
                        ClassTypeSymbol* owner = static_cast<ClassTypeSymbol*>(parent);
                        if (classType->HasBaseClass(owner))
                        {
                            if (classPtr->GetType()->IsConstType())
                            {
                                ArgumentMatch argumentMatch;
                                classPtr.reset(new BoundConversion(std::unique_ptr<BoundExpression>(classPtr.release()),
                                    boundCompileUnit.GetConversion(classType->AddConst(span)->AddPointer(span), owner->AddConst(span)->AddPointer(span), containerScope, boundFunction, node.GetSpan(), argumentMatch)));
                            }
                            else
                            {
                                ArgumentMatch argumentMatch;
                                classPtr.reset(new BoundConversion(std::unique_ptr<BoundExpression>(classPtr.release()),
                                    boundCompileUnit.GetConversion(classType->AddPointer(span), owner->AddPointer(span), containerScope, boundFunction, node.GetSpan(), argumentMatch)));
                            }
                        }
                        bmv->SetClassPtr(std::unique_ptr<BoundExpression>(classPtr.release()));
                    }
                    else
                    {
                        throw Exception("member variable '" + ToUtf8(bmv->GetMemberVariableSymbol()->FullName()) + +"' is static", node.GetSpan());
                    }
                }
                else
                {
                    throw Exception("symbol '" + ToUtf8(name) + "' does not denote a function group or a member variable", node.GetSpan());
                }
            }
            else
            {
                throw Exception("symbol '" + ToUtf8(name) + "' not found from class '" + ToUtf8(classType->FullName()) + "'", node.GetSpan());
            }
        }
        else
        {
            throw Exception("type of arrow expression subject must be pointer to class type", node.GetSpan());
        }
    }
    else if (expression->GetType()->IsClassTypeSymbol())
    {
        TypeSymbol* type = expression->GetType();
        TypeSymbol* pointerType = type->AddPointer(node.GetSpan());
        LocalVariableSymbol* temporary = boundFunction->GetFunctionSymbol()->CreateTemporary(type, node.GetSpan());
        Assert(expression->GetBoundNodeType() == BoundNodeType::boundFunctionCall, "function call expected");
        BoundFunctionCall* boundFunctionCall = static_cast<BoundFunctionCall*>(expression.get());
        boundFunctionCall->AddArgument(std::unique_ptr<BoundExpression>(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)), pointerType)));
        expression.reset(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(
            new BoundConstructAndReturnTemporaryExpression(std::move(expression), std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)))), pointerType));
        BindUnaryOp(expression.release(), node, U"operator->");
        BindArrow(node, name);
    }
    else
    {
        throw Exception("arrow operator member function must return a class type object or a pointer to a class type object", node.GetSpan());
    }
}

void ExpressionBinder::Visit(ArrowNode& arrowNode) 
{
    arrowNode.Subject()->Accept(*this);
    if (expression->GetType()->IsReferenceType() && expression->GetType()->PlainType(arrowNode.GetSpan())->IsClassTypeSymbol())
    {
        TypeSymbol* type = expression->GetType()->RemoveReference(arrowNode.GetSpan())->AddPointer(arrowNode.GetSpan());
        expression.reset(new BoundReferenceToPointerExpression(std::move(expression), type));
    }
    else if (expression->GetType()->IsReferenceType())
    {
        TypeSymbol* type = expression->GetType()->RemoveReference(arrowNode.GetSpan())->AddPointer(arrowNode.GetSpan());
        expression.reset(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(new BoundDereferenceExpression(std::move(expression), type)), type->AddPointer(arrowNode.GetSpan())));
    }
    else
    {
        TypeSymbol* type = expression->GetType()->AddPointer(arrowNode.GetSpan());
        expression.reset(new BoundAddressOfExpression(std::move(expression), type));
    }
    BindUnaryOp(expression.release(), arrowNode, U"operator->");
    BindArrow(arrowNode, arrowNode.MemberId()->Str());
}

void ExpressionBinder::Visit(DisjunctionNode& disjunctionNode) 
{
    std::unique_ptr<BoundExpression> left = BindExpression(disjunctionNode.Left(), boundCompileUnit, boundFunction, containerScope, statementBinder);
    std::unique_ptr<BoundExpression> right = BindExpression(disjunctionNode.Right(), boundCompileUnit, boundFunction, containerScope, statementBinder);
    BoundDisjunction* boundDisjunction = new BoundDisjunction(disjunctionNode.GetSpan(), std::move(left), std::move(right), symbolTable.GetTypeByName(U"bool"));
    LocalVariableSymbol* temporary = boundFunction->GetFunctionSymbol()->CreateTemporary(symbolTable.GetTypeByName(U"bool"), disjunctionNode.GetSpan());
    boundDisjunction->SetTemporary(new BoundLocalVariable(temporary));
    expression.reset(boundDisjunction);
}

void ExpressionBinder::Visit(ConjunctionNode& conjunctionNode) 
{
    std::unique_ptr<BoundExpression> left = BindExpression(conjunctionNode.Left(), boundCompileUnit, boundFunction, containerScope, statementBinder);
    std::unique_ptr<BoundExpression> right = BindExpression(conjunctionNode.Right(), boundCompileUnit, boundFunction, containerScope, statementBinder);
    BoundConjunction* boundConjunction = new BoundConjunction(conjunctionNode.GetSpan(), std::move(left), std::move(right), symbolTable.GetTypeByName(U"bool"));
    LocalVariableSymbol* temporary = boundFunction->GetFunctionSymbol()->CreateTemporary(symbolTable.GetTypeByName(U"bool"), conjunctionNode.GetSpan());
    boundConjunction->SetTemporary(new BoundLocalVariable(temporary));
    expression.reset(boundConjunction);
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
            if (expression->GetType()->IsReferenceType())
            {
                TypeSymbol* type = expression->GetType()->RemoveReference(node.GetSpan())->AddPointer(node.GetSpan());
                expression.reset(new BoundReferenceToPointerExpression(std::move(expression), type));
            }
            else if (expression->GetType()->IsClassTypeSymbol())
            {
                TypeSymbol* type = expression->GetType()->AddPointer(node.GetSpan());
                expression.reset(new BoundAddressOfExpression(std::move(expression), type));
            }
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
        if (expression->GetType()->IsReferenceType())
        {
            TypeSymbol* type = expression->GetType()->RemoveReference(addrOfNode.GetSpan())->AddPointer(addrOfNode.GetSpan());
            expression.reset(new BoundReferenceToPointerExpression(std::unique_ptr<BoundExpression>(expression.release()), type));
        }
        else
        {
            TypeSymbol* type = expression->GetType()->AddPointer(addrOfNode.GetSpan());
            expression.reset(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(expression.release()), type));
        }
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
    TypeSymbol* rightType = ResolveType(isNode.TargetTypeExpr(), boundCompileUnit, containerScope);
    if (rightType->IsPointerType())
    {
        TypeSymbol* rightBaseType = rightType->RemovePointer(span);
        if (rightBaseType->IsClassTypeSymbol())
        {
            ClassTypeSymbol* rightClassType = static_cast<ClassTypeSymbol*>(rightBaseType);
            if (rightClassType->IsPolymorphic())
            {
                std::unique_ptr<BoundExpression> boundExpr = BindExpression(isNode.Expr(), boundCompileUnit, boundFunction, containerScope, statementBinder, false, false, false, false, false);
                TypeSymbol* leftType = boundExpr->GetType();
                if (leftType->IsPointerType())
                {
                    TypeSymbol* leftBaseType = leftType->RemovePointer(span);
                    if (leftBaseType->IsClassTypeSymbol())
                    {
                        ClassTypeSymbol* leftClassType = static_cast<ClassTypeSymbol*>(leftBaseType);
                        if (leftClassType->IsPolymorphic())
                        {
                            expression.reset(new BoundIsExpression(std::move(boundExpr), rightClassType, symbolTable.GetTypeByName(U"bool")));
                        }
                        else
                        {
                            throw Exception("left type in 'is' expression must be pointer to polymorphic class type", isNode.Expr()->GetSpan());
                        }
                    }
                    else
                    {
                        throw Exception("left type in 'is' expression must be pointer to polymorphic class type", isNode.Expr()->GetSpan());
                    }
                }
                else
                {
                    throw Exception("left type in 'is' expression must be pointer to polymorphic class type", isNode.Expr()->GetSpan());
                }
            }
            else
            {
                throw Exception("right type in 'is' expression must be pointer to polymorphic class type", isNode.TargetTypeExpr()->GetSpan());
            }
        }
        else
        {
            throw Exception("right type in 'is' expression must be be pointer to polymorphic class type", isNode.TargetTypeExpr()->GetSpan());
        }
    }
    else
    {
        throw Exception("right type in 'is' expression must be be pointer to polymorphic class type", isNode.TargetTypeExpr()->GetSpan());
    }
}

void ExpressionBinder::Visit(AsNode& asNode) 
{
    TypeSymbol* rightType = ResolveType(asNode.TargetTypeExpr(), boundCompileUnit, containerScope);
    if (rightType->IsPointerType())
    {
        TypeSymbol* rightBaseType = rightType->RemovePointer(span);
        if (rightBaseType->IsClassTypeSymbol())
        {
            ClassTypeSymbol* rightClassType = static_cast<ClassTypeSymbol*>(rightBaseType);
            if (rightClassType->IsPolymorphic())
            {
                std::unique_ptr<BoundExpression> boundExpr = BindExpression(asNode.Expr(), boundCompileUnit, boundFunction, containerScope, statementBinder, false, false, false, false, false);
                TypeSymbol* leftType = boundExpr->GetType();
                if (leftType->IsPointerType())
                {
                    TypeSymbol* leftBaseType = leftType->RemovePointer(span);
                    if (leftBaseType->IsClassTypeSymbol())
                    {
                        ClassTypeSymbol* leftClassType = static_cast<ClassTypeSymbol*>(leftBaseType);
                        if (leftClassType->IsPolymorphic())
                        {
                            expression.reset(new BoundAsExpression(std::move(boundExpr), rightClassType,
                                std::unique_ptr<BoundLocalVariable>(new BoundLocalVariable(boundFunction->GetFunctionSymbol()->CreateTemporary(
                                    rightClassType->AddPointer(asNode.GetSpan()), asNode.GetSpan())))));
                        }
                        else
                        {
                            throw Exception("left type in 'as' expression must be pointer to polymorphic class type", asNode.Expr()->GetSpan());
                        }
                    }
                    else
                    {
                        throw Exception("left type in 'as' expression must be pointer to polymorphic class type", asNode.Expr()->GetSpan());
                    }
                }
                else
                {
                    throw Exception("left type in 'as' expression must be pointer to polymorphic class type", asNode.Expr()->GetSpan());
                }
            }
            else
            {
                throw Exception("right type in 'as' expression must be pointer to polymorphic class type", asNode.TargetTypeExpr()->GetSpan());
            }
        }
        else
        {
            throw Exception("right type in 'as' expression must be be pointer to polymorphic class type", asNode.TargetTypeExpr()->GetSpan());
        }
    }
    else
    {
        throw Exception("right type in 'as' expression must be be pointer to polymorphic class type", asNode.TargetTypeExpr()->GetSpan());
    }
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
        BindBinaryOp(subject.release(), index.release(), indexingNode, U"operator[]");
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
    std::vector<TypeSymbol*> templateArgumentTypes;
    std::u32string groupName;
    bool scopeQualified = false;
    LocalVariableSymbol* temporary = nullptr;
    if (expression->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExpression)
    {
        BoundFunctionGroupExpression* bfge = static_cast<BoundFunctionGroupExpression*>(expression.get());
        functionGroupSymbol = bfge->FunctionGroup();
        templateArgumentTypes = bfge->TemplateArgumentTypes();
        groupName = functionGroupSymbol->Name();
        if (bfge->IsScopeQualified())
        {
            functionScopeLookups.clear();
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, bfge->QualifiedScope()));
            scopeQualified = true;
        }
    }
    else if (expression->GetBoundNodeType() == BoundNodeType::boundMemberExpression)
    {
        BoundMemberExpression* bme = static_cast<BoundMemberExpression*>(expression.get());
        if (bme->Member()->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExpression)
        {
            BoundFunctionGroupExpression* bfge = static_cast<BoundFunctionGroupExpression*>(bme->Member());
            functionGroupSymbol = bfge->FunctionGroup();
            templateArgumentTypes = bfge->TemplateArgumentTypes();
            groupName = functionGroupSymbol->Name();
            if (bfge->IsScopeQualified())
            {
                functionScopeLookups.clear();
                functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, bfge->QualifiedScope()));
                scopeQualified = true;
            }
            if (!scopeQualified)
            {
                functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base, bme->ClassPtr()->GetType()->BaseType()->ClassInterfaceOrNsScope()));
            }
            arguments.push_back(std::unique_ptr<BoundExpression>(bme->ReleaseClassPtr()));
            if (arguments.front()->GetType()->PlainType(span)->GetSymbolType() == SymbolType::interfaceTypeSymbol)
            {
                if (arguments.front()->GetType()->IsReferenceType())
                {
                    TypeSymbol* type = arguments.front()->GetType()->RemoveReference(span)->AddPointer(span);
                    arguments[0].reset(new BoundReferenceToPointerExpression(std::move(arguments[0]), type));
                }
                else
                {
                    TypeSymbol* type = arguments.front()->GetType()->AddPointer(span);
                    arguments[0].reset(new BoundAddressOfExpression(std::move(arguments[0]), type));
                }
            }
        }
        else
        {
            throw Exception("invoke cannot be applied to this type of expression", invokeNode.Subject()->GetSpan());
        }
    }
    else if (expression->GetBoundNodeType() == BoundNodeType::boundTypeExpression)
    {
        TypeSymbol* type = expression->GetType();
        if (type->GetSymbolType() == SymbolType::classGroupTypeSymbol)
        {
            ClassGroupTypeSymbol* classGroup = static_cast<ClassGroupTypeSymbol*>(type);
            ClassTypeSymbol* classTypeSymbol = classGroup->GetClass(0);
            if (!classTypeSymbol)
            {
                throw Exception("ordinary class not found from class group '" + ToUtf8(classGroup->FullName()) + "'", span, classGroup->GetSpan());
            }
            expression.reset(new BoundTypeExpression(span, classTypeSymbol));
            type = classTypeSymbol;
        }
        if (!scopeQualified)
        {
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base, type->BaseType()->ClassInterfaceEnumDelegateOrNsScope()));
        }
        temporary = boundFunction->GetFunctionSymbol()->CreateTemporary(type, invokeNode.GetSpan());
        std::unique_ptr<BoundExpression> addrOfTemporary(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)), type->AddPointer(invokeNode.GetSpan())));
        arguments.push_back(std::move(addrOfTemporary));
        groupName = U"@constructor";
        if (type->IsClassTypeSymbol())
        {
            ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(type);
            if (classType->Destructor())
            {
                std::unique_ptr<BoundFunctionCall> destructorCall(new BoundFunctionCall(span, classType->Destructor()));
                destructorCall->AddArgument(std::unique_ptr<BoundExpression>(arguments.back()->Clone()));
                boundFunction->AddTemporaryDestructorCall(std::move(destructorCall));
            }
        }
    }
    else if (expression->GetType()->PlainType(span)->IsClassTypeSymbol())
    {
        TypeSymbol* type = expression->GetType();
        ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(type->PlainType(span));
        groupName = U"operator()";
        functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_, classType->GetContainerScope()));
        if (type->IsReferenceType())
        {
            expression.reset(new BoundReferenceToPointerExpression(std::move(expression), type->RemoveReference(span)->AddPointer(span)));
        }
        else
        {
            expression.reset(new BoundAddressOfExpression(std::move(expression), type->AddPointer(span)));
        }
        arguments.push_back(std::unique_ptr<BoundExpression>(expression.release()));
    }
    else if (expression->GetType()->PlainType(span)->GetSymbolType() == SymbolType::delegateTypeSymbol)
    {
        TypeSymbol* type = expression->GetType();
        if (type->IsReferenceType())
        {
            arguments.push_back(std::unique_ptr<BoundExpression>(new BoundDereferenceExpression(std::move(expression), type->RemoveReference(span))));
        }
        else
        {
            arguments.push_back(std::move(expression));
        }
        DelegateTypeSymbol* delegateTypeSymbol = static_cast<DelegateTypeSymbol*>(type->BaseType());
        int n = invokeNode.Arguments().Count();
        if (n != delegateTypeSymbol->Arity())
        {
            throw Exception("wrong number of arguments for calling delegate type '" + ToUtf8(delegateTypeSymbol->FullName()) + "'", span);
        }
        for (int i = 0; i < n; ++i)
        {
            TypeSymbol* delegateParameterType = delegateTypeSymbol->Parameters()[i]->GetType();
            Node* argument = invokeNode.Arguments()[i];
            argument->Accept(*this);
            TypeSymbol* argumentType = expression->GetType();
            if (!TypesEqual(argumentType, delegateParameterType))
            {
                if (TypesEqual(argumentType->PlainType(span), delegateParameterType->PlainType(span)))
                {
                    if (argumentType->IsReferenceType() && !delegateParameterType->IsReferenceType())
                    {
                        TypeSymbol* type = argumentType->RemoveReference(span);
                        BoundDereferenceExpression* dereferenceExpression = new BoundDereferenceExpression(std::move(expression), type);
                        expression.reset(dereferenceExpression);
                    }
                    else if (!argumentType->IsReferenceType() && (delegateParameterType->IsReferenceType() || delegateParameterType->IsClassTypeSymbol()))
                    {
                        TypeSymbol* type = argumentType->AddLvalueReference(span);
                        BoundAddressOfExpression* addressOfExpression = new BoundAddressOfExpression(std::move(expression), type);
                        expression.reset(addressOfExpression);
                    }
                }
                else
                {
                    ArgumentMatch argumentMatch;
                    FunctionSymbol* conversionFun = boundCompileUnit.GetConversion(argumentType, delegateParameterType, containerScope, boundFunction, span, argumentMatch);
                    if (conversionFun)
                    {
                        BoundConversion* conversion = new BoundConversion(std::move(expression), conversionFun);
                        expression.reset(conversion);
                    }
                    else
                    {
                        throw Exception("cannot convert '" + ToUtf8(argumentType->FullName()) + "' type argument to '" + ToUtf8(delegateParameterType->FullName()) + "' type parameter",
                            argument->GetSpan(), span);
                    }
                }
            }
            arguments.push_back(std::unique_ptr<BoundExpression>(expression.release()));
        }
        BoundDelegateCall* delegateCall = new BoundDelegateCall(span, delegateTypeSymbol);
        for (std::unique_ptr<BoundExpression>& argument : arguments)
        {
            delegateCall->AddArgument(std::move(argument));
        }
        LocalVariableSymbol* temporary = nullptr;
        if (delegateTypeSymbol->ReturnsClassInterfaceOrClassDelegateByValue())
        {
            TypeSymbol* type = delegateTypeSymbol->ReturnType();
            temporary = boundFunction->GetFunctionSymbol()->CreateTemporary(type, invokeNode.GetSpan());
            delegateCall->AddArgument(std::unique_ptr<BoundExpression>(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)),
                type->AddPointer(invokeNode.GetSpan()))));
        }
        expression.reset(delegateCall);
        if (temporary)
        {
            expression.reset(new BoundConstructAndReturnTemporaryExpression(std::move(expression), std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary))));
            expression->SetFlag(BoundExpressionFlags::bindToRvalueReference);
        }
        return;
    }
    else if (expression->GetType()->PlainType(span)->GetSymbolType() == SymbolType::classDelegateTypeSymbol)
    {
        TypeSymbol* type = expression->GetType();
        if (type->IsReferenceType())
        {
            arguments.push_back(std::unique_ptr<BoundExpression>(new BoundReferenceToPointerExpression(std::move(expression), type->RemoveReference(span)->AddPointer(span))));
        }
        else
        {
            arguments.push_back(std::unique_ptr<BoundExpression>(new BoundAddressOfExpression(std::move(expression), type->AddPointer(span))));
        }
        ClassDelegateTypeSymbol* classDelegateTypeSymbol = static_cast<ClassDelegateTypeSymbol*>(type->BaseType());
        int n = invokeNode.Arguments().Count();
        if (n != classDelegateTypeSymbol->Arity())
        {
            throw Exception("wrong number of arguments for calling delegate type '" + ToUtf8(classDelegateTypeSymbol->FullName()) + "'", span);
        }
        for (int i = 0; i < n; ++i)
        {
            TypeSymbol* classDelegateParameterType = classDelegateTypeSymbol->Parameters()[i]->GetType();
            Node* argument = invokeNode.Arguments()[i];
            argument->Accept(*this);
            TypeSymbol* argumentType = expression->GetType();
            if (!TypesEqual(argumentType, classDelegateParameterType))
            {
                if (TypesEqual(argumentType->PlainType(span), classDelegateParameterType->PlainType(span)))
                {
                    if (argumentType->IsReferenceType() && !classDelegateParameterType->IsReferenceType())
                    {
                        TypeSymbol* type = argumentType->RemoveReference(span);
                        BoundDereferenceExpression* dereferenceExpression = new BoundDereferenceExpression(std::move(expression), type);
                        expression.reset(dereferenceExpression);
                    }
                    else if (!argumentType->IsReferenceType() && (classDelegateParameterType->IsReferenceType() || classDelegateParameterType->IsClassTypeSymbol()))
                    {
                        TypeSymbol* type = argumentType->AddLvalueReference(span);
                        BoundAddressOfExpression* addressOfExpression = new BoundAddressOfExpression(std::move(expression), type);
                        expression.reset(addressOfExpression);
                    }
                }
                else
                {
                    ArgumentMatch argumentMatch;
                    FunctionSymbol* conversionFun = boundCompileUnit.GetConversion(argumentType, classDelegateParameterType, containerScope, boundFunction, span, argumentMatch);
                    if (conversionFun)
                    {
                        BoundConversion* conversion = new BoundConversion(std::move(expression), conversionFun);
                        expression.reset(conversion);
                    }
                    else
                    {
                        throw Exception("cannot convert '" + ToUtf8(argumentType->FullName()) + "' type argument to '" + ToUtf8(classDelegateParameterType->FullName()) + "' type parameter",
                            argument->GetSpan(), span);
                    }
                }
            }
            arguments.push_back(std::unique_ptr<BoundExpression>(expression.release()));
        }
        BoundClassDelegateCall* classDelegateCall = new BoundClassDelegateCall(span, classDelegateTypeSymbol);
        for (std::unique_ptr<BoundExpression>& argument : arguments)
        {
            classDelegateCall->AddArgument(std::move(argument));
        }
        LocalVariableSymbol* temporary = nullptr;
        if (classDelegateTypeSymbol->ReturnsClassInterfaceOrClassDelegateByValue())
        {
            TypeSymbol* type = classDelegateTypeSymbol->ReturnType();
            temporary = boundFunction->GetFunctionSymbol()->CreateTemporary(type, invokeNode.GetSpan());
            classDelegateCall->AddArgument(std::unique_ptr<BoundExpression>(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)),
                type->AddPointer(invokeNode.GetSpan()))));
        }
        expression.reset(classDelegateCall);
        if (temporary)
        {
            expression.reset(new BoundConstructAndReturnTemporaryExpression(std::move(expression), std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary))));
            expression->SetFlag(BoundExpressionFlags::bindToRvalueReference);
        }
        return;
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
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, expression->GetType()->BaseType()->ClassInterfaceEnumDelegateOrNsScope()));
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
    std::unique_ptr<BoundFunctionCall> functionCall = ResolveOverload(groupName, containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, 
        invokeNode.GetSpan(), OverloadResolutionFlags::dontThrow, templateArgumentTypes, exception);
    if (!functionCall)
    {
        ParameterSymbol* thisParam = boundFunction->GetFunctionSymbol()->GetThisParam();
        bool thisParamInserted = false;
        if (thisParam)
        {
            BoundParameter* boundThisParam = new BoundParameter(thisParam);
            arguments.insert(arguments.begin(), std::unique_ptr<BoundExpression>(boundThisParam));
            thisParamInserted = true;
            functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base, thisParam->GetType()->BaseType()->ClassInterfaceEnumDelegateOrNsScope()));
            functionCall = std::move(ResolveOverload(groupName, containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, invokeNode.GetSpan(),
                OverloadResolutionFlags::dontThrow, templateArgumentTypes, thisEx));
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
            functionCall = std::move(ResolveOverload(groupName, containerScope, functionScopeLookups, arguments, boundCompileUnit, boundFunction, invokeNode.GetSpan(),
                OverloadResolutionFlags::dontThrow, templateArgumentTypes, nsEx));
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
        Exception* ex = exception.get();
        if (dynamic_cast<NoViableFunctionException*>(ex) && thisEx)
        {
            ex = thisEx.get();
        }
        if (dynamic_cast<NoViableFunctionException*>(ex) && nsEx)
        {
            ex = nsEx.get();
        }
        if (ex)
        {
            throw *ex;
        }
        else
        {
            throw Exception("overload resolution failed: overload not found", invokeNode.GetSpan());
        }
    }
    CheckAccess(boundFunction->GetFunctionSymbol(), functionCall->GetFunctionSymbol());
    FunctionSymbol* functionSymbol = functionCall->GetFunctionSymbol();
    if (functionSymbol->GetSymbolType() == SymbolType::memberFunctionSymbol && !functionSymbol->IsStatic() && functionSymbol->IsVirtualAbstractOrOverride())
    {
        Assert(!functionCall->Arguments().empty(), "nonempty argument list expected");
        if (!functionCall->Arguments()[0]->GetFlag(BoundExpressionFlags::argIsExplicitThisOrBasePtr))
        {
            functionCall->SetFlag(BoundExpressionFlags::virtualCall);
        }
    }
    if (functionSymbol->ReturnsClassInterfaceOrClassDelegateByValue())
    {
        TypeSymbol* type = functionSymbol->ReturnType();
        temporary = boundFunction->GetFunctionSymbol()->CreateTemporary(type, invokeNode.GetSpan());
        functionCall->AddArgument(std::unique_ptr<BoundExpression>(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)), 
            type->AddPointer(invokeNode.GetSpan()))));
    }
    expression.reset(functionCall.release());
    if (temporary)
    {
        expression.reset(new BoundConstructAndReturnTemporaryExpression(std::move(expression), std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary))));
        expression->SetFlag(BoundExpressionFlags::bindToRvalueReference);
    }
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
    if (expression->GetBoundNodeType() == BoundNodeType::boundTypeExpression && expression->GetType()->GetSymbolType() == SymbolType::classGroupTypeSymbol)
    {
        ClassGroupTypeSymbol* classGroup = static_cast<ClassGroupTypeSymbol*>(expression->GetType());
        ClassTypeSymbol* classTypeSymbol = classGroup->GetClass(0);
        if (classTypeSymbol)
        {
            expression.reset(new BoundTypeExpression(span, classTypeSymbol));
        }
        else
        {
            throw Exception("ordinary class not found from class group '" + ToUtf8(classGroup->FullName()) + "'", span, classGroup->GetSpan());
        }
    }
    expression.reset(new BoundSizeOfExpression(sizeOfNode.GetSpan(), symbolTable.GetTypeByName(U"long"), expression->GetType()->AddPointer(sizeOfNode.GetSpan())));
}

void ExpressionBinder::Visit(TypeNameNode& typeNameNode) 
{
    std::unique_ptr<BoundExpression> expr = BindExpression(typeNameNode.Expression(), boundCompileUnit, boundFunction, containerScope, statementBinder, false, false, false, false, false);
    if (expr->GetType()->PlainType(typeNameNode.GetSpan())->IsClassTypeSymbol())
    {
        ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(expr->GetType()->BaseType());
        if (classType->IsPolymorphic())
        {
            if (expr->GetBoundNodeType() == BoundNodeType::boundDereferenceExpression)
            {
                BoundDereferenceExpression* derefExpr = static_cast<BoundDereferenceExpression*>(expr.get());
                expr.reset(derefExpr->Subject().release());
            }
            else
            {
                TypeSymbol* ptrType = expr->GetType()->AddPointer(typeNameNode.GetSpan());
                expr.reset(new BoundAddressOfExpression(std::move(expr), ptrType));
            }
            expression.reset(new BoundTypeNameExpression(std::move(expr), symbolTable.GetTypeByName(U"char")->AddConst(typeNameNode.GetSpan())->AddPointer(typeNameNode.GetSpan())));
        }
        else
        {
            expression.reset(new BoundLiteral(std::unique_ptr<Value>(new StringValue(typeNameNode.GetSpan(), boundCompileUnit.Install(ToUtf8(classType->FullName())))),
                symbolTable.GetTypeByName(U"char")->AddConst(typeNameNode.GetSpan())->AddPointer(typeNameNode.GetSpan())));
        }
    }
    else
    {
        expression.reset(new BoundLiteral(std::unique_ptr<Value>(new StringValue(typeNameNode.GetSpan(), boundCompileUnit.Install(ToUtf8(expr->GetType()->FullName())))),
            symbolTable.GetTypeByName(U"char")->AddConst(typeNameNode.GetSpan())->AddPointer(typeNameNode.GetSpan())));
    }
}

void ExpressionBinder::Visit(CastNode& castNode) 
{
    TypeSymbol* targetType = ResolveType(castNode.TargetTypeExpr(), boundCompileUnit, containerScope);
    castNode.SourceExpr()->Accept(*this);
    std::vector<std::unique_ptr<BoundExpression>> targetExprArgs;
    targetExprArgs.push_back(std::unique_ptr<BoundExpression>(new BoundTypeExpression(castNode.GetSpan(), targetType)));
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, targetType->BaseType()->ClassInterfaceEnumDelegateOrNsScope()));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::fileScopes, nullptr));
    std::unique_ptr<BoundFunctionCall> castFunctionCall = ResolveOverload(U"@return", containerScope, functionScopeLookups, targetExprArgs, boundCompileUnit, boundFunction, castNode.GetSpan());
    std::vector<std::unique_ptr<BoundExpression>> castArguments;
    castArguments.push_back(std::move(expression));
    FunctionMatch functionMatch(castFunctionCall->GetFunctionSymbol());
    bool conversionFound = FindConversions(boundCompileUnit, castFunctionCall->GetFunctionSymbol(), castArguments, functionMatch, ConversionType::explicit_, containerScope, boundFunction,
        castNode.GetSpan());
    if (conversionFound)
    {
        Assert(!functionMatch.argumentMatches.empty(), "argument match expected");
        ArgumentMatch& argumentMatch = functionMatch.argumentMatches[0];
        if (argumentMatch.preReferenceConversionFlags != OperationFlags::none)
        {
            if (argumentMatch.preReferenceConversionFlags == OperationFlags::addr)
            {
                TypeSymbol* type = castArguments[0]->GetType()->AddLvalueReference(span);
                BoundAddressOfExpression* addressOfExpression = new BoundAddressOfExpression(std::move(castArguments[0]), type);
                castArguments[0].reset(addressOfExpression);
            }
            else if (argumentMatch.preReferenceConversionFlags == OperationFlags::deref)
            {
                TypeSymbol* type = castArguments[0]->GetType()->RemoveReference(span);
                BoundDereferenceExpression* dereferenceExpression = new BoundDereferenceExpression(std::move(castArguments[0]), type);
                castArguments[0].reset(dereferenceExpression);
            }
        }
        FunctionSymbol* conversionFun = argumentMatch.conversionFun;
        if (conversionFun)
        {
            if (conversionFun->GetSymbolType() == SymbolType::constructorSymbol)
            {
                BoundFunctionCall* constructorCall = new BoundFunctionCall(span, conversionFun);
                LocalVariableSymbol* temporary = boundFunction->GetFunctionSymbol()->CreateTemporary(conversionFun->ConversionTargetType(), span);
                constructorCall->AddArgument(std::unique_ptr<BoundExpression>(new BoundAddressOfExpression(std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)),
                    conversionFun->ConversionTargetType()->AddPointer(span))));
                TypeSymbol* conversionTargetType = conversionFun->ConversionTargetType();
                if (conversionTargetType->IsClassTypeSymbol())
                {
                    ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(conversionTargetType);
                    if (classType->Destructor())
                    {
                        std::unique_ptr<BoundFunctionCall> destructorCall(new BoundFunctionCall(span, classType->Destructor()));
                        destructorCall->AddArgument(std::unique_ptr<BoundExpression>(constructorCall->Arguments()[0]->Clone()));
                        boundFunction->AddTemporaryDestructorCall(std::move(destructorCall));
                    }
                }
                constructorCall->AddArgument(std::move(castArguments[0]));
                BoundConstructAndReturnTemporaryExpression* conversion = new BoundConstructAndReturnTemporaryExpression(std::unique_ptr<BoundExpression>(constructorCall),
                    std::unique_ptr<BoundExpression>(new BoundLocalVariable(temporary)));
                castArguments[0].reset(conversion);
            }
            else
            {
                castArguments[0].reset(new BoundConversion(std::unique_ptr<BoundExpression>(castArguments[0].release()), conversionFun));
            }
        }
        if (argumentMatch.postReferenceConversionFlags != OperationFlags::none)
        {
            if (argumentMatch.postReferenceConversionFlags == OperationFlags::addr)
            {
                TypeSymbol* type = castArguments[0]->GetType()->AddLvalueReference(span);
                BoundAddressOfExpression* addressOfExpression = new BoundAddressOfExpression(std::move(castArguments[0]), type);
                castArguments[0].reset(addressOfExpression);
            }
            else if (argumentMatch.postReferenceConversionFlags == OperationFlags::deref)
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
            if (!resultType->IsPointerType())
            {
                throw Exception("first argument of a construct expression must be of a pointer type", argumentNode->GetSpan());
            }
            if (!resultType->RemovePointer(constructNode.GetSpan())->IsClassTypeSymbol())
            {
                expression->SetFlag(BoundExpressionFlags::deref);
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
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, resultType->RemovePointer(constructNode.GetSpan())->ClassInterfaceEnumDelegateOrNsScope()));
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
    if (!resultType->RemovePointer(newNode.GetSpan())->IsClassTypeSymbol())
    {
        expression->SetFlag(BoundExpressionFlags::deref);
    }
    arguments.push_back(std::move(expression));
    int n = newNode.Arguments().Count();
    for (int i = 0; i < n; ++i)
    {
        newNode.Arguments()[i]->Accept(*this);
        arguments.push_back(std::move(expression));
    }
    std::vector<FunctionScopeLookup> functionScopeLookups;
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, containerScope));
    functionScopeLookups.push_back(FunctionScopeLookup(ScopeLookup::this_and_base_and_parent, resultType->RemovePointer(newNode.GetSpan())->ClassInterfaceEnumDelegateOrNsScope()));
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
        expression->SetFlag(BoundExpressionFlags::argIsExplicitThisOrBasePtr);
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
                ArgumentMatch argumentMatch;
                FunctionSymbol* thisAsBaseConversionFunction = boundCompileUnit.GetConversion(thisParam->GetType(), basePointerType, containerScope, boundFunction, baseNode.GetSpan(), argumentMatch);
                if (thisAsBaseConversionFunction)
                {
                    expression.reset(new BoundConversion(std::unique_ptr<BoundExpression>(new BoundParameter(thisParam)), thisAsBaseConversionFunction));
                    expression->SetFlag(BoundExpressionFlags::argIsExplicitThisOrBasePtr);
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
    return BindExpression(node, boundCompileUnit, boundFunction, containerScope, statementBinder, lvalue, acceptFunctionGroup, acceptMemberExpression, acceptIncomplete, true);
}

std::unique_ptr<BoundExpression> BindExpression(Node* node, BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, StatementBinder* statementBinder, bool lvalue,
    bool acceptFunctionGroup, bool acceptMemberExpression, bool acceptIncomplete, bool moveTemporaryDestructorCalls)
{
    ExpressionBinder expressionBinder(node->GetSpan(), boundCompileUnit, boundFunction, containerScope, statementBinder, lvalue);
    node->Accept(expressionBinder);
    std::unique_ptr<BoundExpression> expression = expressionBinder.GetExpression();
    if (!expression)
    {
        throw Exception("could not bind expression", node->GetSpan());
    }
    if (moveTemporaryDestructorCalls)
    {
        boundFunction->MoveTemporaryDestructorCallsTo(*expression);
    }
    if (acceptFunctionGroup && expression->GetBoundNodeType() == BoundNodeType::boundFunctionGroupExpression)
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

std::unique_ptr<BoundExpression> BindUnaryOp(BoundExpression* operand, Node& node, const std::u32string& groupName,
    BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope, StatementBinder* statementBinder)
{
    ExpressionBinder expressionBinder(node.GetSpan(), boundCompileUnit, boundFunction, containerScope, statementBinder, false);
    expressionBinder.BindUnaryOp(operand, node, groupName);
    std::unique_ptr<BoundExpression> expression = expressionBinder.GetExpression();
    if (!expression)
    {
        throw Exception("cound not bind expression", node.GetSpan());
    }
    return expression;
}

} } // namespace cmajor::binder
