// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/Parameter.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/ast/Visitor.hpp>
#include <cmajor/ast/AstWriter.hpp>
#include <cmajor/ast/AstReader.hpp>

namespace cmajor { namespace ast {

ParameterNode::ParameterNode(const Span& span_) : Node(NodeType::parameterNode, span_), typeExpr(), id()
{
}

ParameterNode::ParameterNode(const Span& span_, Node* typeExpr_, IdentifierNode* id_) : Node(NodeType::parameterNode, span_), typeExpr(typeExpr_), id(id_)
{
    typeExpr->SetParent(this);
    id->SetParent(this);
}

Node* ParameterNode::Clone(CloneContext& cloneContext) const
{
    return new ParameterNode(GetSpan(), typeExpr->Clone(cloneContext), static_cast<IdentifierNode*>(id->Clone(cloneContext)));
}

void ParameterNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void ParameterNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(typeExpr.get());
    writer.Write(id.get());
}

void ParameterNode::Read(AstReader& reader)
{
    Node::Read(reader);
    typeExpr.reset(reader.ReadNode());
    typeExpr->SetParent(this);
    id.reset(reader.ReadIdentifierNode());
    id->SetParent(this);
}

} } // namespace cmajor::ast
