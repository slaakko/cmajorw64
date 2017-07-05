// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_CONCEPT_INCLUDED
#define CMAJOR_AST_CONCEPT_INCLUDED
#include <cmajor/ast/Parameter.hpp>
#include <cmajor/ast/NodeList.hpp>

namespace cmajor { namespace ast {

class ConstraintNode : public Node
{
public:
    ConstraintNode(NodeType nodeType_, const Span& span_);
    bool IsConstraintNode() const override { return true; }
};

class BinaryConstraintNode : public ConstraintNode
{
public:
    BinaryConstraintNode(NodeType nodeType_, const Span& span_);
    BinaryConstraintNode(NodeType nodeType_, const Span& span_, ConstraintNode* left_, ConstraintNode* right_);
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const ConstraintNode* Left() const { return left.get(); }
    const ConstraintNode* Right() const { return right.get(); }
private:
    std::unique_ptr<ConstraintNode> left;
    std::unique_ptr<ConstraintNode> right;
};

class DisjunctiveConstraintNode : public BinaryConstraintNode
{
public:
    DisjunctiveConstraintNode(const Span& span_);
    DisjunctiveConstraintNode(const Span& span_, ConstraintNode* left_, ConstraintNode* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class ConjunctiveConstraintNode : public BinaryConstraintNode
{
public:
    ConjunctiveConstraintNode(const Span& span_);
    ConjunctiveConstraintNode(const Span& span_, ConstraintNode* left_, ConstraintNode* right_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class WhereConstraintNode : public ConstraintNode
{
public:
    WhereConstraintNode(const Span& span_);
    WhereConstraintNode(const Span& span_, ConstraintNode* constraint_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const ConstraintNode* Constraint() const { return constraint.get(); }
private:
    std::unique_ptr<ConstraintNode> constraint;
};

class PredicateConstraintNode : public ConstraintNode
{
public:
    PredicateConstraintNode(const Span& span_);
    PredicateConstraintNode(const Span& span_, Node* invokeExpr_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* InvokeExpr() const { return invokeExpr.get(); }
private:
    std::unique_ptr<Node> invokeExpr;
};

class IsConstraintNode : public ConstraintNode
{
public:
    IsConstraintNode(const Span& span_);
    IsConstraintNode(const Span& span_, Node* typeExpr_, Node* conceptOrTypeName_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* TypeExpr() const { return typeExpr.get(); }
    const Node* ConceptOrTypeName() const { return conceptOrTypeName.get(); }
private:
    std::unique_ptr<Node> typeExpr;
    std::unique_ptr<Node> conceptOrTypeName;
};

class MultiParamConstraintNode : public ConstraintNode
{
public:
    MultiParamConstraintNode(const Span& span_);
    MultiParamConstraintNode(const Span& span_, IdentifierNode* conceptId_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const IdentifierNode* ConceptId() const { return conceptId.get(); }
    const NodeList<Node>& TypeExprs() const { return typeExprs; }
    void AddTypeExpr(Node* typeExpr);
private:
    std::unique_ptr<IdentifierNode> conceptId;
    NodeList<Node> typeExprs;
};

class TypeNameConstraintNode : public ConstraintNode
{
public:
    TypeNameConstraintNode(const Span& span_);
    TypeNameConstraintNode(const Span& span_, Node* typeId_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* TypeId() const { return typeId.get(); }
private:
    std::unique_ptr<Node> typeId;
};

class SignatureConstraintNode : public ConstraintNode
{
public:
    SignatureConstraintNode(NodeType nodeType_, const Span& span_);
};

class ConstructorConstraintNode : public SignatureConstraintNode
{
public:
    ConstructorConstraintNode(const Span& span_);
    ConstructorConstraintNode(const Span& span_, IdentifierNode* typeParamId_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddParameter(ParameterNode* parameter) override;
    const IdentifierNode* TypeParamId() const { return typeParamId.get(); }
    const NodeList<ParameterNode>& Parameters() const { return parameters; }
private:
    std::unique_ptr<IdentifierNode> typeParamId;
    NodeList<ParameterNode> parameters;
};

class DestructorConstraintNode : public SignatureConstraintNode
{
public:
    DestructorConstraintNode(const Span& span_);
    DestructorConstraintNode(const Span& span_, IdentifierNode* typeParamId_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
private:
    std::unique_ptr<IdentifierNode> typeParamId;
};

class MemberFunctionConstraintNode : public SignatureConstraintNode
{
public:
    MemberFunctionConstraintNode(const Span& span_);
    MemberFunctionConstraintNode(const Span& span_, Node* returnTypeExpr_, IdentifierNode* typeParamId_, const std::u32string& groupId_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddParameter(ParameterNode* parameter) override;
    const Node* ReturnTypeExpr() const { return returnTypeExpr.get(); }
    const IdentifierNode* TypeParamId() const { return typeParamId.get(); }
    const std::u32string& GroupId() const { return groupId; }
    const NodeList<ParameterNode>& Parameters() const { return parameters; }
private:
    std::unique_ptr<Node> returnTypeExpr;
    std::unique_ptr<IdentifierNode> typeParamId;
    std::u32string groupId;
    NodeList<ParameterNode> parameters;
};

class FunctionConstraintNode : public SignatureConstraintNode
{
public:
    FunctionConstraintNode(const Span& span_);
    FunctionConstraintNode(const Span& span_, Node* returnTypeExpr_, const std::u32string& groupId_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddParameter(ParameterNode* parameter) override;
    const Node* ReturnTypeExpr() const { return returnTypeExpr.get(); }
    const std::u32string& GroupId() const { return groupId; }
    const NodeList<ParameterNode>& Parameters() const { return parameters; }
private:
    std::unique_ptr<Node> returnTypeExpr;
    std::u32string groupId;
    NodeList<ParameterNode> parameters;
};

class AxiomStatementNode : public Node
{
public:
    AxiomStatementNode(const Span& span_);
    AxiomStatementNode(const Span& span_, Node* expression_, const std::u32string& text_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Expression() const { return expression.get(); }
    const std::u32string& Text() const { return text; }
private:
    std::unique_ptr<Node> expression;
    std::u32string text;
};

class AxiomNode : public Node
{
public:
    AxiomNode(const Span& span_);
    AxiomNode(const Span& span_, IdentifierNode* id_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddParameter(ParameterNode* parameter) override;
    void AddStatement(AxiomStatementNode* statement);
    const IdentifierNode* Id() const { return id.get(); }
    const NodeList<ParameterNode>& Parameters() const { return parameters; }
    const NodeList<AxiomStatementNode>& Statements() const { return statements; }
private:
    std::unique_ptr<IdentifierNode> id;
    NodeList<ParameterNode> parameters;
    NodeList<AxiomStatementNode> statements;
};

class ConceptIdNode : public Node
{
public:
    ConceptIdNode(const Span& span_);
    ConceptIdNode(const Span& span_, IdentifierNode* id_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const IdentifierNode* Id() const { return id.get(); }
    const NodeList<Node>& TypeParameters() const { return typeParameters; }
    void AddTypeParameter(Node* typeParameter);
private:
    std::unique_ptr<IdentifierNode> id;
    NodeList<Node> typeParameters;
};

class ConceptNode : public Node
{
public:
    ConceptNode(const Span& span_);
    ConceptNode(const Span& span_, Specifiers specifiers_, IdentifierNode* id_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const IdentifierNode* Id() const { return id.get(); }
    const NodeList<IdentifierNode>& TypeParameters() const { return typeParameters; }
    void AddTypeParameter(IdentifierNode* typeParameter);
    void SetRefinement(ConceptIdNode* refinement_);
    void AddConstraint(ConstraintNode* constraint);
    void AddAxiom(AxiomNode* axiom);
private:
    Specifiers specifiers;
    std::unique_ptr<IdentifierNode> id;
    NodeList<IdentifierNode> typeParameters;
    std::unique_ptr<ConceptIdNode> refinement;
    NodeList<ConstraintNode> constraints;
    NodeList<AxiomNode> axioms;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_CONCEPT_INCLUDED
