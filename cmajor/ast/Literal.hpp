// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_LITERAL_INCLUDED
#define CMAJOR_AST_LITERAL_INCLUDED
#include <cmajor/ast/Node.hpp>
#include <cmajor/ast/NodeList.hpp>

namespace cmajor { namespace ast {

Node* CreateIntegerLiteralNode(const Span& span, uint64_t value, bool unsignedSuffix);
Node* CreateFloatingLiteralNode(const Span& span, double value, bool float_);

class BooleanLiteralNode : public Node
{
public:
    BooleanLiteralNode(const Span& span_);
    BooleanLiteralNode(const Span& span_, bool value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    bool Value() const { return value; }
private:
    bool value;
};

class SByteLiteralNode : public Node
{
public:
    SByteLiteralNode(const Span& span_);
    SByteLiteralNode(const Span& span_, int8_t value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    int8_t Value() const { return value; }
private:
    int8_t value;
};

class ByteLiteralNode : public Node
{
public:
    ByteLiteralNode(const Span& span_);
    ByteLiteralNode(const Span& span_, uint8_t value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    uint8_t Value() const { return value; }
private:
    uint8_t value;
};

class ShortLiteralNode : public Node
{
public:
    ShortLiteralNode(const Span& span_);
    ShortLiteralNode(const Span& span_, int16_t value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    int16_t Value() const { return value; }
private:
    int16_t value;
};

class UShortLiteralNode : public Node
{
public:
    UShortLiteralNode(const Span& span_);
    UShortLiteralNode(const Span& span_, uint16_t value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    uint16_t Value() const { return value; }
private:
    uint16_t value;
};

class IntLiteralNode : public Node
{
public:
    IntLiteralNode(const Span& span_);
    IntLiteralNode(const Span& span_, int32_t value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    int32_t Value() const { return value; }
private:
    int32_t value;
};

class UIntLiteralNode : public Node
{
public:
    UIntLiteralNode(const Span& span_);
    UIntLiteralNode(const Span& span_, uint32_t value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    uint32_t Value() const { return value; }
private:
    uint32_t value;
};

class LongLiteralNode : public Node
{
public:
    LongLiteralNode(const Span& span_);
    LongLiteralNode(const Span& span_, int64_t value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    int64_t Value() const { return value; }
private:
    int64_t value;
};

class ULongLiteralNode : public Node
{
public:
    ULongLiteralNode(const Span& span_);
    ULongLiteralNode(const Span& span_, uint64_t value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    uint64_t Value() const { return value; }
private:
    uint64_t value;
};

class FloatLiteralNode : public Node
{
public:
    FloatLiteralNode(const Span& span_);
    FloatLiteralNode(const Span& span_, float value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    float Value() const { return value; }
private:
    float value;
};

class DoubleLiteralNode : public Node
{
public:
    DoubleLiteralNode(const Span& span_);
    DoubleLiteralNode(const Span& span_, double value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    double Value() const { return value; }
private:
    double value;
};

class CharLiteralNode : public Node
{
public:
    CharLiteralNode(const Span& span_);
    CharLiteralNode(const Span& span_, char value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    char Value() const { return value; }
private:
    char value;
};

class WCharLiteralNode : public Node
{
public:
    WCharLiteralNode(const Span& span_);
    WCharLiteralNode(const Span& span_, char16_t value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    char16_t Value() const { return value; }
private:
    char16_t value;
};

class UCharLiteralNode : public Node
{
public:
    UCharLiteralNode(const Span& span_);
    UCharLiteralNode(const Span& span_, char32_t value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    char32_t Value() const { return value; }
private:
    char32_t value;
};

class StringLiteralNode : public Node
{
public:
    StringLiteralNode(const Span& span_);
    StringLiteralNode(const Span& span_, const std::string& value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    const std::string& Value() const { return value; }
private:
    std::string value;
};

class WStringLiteralNode : public Node
{
public:
    WStringLiteralNode(const Span& span_);
    WStringLiteralNode(const Span& span_, const std::u16string& value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    const std::u16string& Value() const { return value; }
private:
    std::u16string value;
};

class UStringLiteralNode : public Node
{
public:
    UStringLiteralNode(const Span& span_);
    UStringLiteralNode(const Span& span_, const std::u32string& value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override;
    const std::u32string& Value() const { return value; }
private:
    std::u32string value;
};

class NullLiteralNode : public Node
{
public:
    NullLiteralNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    std::string ToString() const override { return "null"; }
};

class ArrayLiteralNode : public Node
{
public:
    ArrayLiteralNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    std::string ToString() const override { return "array"; }
    void AddValue(Node* value);
    const NodeList<Node>& Values() const { return values; }
    NodeList<Node>& Values() { return values; }
private:
    NodeList<Node> values;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_LITERAL_INCLUDED
