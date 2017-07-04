// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/Function.hpp>
#include <cmajor/ast/Visitor.hpp>

namespace cmajor { namespace ast {

FunctionNode::FunctionNode(const Span& span_) : Node(NodeType::functionNode, span_), specifiers(Specifiers::none), returnTypeExpr(), groupId(), parameters(), body(), bodySource()
{
}

FunctionNode::FunctionNode(const Span& span_, Specifiers specifiers_, Node* returnTypeExpr_, const std::u32string& groupId_) :
    Node(NodeType::functionNode, span_), specifiers(specifiers_), returnTypeExpr(returnTypeExpr_), groupId(groupId_), parameters(), body(), bodySource()
{
    if (returnTypeExpr)
    {
        returnTypeExpr->SetParent(this);
    }
}

Node* FunctionNode::Clone(CloneContext& cloneContext) const
{
    Node* clonedReturnTypeExpr = nullptr;
    if (returnTypeExpr)
    {
        clonedReturnTypeExpr = returnTypeExpr->Clone(cloneContext);
    }
    FunctionNode* clone = new FunctionNode(GetSpan(), specifiers, clonedReturnTypeExpr, groupId);
    int n = parameters.Count();
    for (int i = 0; i < n; ++i)
    {
        clone->AddParameter(static_cast<ParameterNode*>(parameters[i]->Clone(cloneContext)));
    }
    if (body)
    {
        clone->SetBody(static_cast<CompoundStatementNode*>(body->Clone(cloneContext)));
    }
    return clone;
}

void FunctionNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void FunctionNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(specifiers);
    bool hasReturnTypeExpr = returnTypeExpr != nullptr;
    writer.GetBinaryWriter().Write(hasReturnTypeExpr);
    if (hasReturnTypeExpr)
    {
        writer.Write(returnTypeExpr.get());
    }
    writer.GetBinaryWriter().Write(groupId);
    parameters.Write(writer);
    bool hasBody = body != nullptr;
    writer.GetBinaryWriter().Write(hasBody);
    if (hasBody)
    {
        writer.Write(body.get());
    }
    bool hasBodySource = bodySource != nullptr;
    writer.GetBinaryWriter().Write(hasBodySource);
    if (hasBodySource)
    {
        writer.Write(bodySource.get());
    }
}

void FunctionNode::Read(AstReader& reader)
{
    Node::Read(reader);
    specifiers = reader.ReadSpecifiers();
    bool hasReturnTypeExpr = reader.GetBinaryReader().ReadBool();
    if (hasReturnTypeExpr)
    {
        returnTypeExpr.reset(reader.ReadNode());
        returnTypeExpr->SetParent(this);
    }
    groupId = reader.GetBinaryReader().ReadUtf32String();
    parameters.Read(reader);
    parameters.SetParent(this);
    bool hasBody = reader.GetBinaryReader().ReadBool();
    if (hasBody)
    {
        body.reset(reader.ReadCompoundStatementNode());
        body->SetParent(this);
    }
    bool hasBodySource = reader.GetBinaryReader().ReadBool();
    if (hasBodySource)
    {
        bodySource.reset(reader.ReadCompoundStatementNode());
        bodySource->SetParent(this);
    }
}

void FunctionNode::AddParameter(ParameterNode* parameter)
{
    parameter->SetParent(this);
    parameters.Add(parameter);
}

void FunctionNode::SwitchToBody()
{
    if (bodySource && !body)
    {
        SetBody(bodySource.release());
    }
}

void FunctionNode::SetBody(CompoundStatementNode* body_)
{
    body.reset(body_);
    body->SetParent(this);
}

void FunctionNode::SetBodySource(CompoundStatementNode* bodySource_)
{
    bodySource.reset(bodySource_);
    bodySource->SetParent(this);
}

} } // namespace cmajor::ast
