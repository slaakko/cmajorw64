// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_CONSTANT_INCLUDED
#define CMAJOR_AST_CONSTANT_INCLUDED
#include <cmajor/ast/Node.hpp>
#include <cmajor/ast/Specifier.hpp>

namespace cmajor { namespace ast {

class IdentifierNode;

class ConstantNode : public Node
{
public:
    ConstantNode(const Span& span_);
    ConstantNode(const Span& span_, Specifiers specifiers_, Node* typeExpr_, IdentifierNode* id_, Node* value_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    Specifiers GetSpecifiers() const { return specifiers; }
    const Node* TypeExpr() const { return typeExpr.get(); }
    const IdentifierNode* Id() const { return id.get(); }
    const Node* Value() const { return value.get(); }
private:
    Specifiers specifiers;
    std::unique_ptr<Node> typeExpr;
    std::unique_ptr<IdentifierNode> id;
    std::unique_ptr<Node> value;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_CONSTANT_INCLUDED
