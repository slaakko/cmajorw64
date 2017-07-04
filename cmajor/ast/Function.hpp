// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_FUNCTION_INCLUDED
#define CMAJOR_AST_FUNCTION_INCLUDED
#include <cmajor/ast/Specifier.hpp>
#include <cmajor/ast/Parameter.hpp>
#include <cmajor/ast/Statement.hpp>

namespace cmajor { namespace ast {

class FunctionNode : public Node
{
public:
    FunctionNode(const Span& span_);
    FunctionNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, const std::u32string& groupId_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddParameter(ParameterNode* parameter) override;
    void SwitchToBody();
    Specifiers GetSpecifiers() const { return specifiers; }
    const Node* ReturnTypeExpr() const { return returnTypeExpr.get(); }
    const std::u32string& GroupId() const { return groupId; }
    const NodeList<ParameterNode>& Parameters() const { return parameters; }
    const CompoundStatementNode* Body() const { return body.get(); }
    void SetBody(CompoundStatementNode* body_);
    const CompoundStatementNode* BodySource() const { return bodySource.get(); }
    void SetBodySource(CompoundStatementNode* bodySource_);
private:
    Specifiers specifiers;
    std::unique_ptr<Node> returnTypeExpr;
    std::u32string groupId;
    NodeList<ParameterNode> parameters;
    std::unique_ptr<CompoundStatementNode> body;
    std::unique_ptr<CompoundStatementNode> bodySource;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_FUNCTION_INCLUDED
