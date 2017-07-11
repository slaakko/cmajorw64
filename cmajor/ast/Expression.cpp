// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/Expression.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/ast/Visitor.hpp>

namespace cmajor { namespace ast {

DotNode::DotNode(const Span& span_) : Node(NodeType::dotNode, span_), subject(), memberId()
{
}

DotNode::DotNode(const Span& span_, Node* subject_, IdentifierNode* memberId_) : Node(NodeType::dotNode, span_), subject(subject_), memberId(memberId_)
{
    subject->SetParent(this);
    memberId->SetParent(this);
}

Node* DotNode::Clone(CloneContext& cloneContext) const
{
    return new DotNode(GetSpan(), subject->Clone(cloneContext), static_cast<IdentifierNode*>(memberId->Clone(cloneContext)));
}

void DotNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void DotNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
    writer.Write(memberId.get());
}

void DotNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
    memberId.reset(reader.ReadIdentifierNode());
    memberId->SetParent(this);
}

ArrowNode::ArrowNode(const Span& span_) : Node(NodeType::arrowNode, span_), subject(), memberId()
{
}

ArrowNode::ArrowNode(const Span& span_, Node* subject_, IdentifierNode* memberId_) : Node(NodeType::arrowNode, span_), subject(subject_), memberId(memberId_)
{
    subject->SetParent(this);
    memberId->SetParent(this);
}

Node* ArrowNode::Clone(CloneContext& cloneContext) const
{
    return new ArrowNode(GetSpan(), subject->Clone(cloneContext), static_cast<IdentifierNode*>(memberId->Clone(cloneContext)));
}

void ArrowNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void ArrowNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
    writer.Write(memberId.get());
}

void ArrowNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
    memberId.reset(reader.ReadIdentifierNode());
    memberId->SetParent(this);
}

EquivalenceNode::EquivalenceNode(const Span& span_) : BinaryNode(NodeType::equivalenceNode, span_)
{
}

EquivalenceNode::EquivalenceNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::equivalenceNode, span_, left_, right_)
{
}

Node* EquivalenceNode::Clone(CloneContext& cloneContext) const
{
    return new EquivalenceNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void EquivalenceNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ImplicationNode::ImplicationNode(const Span& span_) : BinaryNode(NodeType::implicationNode, span_)
{
}

ImplicationNode::ImplicationNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::implicationNode, span_, left_, right_)
{
}

Node* ImplicationNode::Clone(CloneContext& cloneContext) const
{
    return new ImplicationNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void ImplicationNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

DisjunctionNode::DisjunctionNode(const Span& span_) : BinaryNode(NodeType::disjunctionNode, span_)
{
}

DisjunctionNode::DisjunctionNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::disjunctionNode, span_, left_, right_)
{
}

Node* DisjunctionNode::Clone(CloneContext& cloneContext) const
{
    return new DisjunctionNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void DisjunctionNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ConjunctionNode::ConjunctionNode(const Span& span_) : BinaryNode(NodeType::conjunctionNode, span_)
{
}

ConjunctionNode::ConjunctionNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::conjunctionNode, span_, left_, right_)
{
}

Node* ConjunctionNode::Clone(CloneContext& cloneContext) const
{
    return new ConjunctionNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void ConjunctionNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

BitOrNode::BitOrNode(const Span& span_) : BinaryNode(NodeType::bitOrNode, span_)
{
}

BitOrNode::BitOrNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::bitOrNode, span_, left_, right_)
{
}

Node* BitOrNode::Clone(CloneContext& cloneContext) const
{
    return new BitOrNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void BitOrNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

BitXorNode::BitXorNode(const Span& span_) : BinaryNode(NodeType::bitXorNode, span_)
{
}

BitXorNode::BitXorNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::bitXorNode, span_, left_, right_)
{
}

Node* BitXorNode::Clone(CloneContext& cloneContext) const
{
    return new BitXorNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void BitXorNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

BitAndNode::BitAndNode(const Span& span_) : BinaryNode(NodeType::bitAndNode, span_)
{
}

BitAndNode::BitAndNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::bitAndNode, span_, left_, right_)
{
}

Node* BitAndNode::Clone(CloneContext& cloneContext) const
{
    return new BitAndNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void BitAndNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

EqualNode::EqualNode(const Span& span_) : BinaryNode(NodeType::equalNode, span_)
{
}

EqualNode::EqualNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::equalNode, span_, left_, right_)
{
}

Node* EqualNode::Clone(CloneContext& cloneContext) const
{
    return new EqualNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void EqualNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

NotEqualNode::NotEqualNode(const Span& span_) : BinaryNode(NodeType::notEqualNode, span_)
{
}

NotEqualNode::NotEqualNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::notEqualNode, span_, left_, right_)
{
}

Node* NotEqualNode::Clone(CloneContext& cloneContext) const
{
    return new NotEqualNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void NotEqualNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

LessNode::LessNode(const Span& span_) : BinaryNode(NodeType::lessNode, span_)
{
}

LessNode::LessNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::lessNode, span_, left_, right_)
{
}

Node* LessNode::Clone(CloneContext& cloneContext) const
{
    return new LessNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void LessNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

GreaterNode::GreaterNode(const Span& span_) : BinaryNode(NodeType::greaterNode, span_)
{
}

GreaterNode::GreaterNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::greaterNode, span_, left_, right_)
{
}

Node* GreaterNode::Clone(CloneContext& cloneContext) const
{
    return new GreaterNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void GreaterNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

LessOrEqualNode::LessOrEqualNode(const Span& span_) : BinaryNode(NodeType::lessOrEqualNode, span_)
{
}

LessOrEqualNode::LessOrEqualNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::lessOrEqualNode, span_, left_, right_)
{
}

Node* LessOrEqualNode::Clone(CloneContext& cloneContext) const
{
    return new LessOrEqualNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void LessOrEqualNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

GreaterOrEqualNode::GreaterOrEqualNode(const Span& span_) : BinaryNode(NodeType::greaterOrEqualNode, span_)
{
}

GreaterOrEqualNode::GreaterOrEqualNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::greaterOrEqualNode, span_, left_, right_)
{
}

Node* GreaterOrEqualNode::Clone(CloneContext& cloneContext) const
{
    return new GreaterOrEqualNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void GreaterOrEqualNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ShiftLeftNode::ShiftLeftNode(const Span& span_) : BinaryNode(NodeType::shiftLeftNode, span_)
{
}

ShiftLeftNode::ShiftLeftNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::shiftLeftNode, span_, left_, right_)
{
}

Node* ShiftLeftNode::Clone(CloneContext& cloneContext) const
{
    return new ShiftLeftNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void ShiftLeftNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

ShiftRightNode::ShiftRightNode(const Span& span_) : BinaryNode(NodeType::shiftRightNode, span_)
{
}

ShiftRightNode::ShiftRightNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::shiftRightNode, span_, left_, right_)
{
}

Node* ShiftRightNode::Clone(CloneContext& cloneContext) const
{
    return new ShiftRightNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void ShiftRightNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

AddNode::AddNode(const Span& span_) : BinaryNode(NodeType::addNode, span_)
{
}

AddNode::AddNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::addNode, span_, left_, right_)
{
}

Node* AddNode::Clone(CloneContext& cloneContext) const
{
    return new AddNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void AddNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

SubNode::SubNode(const Span& span_) : BinaryNode(NodeType::subNode, span_)
{
}

SubNode::SubNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::subNode, span_, left_, right_)
{
}

Node* SubNode::Clone(CloneContext& cloneContext) const
{
    return new SubNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void SubNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

MulNode::MulNode(const Span& span_) : BinaryNode(NodeType::mulNode, span_)
{
}

MulNode::MulNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::mulNode, span_, left_, right_)
{
}

Node* MulNode::Clone(CloneContext& cloneContext) const
{
    return new MulNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void MulNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

DivNode::DivNode(const Span& span_) : BinaryNode(NodeType::divNode, span_)
{
}

DivNode::DivNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::divNode, span_, left_, right_)
{
}

Node* DivNode::Clone(CloneContext& cloneContext) const
{
    return new DivNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void DivNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

RemNode::RemNode(const Span& span_) : BinaryNode(NodeType::remNode, span_)
{
}

RemNode::RemNode(const Span& span_, Node* left_, Node* right_) : BinaryNode(NodeType::remNode, span_, left_, right_)
{
}

Node* RemNode::Clone(CloneContext& cloneContext) const
{
    return new RemNode(GetSpan(), Left()->Clone(cloneContext), Right()->Clone(cloneContext));
}

void RemNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

NotNode::NotNode(const Span& span_) : Node(NodeType::notNode, span_), subject()
{
}

NotNode::NotNode(const Span& span_, Node* subject_) : Node(NodeType::notNode, span_), subject(subject_)
{
    subject->SetParent(this);
}

Node* NotNode::Clone(CloneContext& cloneContext) const
{
    return new NotNode(GetSpan(), subject->Clone(cloneContext));
}

void NotNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void NotNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
}

void NotNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
}

UnaryPlusNode::UnaryPlusNode(const Span& span_) : Node(NodeType::unaryPlusNode, span_), subject()
{
}

UnaryPlusNode::UnaryPlusNode(const Span& span_, Node* subject_) : Node(NodeType::unaryPlusNode, span_), subject(subject_)
{
    subject->SetParent(this);
}

Node* UnaryPlusNode::Clone(CloneContext& cloneContext) const
{
    return new UnaryPlusNode(GetSpan(), subject->Clone(cloneContext));
}

void UnaryPlusNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void UnaryPlusNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
}

void UnaryPlusNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
}

UnaryMinusNode::UnaryMinusNode(const Span& span_) : Node(NodeType::unaryMinusNode, span_), subject()
{
}

UnaryMinusNode::UnaryMinusNode(const Span& span_, Node* subject_) : Node(NodeType::unaryMinusNode, span_), subject(subject_)
{
    subject->SetParent(this);
}

Node* UnaryMinusNode::Clone(CloneContext& cloneContext) const
{
    return new UnaryMinusNode(GetSpan(), subject->Clone(cloneContext));
}

void UnaryMinusNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void UnaryMinusNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
}

void UnaryMinusNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
}

PrefixIncrementNode::PrefixIncrementNode(const Span& span_) : Node(NodeType::prefixIncrementNode, span_), subject()
{
}

PrefixIncrementNode::PrefixIncrementNode(const Span& span_, Node* subject_) : Node(NodeType::prefixIncrementNode, span_), subject(subject_)
{
    subject->SetParent(this);
}

Node* PrefixIncrementNode::Clone(CloneContext& cloneContext) const
{
    return new PrefixIncrementNode(GetSpan(), subject->Clone(cloneContext));
}

void PrefixIncrementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void PrefixIncrementNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
}

void PrefixIncrementNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
}

PrefixDecrementNode::PrefixDecrementNode(const Span& span_) : Node(NodeType::prefixDecrementNode, span_), subject()
{
}

PrefixDecrementNode::PrefixDecrementNode(const Span& span_, Node* subject_) : Node(NodeType::prefixDecrementNode, span_), subject(subject_)
{
    subject->SetParent(this);
}

Node* PrefixDecrementNode::Clone(CloneContext& cloneContext) const
{
    return new PrefixDecrementNode(GetSpan(), subject->Clone(cloneContext));
}

void PrefixDecrementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void PrefixDecrementNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
}

void PrefixDecrementNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
}

ComplementNode::ComplementNode(const Span& span_) : Node(NodeType::complementNode, span_), subject()
{
}

ComplementNode::ComplementNode(const Span& span_, Node* subject_) : Node(NodeType::complementNode, span_), subject(subject_)
{
    subject->SetParent(this);
}

Node* ComplementNode::Clone(CloneContext& cloneContext) const
{
    return new ComplementNode(GetSpan(), subject->Clone(cloneContext));
}

void ComplementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void ComplementNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
}

void ComplementNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
}

DerefNode::DerefNode(const Span& span_) : Node(NodeType::derefNode, span_), subject()
{
}

DerefNode::DerefNode(const Span& span_, Node* subject_) : Node(NodeType::derefNode, span_), subject(subject_)
{
    subject->SetParent(this);
}

Node* DerefNode::Clone(CloneContext& cloneContext) const
{
    return new DerefNode(GetSpan(), subject->Clone(cloneContext));
}

void DerefNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void DerefNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
}

void DerefNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
}

AddrOfNode::AddrOfNode(const Span& span_) : Node(NodeType::addrOfNode, span_), subject()
{
}

AddrOfNode::AddrOfNode(const Span& span_, Node* subject_) : Node(NodeType::addrOfNode , span_), subject(subject_)
{
    subject->SetParent(this);
}

Node* AddrOfNode::Clone(CloneContext& cloneContext) const
{
    return new AddrOfNode(GetSpan(), subject->Clone(cloneContext));
}

void AddrOfNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void AddrOfNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
}

void AddrOfNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
}

IsNode::IsNode(const Span& span_) : Node(NodeType::isNode, span_), expr(), targetTypeExpr()
{
}

IsNode::IsNode(const Span& span_, Node* expr_, Node* targetTypeExpr_) : Node(NodeType::isNode, span_), expr(expr_), targetTypeExpr(targetTypeExpr_)
{
    expr->SetParent(this);
    targetTypeExpr->SetParent(this);
}

Node* IsNode::Clone(CloneContext& cloneContext) const
{
    return new IsNode(GetSpan(), expr->Clone(cloneContext), targetTypeExpr->Clone(cloneContext));
}

void IsNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void IsNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(expr.get());
    writer.Write(targetTypeExpr.get());
}

void IsNode::Read(AstReader& reader)
{
    Node::Read(reader);
    expr.reset(reader.ReadNode());
    expr->SetParent(this);
    targetTypeExpr.reset(reader.ReadNode());
    targetTypeExpr->SetParent(this);
}

AsNode::AsNode(const Span& span_) : Node(NodeType::asNode, span_), expr(), targetTypeExpr()
{
}

AsNode::AsNode(const Span& span_, Node* expr_, Node* targetTypeExpr_) : Node(NodeType::asNode, span_), expr(expr_), targetTypeExpr(targetTypeExpr_)
{
    expr->SetParent(this);
    targetTypeExpr->SetParent(this);
}

Node* AsNode::Clone(CloneContext& cloneContext) const
{
    return new AsNode(GetSpan(), expr->Clone(cloneContext), targetTypeExpr->Clone(cloneContext));
}

void AsNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void AsNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(expr.get());
    writer.Write(targetTypeExpr.get());
}

void AsNode::Read(AstReader& reader)
{
    Node::Read(reader);
    expr.reset(reader.ReadNode());
    expr->SetParent(this);
    targetTypeExpr.reset(reader.ReadNode());
    targetTypeExpr->SetParent(this);
}

IndexingNode::IndexingNode(const Span& span_) : Node(NodeType::indexingNode, span_), subject(), index()
{
}

IndexingNode::IndexingNode(const Span& span_, Node* subject_, Node* index_) : Node(NodeType::indexingNode, span_), subject(subject_), index(index_)
{
    subject->SetParent(this);
    index->SetParent(this);
}

Node* IndexingNode::Clone(CloneContext& cloneContext) const
{
    return new IndexingNode(GetSpan(), subject->Clone(cloneContext), index->Clone(cloneContext));
}

void IndexingNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void IndexingNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
    writer.Write(index.get());
}

void IndexingNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
    index.reset(reader.ReadNode());
    index->SetParent(this);
}

InvokeNode::InvokeNode(const Span& span_) : Node(NodeType::invokeNode, span_), subject(), arguments()
{
}

InvokeNode::InvokeNode(const Span& span_, Node* subject_) : Node(NodeType::invokeNode, span_), subject(subject_), arguments()
{
    subject->SetParent(this);
}

Node* InvokeNode::Clone(CloneContext& cloneContext) const
{
    InvokeNode* clone = new InvokeNode(GetSpan(), subject->Clone(cloneContext));
    int n = arguments.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = arguments[i];
        clone->AddArgument(argument->Clone(cloneContext));
    }
    return clone;
}

void InvokeNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void InvokeNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
    arguments.Write(writer);
}

void InvokeNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
    arguments.Read(reader);
    arguments.SetParent(this);
}

void InvokeNode::AddArgument(Node* argument)
{
    argument->SetParent(this);
    arguments.Add(argument);
}

PostfixIncrementNode::PostfixIncrementNode(const Span& span_) : Node(NodeType::postfixIncrementNode, span_), subject()
{
}

PostfixIncrementNode::PostfixIncrementNode(const Span& span_, Node* subject_) : Node(NodeType::postfixIncrementNode, span_), subject(subject_)
{
    subject->SetParent(this);
}

Node* PostfixIncrementNode::Clone(CloneContext& cloneContext) const
{
    return new PostfixIncrementNode(GetSpan(), subject->Clone(cloneContext));
}

void PostfixIncrementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void PostfixIncrementNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
}

void PostfixIncrementNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
}

PostfixDecrementNode::PostfixDecrementNode(const Span& span_) : Node(NodeType::postfixDecrementNode, span_), subject()
{
}

PostfixDecrementNode::PostfixDecrementNode(const Span& span_, Node* subject_) : Node(NodeType::postfixDecrementNode, span_), subject(subject_)
{
    subject->SetParent(this);
}

Node* PostfixDecrementNode::Clone(CloneContext& cloneContext) const
{
    return new PostfixDecrementNode(GetSpan(), subject->Clone(cloneContext));
}

void PostfixDecrementNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void PostfixDecrementNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(subject.get());
}

void PostfixDecrementNode::Read(AstReader& reader)
{
    Node::Read(reader);
    subject.reset(reader.ReadNode());
    subject->SetParent(this);
}

SizeOfNode::SizeOfNode(const Span& span_) : Node(NodeType::sizeOfNode, span_), expression()
{
}

SizeOfNode::SizeOfNode(const Span& span_, Node* expression_) : Node(NodeType::sizeOfNode, span_), expression(expression_)
{
    expression->SetParent(this);
}

Node* SizeOfNode::Clone(CloneContext& cloneContext) const
{
    return new SizeOfNode(GetSpan(), expression->Clone(cloneContext));
}

void SizeOfNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void SizeOfNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(expression.get());
}

void SizeOfNode::Read(AstReader& reader)
{
    Node::Read(reader);
    expression.reset(reader.ReadNode());
    expression->SetParent(this);
}

TypeNameNode::TypeNameNode(const Span& span_) : Node(NodeType::typeNameNode, span_), expression()
{
}

TypeNameNode::TypeNameNode(const Span& span_, Node* expression_) : Node(NodeType::typeNameNode, span_), expression(expression_)
{
    expression->SetParent(this);
}

Node* TypeNameNode::Clone(CloneContext& cloneContext) const
{
    return new TypeNameNode(GetSpan(), expression->Clone(cloneContext));
}

void TypeNameNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void TypeNameNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(expression.get());
}

void TypeNameNode::Read(AstReader& reader)
{
    Node::Read(reader);
    expression.reset(reader.ReadNode());
    expression->SetParent(this);
}

CastNode::CastNode(const Span& span_) : Node(NodeType::castNode, span_), targetTypeExpr(), sourceExpr()
{
}

CastNode::CastNode(const Span& span_, Node* targetTypeExpr_, Node* sourceExpr_) : Node(NodeType::castNode, span_), targetTypeExpr(targetTypeExpr_), sourceExpr(sourceExpr_)
{
    targetTypeExpr->SetParent(this);
    sourceExpr->SetParent(this);
}

Node* CastNode::Clone(CloneContext& cloneContext) const
{
    return new CastNode(GetSpan(), targetTypeExpr->Clone(cloneContext), sourceExpr->Clone(cloneContext));
}

void CastNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void CastNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(targetTypeExpr.get());
    writer.Write(sourceExpr.get());
}

void CastNode::Read(AstReader& reader)
{
    Node::Read(reader);
    targetTypeExpr.reset(reader.ReadNode());
    targetTypeExpr->SetParent(this);
    sourceExpr.reset(reader.ReadNode());
    sourceExpr->SetParent(this);
}

ConstructNode::ConstructNode(const Span& span_) : Node(NodeType::constructNode, span_), typeExpr(), arguments()
{
}

ConstructNode::ConstructNode(const Span& span_, Node* typeExpr_) : Node(NodeType::constructNode, span_), typeExpr(typeExpr_), arguments()
{
    typeExpr->SetParent(this);
}

Node* ConstructNode::Clone(CloneContext& cloneContext) const
{
    ConstructNode* clone = new ConstructNode(GetSpan(), typeExpr->Clone(cloneContext));
    int n = arguments.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = arguments[i];
        clone->AddArgument(argument->Clone(cloneContext));
    }
    return clone;
}

void ConstructNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void ConstructNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(typeExpr.get());
    arguments.Write(writer);
}

void ConstructNode::Read(AstReader& reader)
{
    Node::Read(reader);
    typeExpr.reset(reader.ReadNode());
    typeExpr->SetParent(this);
    arguments.Read(reader);
    arguments.SetParent(this);
}

void ConstructNode::AddArgument(Node* argument)
{
    argument->SetParent(this);
    arguments.Add(argument);
}

NewNode::NewNode(const Span& span_) : Node(NodeType::newNode, span_), typeExpr(), arguments()
{
}

NewNode::NewNode(const Span& span_, Node* typeExpr_) : Node(NodeType::newNode, span_), typeExpr(typeExpr_), arguments()
{
    typeExpr->SetParent(this);
}

Node* NewNode::Clone(CloneContext& cloneContext) const
{
    NewNode* clone = new NewNode(GetSpan(), typeExpr->Clone(cloneContext));
    int n = arguments.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* argument = arguments[i];
        clone->AddArgument(argument->Clone(cloneContext));
    }
    return clone;
}

void NewNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void NewNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(typeExpr.get());
    arguments.Write(writer);
}

void NewNode::Read(AstReader& reader)
{
    Node::Read(reader);
    typeExpr.reset(reader.ReadNode());
    typeExpr->SetParent(this);
    arguments.Read(reader);
    arguments.SetParent(this);
}

void NewNode::AddArgument(Node* argument)
{
    argument->SetParent(this);
    arguments.Add(argument);
}

ThisNode::ThisNode(const Span& span_) : Node(NodeType::thisNode, span_)
{
}

Node* ThisNode::Clone(CloneContext& cloneContext) const
{
    return new ThisNode(GetSpan());
}

void ThisNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

BaseNode::BaseNode(const Span& span_) : Node(NodeType::baseNode, span_)
{
}

Node* BaseNode::Clone(CloneContext& cloneContext) const
{
    return new BaseNode(GetSpan());
}

void BaseNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cmajor::ast
