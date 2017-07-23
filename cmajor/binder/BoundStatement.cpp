// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundStatement.hpp>
#include <cmajor/binder/BoundNodeVisitor.hpp>
#include <cmajor/binder/BoundExpression.hpp>
#include <cmajor/symbols/Exception.hpp>

namespace cmajor { namespace binder {

BoundStatement::BoundStatement(const Span& span_, BoundNodeType boundNodeType_) : BoundNode(span_, boundNodeType_), parent(nullptr), postfix(false)
{
}

BoundCompoundStatement* BoundStatement::Block() 
{
    if (GetBoundNodeType() == BoundNodeType::boundCompoundStatement)
    {
        return static_cast<BoundCompoundStatement*>(this);
    }
    return parent->Block();
}

void BoundStatement::Load(Emitter& emitter)
{
    throw Exception("cannot load from statement", GetSpan());
}

void BoundStatement::Store(Emitter& emitter)
{
    throw Exception("cannot store to statement", GetSpan());
}

void BoundStatement::SetLabel(const std::u32string& label_)
{
    label = label_;
}

BoundSequenceStatement::BoundSequenceStatement(const Span& span_, std::unique_ptr<BoundStatement>&& first_, std::unique_ptr<BoundStatement>&& second_) : 
    BoundStatement(span_, BoundNodeType::boundSequenceStatement), first(std::move(first_)), second(std::move(second_))
{
    first->SetParent(this);
    second->SetParent(this);
}

void BoundSequenceStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundCompoundStatement::BoundCompoundStatement(const Span& span_) : BoundStatement(span_, BoundNodeType::boundCompoundStatement)
{
}

void BoundCompoundStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

void BoundCompoundStatement::AddStatement(std::unique_ptr<BoundStatement>&& statement)
{
    statement->SetParent(this);
    statements.push_back(std::move(statement));
}

BoundReturnStatement::BoundReturnStatement(std::unique_ptr<BoundFunctionCall>&& returnFunctionCall_) : 
    BoundStatement(returnFunctionCall_->GetSpan(), BoundNodeType::boundReturnStatement), returnFunctionCall(std::move(returnFunctionCall_))
{
}

void BoundReturnStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundIfStatement::BoundIfStatement(const Span& span_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& thenS_, std::unique_ptr<BoundStatement>&& elseS_) :
    BoundStatement(span_, BoundNodeType::boundIfStatement), condition(std::move(condition_)), thenS(std::move(thenS_)), elseS(std::move(elseS_))
{
    thenS->SetParent(this);
    if (elseS)
    {
        elseS->SetParent(this);
    }
}

void BoundIfStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundWhileStatement::BoundWhileStatement(const Span& span_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& statement_) : 
    BoundStatement(span_, BoundNodeType::boundWhileStatement), condition(std::move(condition_)), statement(std::move(statement_))
{
    statement->SetParent(this);
}

void BoundWhileStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundDoStatement::BoundDoStatement(const Span& span_, std::unique_ptr<BoundStatement>&& statement_, std::unique_ptr<BoundExpression>&& condition_) :
    BoundStatement(span_, BoundNodeType::boundDoStatement), statement(std::move(statement_)), condition(std::move(condition_))
{
    statement->SetParent(this);
}

void BoundDoStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundForStatement::BoundForStatement(const Span& span_, std::unique_ptr<BoundStatement>&& initS_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& loopS_,
    std::unique_ptr<BoundStatement>&& actionS_) : BoundStatement(span_, BoundNodeType::boundForStatement), initS(std::move(initS_)), condition(std::move(condition_)), loopS(std::move(loopS_)), 
    actionS(std::move(actionS_))
{
    initS->SetParent(this);
    loopS->SetParent(this);
    actionS->SetParent(this);
}

void BoundForStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundBreakStatement::BoundBreakStatement(const Span& span_) : BoundStatement(span_, BoundNodeType::boundBreakStatement)
{
}

void BoundBreakStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundContinueStatement::BoundContinueStatement(const Span& span_) : BoundStatement(span_, BoundNodeType::boundContinueStatement)
{
}

void BoundContinueStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundGotoStatement::BoundGotoStatement(const Span& span_, const std::u32string& target_) : BoundStatement(span_, BoundNodeType::boundGotoStatement), target(target_)
{
}

void BoundGotoStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundConstructionStatement::BoundConstructionStatement(std::unique_ptr<BoundFunctionCall>&& constructorCall_) : 
    BoundStatement(constructorCall_->GetSpan(), BoundNodeType::boundConstructionStatement), constructorCall(std::move(constructorCall_))
{
}

void BoundConstructionStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundAssignmentStatement::BoundAssignmentStatement(std::unique_ptr<BoundFunctionCall>&& assignmentCall_) :
    BoundStatement(assignmentCall_->GetSpan(), BoundNodeType::boundAssignmentStatement), assignmentCall(std::move(assignmentCall_))
{
}

void BoundAssignmentStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundExpressionStatement::BoundExpressionStatement(std::unique_ptr<BoundExpression>&& expression_) : BoundStatement(expression_->GetSpan(), BoundNodeType::boundExpressionStatement), expression(std::move(expression_))
{ 
}

void BoundExpressionStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundEmptyStatement::BoundEmptyStatement(const Span& span_) : BoundStatement(span_, BoundNodeType::boundEmptyStatement)
{
}

void BoundEmptyStatement::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cmajor::binder
