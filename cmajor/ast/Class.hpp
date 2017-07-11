// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_CLASS_INCLUDED
#define CMAJOR_AST_CLASS_INCLUDED
#include <cmajor/ast/Function.hpp>

namespace cmajor { namespace ast {

class ClassNode : public Node
{
public:
    ClassNode(const Span& span_);
    ClassNode(const Span& span_, Specifiers specifiers_, IdentifierNode* id_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddTemplateParameter(TemplateParameterNode* templateParameter) override;
    Specifiers GetSpecifiers() const { return specifiers; }
    IdentifierNode* Id() const { return id.get(); }
    void AddBaseClassOrInterface(Node* baseClassOrInterface);
    const WhereConstraintNode* WhereConstraint() const { return constraint.get(); }
    void SetConstraint(WhereConstraintNode* whereConstraint);
    void AddMember(Node* member);
    const NodeList<TemplateParameterNode>& TemplateParameters() const { return templateParameters; }
    const NodeList<Node>& BaseClassOrInterfaces() const { return baseClassOrInterfaces; }
    const NodeList<Node>& Members() const { return members; }
private:
    Specifiers specifiers;
    std::unique_ptr<IdentifierNode> id;
    NodeList<TemplateParameterNode> templateParameters;
    NodeList<Node> baseClassOrInterfaces;
    std::unique_ptr<WhereConstraintNode> constraint;
    NodeList<Node> members;
};

class InitializerNode : public Node
{
public:
    InitializerNode(NodeType nodeType_, const Span& span_);
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddArgument(Node* argument) override;
    const NodeList<Node>& Arguments() const { return arguments; }
private:
    NodeList<Node> arguments;
};

class ThisInitializerNode : public InitializerNode
{
public:
    ThisInitializerNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class BaseInitializerNode : public InitializerNode
{
public:
    BaseInitializerNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class MemberInitializerNode : public InitializerNode
{
public:
    MemberInitializerNode(const Span& span_);
    MemberInitializerNode(const Span& span_, IdentifierNode* memberId_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const IdentifierNode* MemberId() const { return memberId.get(); }
private:
    std::unique_ptr<IdentifierNode> memberId;
};

class StaticConstructorNode : public FunctionNode
{
public:
    StaticConstructorNode(const Span& span_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddInitializer(InitializerNode* initializer);
    const NodeList<InitializerNode>& Initializers() const { return initializers; }
private:
    NodeList<InitializerNode> initializers;
};

class ConstructorNode : public FunctionNode
{
public:
    ConstructorNode(const Span& span_);
    ConstructorNode(const Span& span_, Specifiers specifiers_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    void AddInitializer(InitializerNode* initializer);
    const NodeList<InitializerNode>& Initializers() const { return initializers; }
private:
    NodeList<InitializerNode> initializers;
};

class DestructorNode : public FunctionNode
{
public:
    DestructorNode(const Span& span_);
    DestructorNode(const Span& span_, Specifiers specifiers_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
};

class MemberFunctionNode : public FunctionNode
{
public:
    MemberFunctionNode(const Span& span_);
    MemberFunctionNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, const std::u32string& groupId_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    bool IsConst() const { return (GetSpecifiers() & Specifiers::const_) != Specifiers::none; }
    void SetConst() { SetSpecifiers(GetSpecifiers() | Specifiers::const_); }
};

class MemberVariableNode : public Node
{
public:
    MemberVariableNode(const Span& span_);
    MemberVariableNode(const Span& span_, Specifiers specifiers_, Node* typeExpr_, IdentifierNode* id_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    Specifiers GetSpecifiers() const { return specifiers; }
    Node* TypeExpr() const { return typeExpr.get(); }
    IdentifierNode* Id() const { return id.get(); }
private:
    Specifiers specifiers;
    std::unique_ptr<Node> typeExpr;
    std::unique_ptr<IdentifierNode> id;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_CLASS_INCLUDED
