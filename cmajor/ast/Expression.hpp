// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_EXPRESSION_INCLUDED
#define CMAJOR_AST_EXPRESSION_INCLUDED
#include <cmajor/ast/Node.hpp>
#include <cmajor/ast/NodeList.hpp>

namespace cmajor { namespace ast {

class DotNode : public Node
{
public:
    DotNode(const Span& span_);
    DotNode(const Span& span_, Node* subject_, IdentifierNode* memberId_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Subject() const { return subject.get(); }
    const IdentifierNode* MemberId() const { return memberId.get(); }
private:
    std::unique_ptr<Node> subject;
    std::unique_ptr<IdentifierNode> memberId;
};

class ArrowNode : public Node
{
public:
    ArrowNode(const Span& span_);
    ArrowNode(const Span& span_, Node* subject_, IdentifierNode* memberId_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Subject() const { return subject.get(); }
    const IdentifierNode* MemberId() const { return memberId.get(); }
private:
    std::unique_ptr<Node> subject;
    std::unique_ptr<IdentifierNode> memberId;
};

class DisjunctionNode : public BinaryNode
{
public:
    DisjunctionNode(const Span& span_);
    DisjunctionNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ConjunctionNode : public BinaryNode
{
public:
    ConjunctionNode(const Span& span_);
    ConjunctionNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class BitOrNode : public BinaryNode
{
public:
    BitOrNode(const Span& span_);
    BitOrNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class BitXorNode : public BinaryNode
{
public:
    BitXorNode(const Span& span_);
    BitXorNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class BitAndNode : public BinaryNode
{
public:
    BitAndNode(const Span& span_);
    BitAndNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class EqualNode : public BinaryNode
{
public:
    EqualNode(const Span& span_);
    EqualNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class NotEqualNode : public BinaryNode
{
public:
    NotEqualNode(const Span& span_);
    NotEqualNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class LessNode : public BinaryNode
{
public:
    LessNode(const Span& span_);
    LessNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class GreaterNode : public BinaryNode
{
public:
    GreaterNode(const Span& span_);
    GreaterNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class LessOrEqualNode : public BinaryNode
{
public:
    LessOrEqualNode(const Span& span_);
    LessOrEqualNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class GreaterOrEqualNode : public BinaryNode
{
public:
    GreaterOrEqualNode(const Span& span_);
    GreaterOrEqualNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ShiftLeftNode : public BinaryNode
{
public:
    ShiftLeftNode(const Span& span_);
    ShiftLeftNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ShiftRightNode : public BinaryNode
{
public:
    ShiftRightNode(const Span& span_);
    ShiftRightNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class AddNode : public BinaryNode
{
public:
    AddNode(const Span& span_);
    AddNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class SubNode : public BinaryNode
{
public:
    SubNode(const Span& span_);
    SubNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class MulNode : public BinaryNode
{
public:
    MulNode(const Span& span_);
    MulNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class DivNode : public BinaryNode
{
public:
    DivNode(const Span& span_);
    DivNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class RemNode : public BinaryNode
{
public:
    RemNode(const Span& span_);
    RemNode(const Span& span_, Node* left_, Node* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class NotNode : public Node
{
public:
    NotNode(const Span& span_);
    NotNode(const Span& span_, Node* subject_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Subject() const { return subject.get(); }
private:
    std::unique_ptr<Node> subject;
};

class UnaryPlusNode : public Node
{
public:
    UnaryPlusNode(const Span& span_);
    UnaryPlusNode(const Span& span_, Node* subject_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Subject() const { return subject.get(); }
private:
    std::unique_ptr<Node> subject;
};

class UnaryMinusNode : public Node
{
public:
    UnaryMinusNode(const Span& span_);
    UnaryMinusNode(const Span& span_, Node* subject_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Subject() const { return subject.get(); }
private:
    std::unique_ptr<Node> subject;
};

class ComplementNode : public Node
{
public:
    ComplementNode(const Span& span_);
    ComplementNode(const Span& span_, Node* subject_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Subject() const { return subject.get(); }
private:
    std::unique_ptr<Node> subject;
};

class DerefNode : public Node
{
public:
    DerefNode(const Span& span_);
    DerefNode(const Span& span_, Node* subject_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Subject() const { return subject.get(); }
private:
    std::unique_ptr<Node> subject;
};

class AddrOfNode : public Node
{
public:
    AddrOfNode(const Span& span_);
    AddrOfNode(const Span& span_, Node* subject_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Subject() const { return subject.get(); }
private:
    std::unique_ptr<Node> subject;
};

class IsNode : public Node
{
public:
    IsNode(const Span& span_);
    IsNode(const Span& span_, Node* expr_, Node* targetTypeExpr_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Expr() const { return expr.get(); }
    const Node* TargetTypeExpr() const { return targetTypeExpr.get();  }
private:
    std::unique_ptr<Node> expr;
    std::unique_ptr<Node> targetTypeExpr;
};

class AsNode : public Node
{
public:
    AsNode(const Span& span_);
    AsNode(const Span& span_, Node* expr_, Node* targetTypeExpr_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Expr() const { return expr.get(); }
    const Node* TargetTypeExpr() const { return targetTypeExpr.get(); }
private:
    std::unique_ptr<Node> expr;
    std::unique_ptr<Node> targetTypeExpr;
};

class IndexingNode : public Node
{
public:
    IndexingNode(const Span& span_);
    IndexingNode(const Span& span_, Node* subject_, Node* index_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Subject() const { return subject.get(); }
    const Node* Index() const { return index.get(); }
private:
    std::unique_ptr<Node> subject;
    std::unique_ptr<Node> index;
};

class InvokeNode : public Node
{
public:
    InvokeNode(const Span& span_);
    InvokeNode(const Span& span_, Node* subject_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddArgument(Node* argument) override;
    const Node* Subject() const { return subject.get(); }
    const NodeList<Node>& Arguments() const { return arguments; }
private:
    std::unique_ptr<Node> subject;
    NodeList<Node> arguments;
};

class SizeOfNode : public Node
{
public:
    SizeOfNode(const Span& span_);
    SizeOfNode(const Span& span_, Node* expression_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Expression() const { return expression.get(); }
private:
    std::unique_ptr<Node> expression;
};

class TypeNameNode : public Node
{
public:
    TypeNameNode(const Span& span_);
    TypeNameNode(const Span& span_, Node* expression_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Expression() const { return expression.get(); }
private:
    std::unique_ptr<Node> expression;
};

class CastNode : public Node
{
public:
    CastNode(const Span& span_);
    CastNode(const Span& span_, Node* targetTypeExpr_, Node* sourceExpr_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* TargetTypeExpr() const { return targetTypeExpr.get(); }
    const Node* SourceExpr() const { return sourceExpr.get(); }
private:
    std::unique_ptr<Node> targetTypeExpr;
    std::unique_ptr<Node> sourceExpr;
};

class ConstructNode : public Node
{
public:
    ConstructNode(const Span& span_);
    ConstructNode(const Span& span_, Node* typeExpr_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddArgument(Node* argument) override;
    const Node* TypeExpr() const { return typeExpr.get(); }
    const NodeList<Node>& Arguments() const { return arguments; }
private:
    std::unique_ptr<Node> typeExpr;
    NodeList<Node> arguments;
};

class NewNode : public Node
{
public:
    NewNode(const Span& span_);
    NewNode(const Span& span_, Node* typeExpr_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddArgument(Node* argument) override;
    const Node* TypeExpr() const { return typeExpr.get(); }
    const NodeList<Node>& Arguments() const { return arguments; }
private:
    std::unique_ptr<Node> typeExpr;
    NodeList<Node> arguments;
};

class ThisNode : public Node
{
public:
    ThisNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class BaseNode : public Node
{
public:
    BaseNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_EXPRESSION_INCLUDED
