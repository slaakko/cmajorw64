// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_ENUMERATION_INCLUDED
#define CMAJOR_AST_ENUMERATION_INCLUDED
#include <cmajor/ast/Function.hpp>
#include <cmajor/ast/NodeList.hpp>
#include <cmajor/ast/Specifier.hpp>

namespace cmajor { namespace ast {

class IdentifierNode;
class EnumConstantNode;

class EnumTypeNode : public Node
{
public:
    EnumTypeNode(const Span& span_);
    EnumTypeNode(const Span& span_, Specifiers specifiers_, IdentifierNode* id_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddConstant(EnumConstantNode* constant);
    EnumConstantNode* GetLastConstant() const;
    Specifiers GetSpecifiers() const { return specifiers; }
    IdentifierNode* Id() const { return id.get(); }
    void SetUnderlyingType(Node* underlyingType_);
    Node* GetUnderlyingType() const { return underlyingType.get(); }
    const NodeList<EnumConstantNode>& Constants() const { return constants; }
private:
    Specifiers specifiers;
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<Node> underlyingType;
    NodeList<EnumConstantNode> constants;
};

class EnumConstantNode : public Node
{
public:
    EnumConstantNode(const Span& span_);
    EnumConstantNode(const Span& span_, IdentifierNode* id_, Node* value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    Node* GetValue() const { return value.get(); }
    IdentifierNode* Id() const { return id.get(); }
private:
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<Node> value;
};

Node* MakeNextEnumConstantValue(const Span& span, EnumTypeNode* enumType);

} } // namespace cmajor::ast

#endif // CMAJOR_AST_ENUMERATION_INCLUDED
