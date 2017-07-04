// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_BASIC_TYPE_INCLUDED
#define CMAJOR_AST_BASIC_TYPE_INCLUDED
#include <cmajor/ast/Node.hpp>

namespace cmajor { namespace ast {

class BoolNode : public Node
{
public:
    BoolNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class SByteNode : public Node
{
public:
    SByteNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ByteNode : public Node
{
public:
    ByteNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    bool IsUnsignedTypeNode() const override { return true; }
};

class ShortNode : public Node
{
public:
    ShortNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class UShortNode : public Node
{
public:
    UShortNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    bool IsUnsignedTypeNode() const override { return true; }
};

class IntNode : public Node
{
public:
    IntNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class UIntNode : public Node
{
public:
    UIntNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    bool IsUnsignedTypeNode() const override { return true; }
};

class LongNode : public Node
{
public:
    LongNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ULongNode : public Node
{
public:
    ULongNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    bool IsUnsignedTypeNode() const override { return true; }
};

class FloatNode : public Node
{
public:
    FloatNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class DoubleNode : public Node
{
public:
    DoubleNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class CharNode : public Node
{
public:
    CharNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class WCharNode : public Node
{
public:
    WCharNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class UCharNode : public Node
{
public:
    UCharNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class VoidNode : public Node
{
public:
    VoidNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_BASIC_TYPE_INCLUDED
