// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/ExpressionBinder.hpp>
#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/binder/BoundExpression.hpp>
#include <cmajor/binder/OverloadResolution.hpp>
#include <cmajor/binder/Access.hpp>
#include <cmajor/ast/Expression.hpp>
#include <cmajor/ast/Visitor.hpp>

namespace cmajor { namespace binder {

using cmajor::parsing::Span;

class ExpressionBinder : public cmajor::ast::Visitor
{
public:
    ExpressionBinder(const Span& span_, BoundCompileUnit& boundCompileUnit_, BoundFunction* boundFunction_, ContainerScope* containerScope_);
    std::unique_ptr<BoundExpression> GetExpression() { return std::move(expression); }

    void Visit(DotNode& dotNode) override;
    void Visit(ArrowNode& arrowNode) override;
    void Visit(EquivalenceNode& equivalenceNode) override;
    void Visit(ImplicationNode& implicationNode) override;
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
    BoundFunction* boundFunction;
    ContainerScope* containerScope;
    std::unique_ptr<BoundExpression> expression;
    void BindBinaryOp(BinaryNode& binaryNode, const std::u32string& groupName);
    void BindBinaryOp(BoundExpression* left, BoundExpression* right, Node& node, const std::u32string& groupName);
};

ExpressionBinder::ExpressionBinder(const Span& span_, BoundCompileUnit& boundCompileUnit_, BoundFunction* boundFunction_, ContainerScope* containerScope_) : 
    span(span_), boundCompileUnit(boundCompileUnit_), boundFunction(boundFunction_), containerScope(containerScope_)
{
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
    std::unique_ptr<BoundFunctionCall> operatorFunCall = ResolveOverload(groupName, functionScopeLookups, arguments, boundCompileUnit, node.GetSpan());
    CheckAccess(boundFunction->GetFunctionSymbol(), operatorFunCall->GetFunctionSymbol());
    expression.reset(operatorFunCall.release());
}

void ExpressionBinder::Visit(DotNode& dotNode)
{
}

void ExpressionBinder::Visit(ArrowNode& arrowNode) 
{
}

void ExpressionBinder::Visit(EquivalenceNode& equivalenceNode) 
{
}

void ExpressionBinder::Visit(ImplicationNode& implicationNode) 
{
}

void ExpressionBinder::Visit(DisjunctionNode& disjunctionNode) 
{
}

void ExpressionBinder::Visit(ConjunctionNode& conjunctionNode) 
{
}

void ExpressionBinder::Visit(BitOrNode& bitOrNode) 
{
}

void ExpressionBinder::Visit(BitXorNode& bitXorNode) 
{
}

void ExpressionBinder::Visit(BitAndNode& bitAndNode) 
{
}

void ExpressionBinder::Visit(EqualNode& equalNode) 
{
}

void ExpressionBinder::Visit(NotEqualNode& notEqualNode) 
{
}

void ExpressionBinder::Visit(LessNode& lessNode) 
{
}

void ExpressionBinder::Visit(GreaterNode& greaterNode) 
{
}

void ExpressionBinder::Visit(LessOrEqualNode& lessOrEqualNode) 
{
}

void ExpressionBinder::Visit(GreaterOrEqualNode& greaterOrEqualNode) 
{
}

void ExpressionBinder::Visit(ShiftLeftNode& shiftLeftNode) 
{
}

void ExpressionBinder::Visit(ShiftRightNode& shiftRightNode) 
{
}

void ExpressionBinder::Visit(AddNode& addNode) 
{
    BindBinaryOp(addNode, U"operator+");
}

void ExpressionBinder::Visit(SubNode& subNode) 
{
}

void ExpressionBinder::Visit(MulNode& mulNode) 
{
}

void ExpressionBinder::Visit(DivNode& divNode) 
{
}

void ExpressionBinder::Visit(RemNode& remNode) 
{
}

void ExpressionBinder::Visit(NotNode& notNode) 
{
}

void ExpressionBinder::Visit(UnaryPlusNode& unaryPlusNode) 
{
}

void ExpressionBinder::Visit(UnaryMinusNode& unaryMinusNode) 
{
}

void ExpressionBinder::Visit(PrefixIncrementNode& prefixIncrementNode) 
{
}

void ExpressionBinder::Visit(PrefixDecrementNode& prefixDecrementNode) 
{
}

void ExpressionBinder::Visit(DerefNode& derefNode) 
{
}

void ExpressionBinder::Visit(AddrOfNode& addrOfNode) 
{
}

void ExpressionBinder::Visit(ComplementNode& complementNode) 
{
}

void ExpressionBinder::Visit(IsNode& isNode) 
{
}

void ExpressionBinder::Visit(AsNode& asNode) 
{
}

void ExpressionBinder::Visit(IndexingNode& indexingNode) 
{
}

void ExpressionBinder::Visit(InvokeNode& invokeNode) 
{
}

void ExpressionBinder::Visit(PostfixIncrementNode& postfixIncrementNode) 
{
}

void ExpressionBinder::Visit(PostfixDecrementNode& postfixDecrementNode) 
{
}

void ExpressionBinder::Visit(SizeOfNode& sizeOfNode) 
{
}

void ExpressionBinder::Visit(TypeNameNode& typeNameNode) 
{
}

void ExpressionBinder::Visit(CastNode& castNode) 
{
}

void ExpressionBinder::Visit(ConstructNode& constructNode) 
{
}

void ExpressionBinder::Visit(NewNode& newNode) 
{
}

void ExpressionBinder::Visit(ThisNode& thisNode) 
{
}

void ExpressionBinder::Visit(BaseNode& baseNode) 
{
}

std::unique_ptr<BoundExpression> BindExpression(Node* node, BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, ContainerScope* containerScope)
{
    ExpressionBinder expressionBinder(node->GetSpan(), boundCompileUnit, boundFunction, containerScope);
    node->Accept(expressionBinder);
    std::unique_ptr<BoundExpression> expression = expressionBinder.GetExpression();
    return expression;
}

} } // namespace cmajor::binder