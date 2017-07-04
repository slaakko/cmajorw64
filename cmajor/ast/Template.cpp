// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/Template.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/ast/Visitor.hpp>

namespace cmajor { namespace ast {

TemplateIdNode::TemplateIdNode(const Span& span_) : Node(NodeType::templateIdNode, span_)
{
}

TemplateIdNode::TemplateIdNode(const Span& span_, Node* primary_) : Node(NodeType::templateIdNode, span_), primary(primary_)
{
    primary->SetParent(this);
}

Node* TemplateIdNode::Clone(CloneContext& cloneContext) const
{
    TemplateIdNode* clone = new TemplateIdNode(GetSpan(), primary->Clone(cloneContext));
    int n = templateArguments.Count();
    for (int i = 0; i < n; ++i)
    {
        Node* templateArgument = templateArguments[i];
        clone->AddTemplateArgument(templateArgument->Clone(cloneContext));
    }
    return clone;
}

void TemplateIdNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void TemplateIdNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(primary.get());
    templateArguments.Write(writer);
}

void TemplateIdNode::Read(AstReader& reader)
{
    Node::Read(reader);
    primary.reset(reader.ReadNode());
    primary->SetParent(this);
    templateArguments.Read(reader);
    templateArguments.SetParent(this);
}

void TemplateIdNode::AddTemplateArgument(Node* templateArgument)
{
    templateArgument->SetParent(this);
    templateArguments.Add(templateArgument);
}

TemplateParameterNode::TemplateParameterNode(const Span& span_) : Node(NodeType::templateParameterNode, span_), id()
{
}

TemplateParameterNode::TemplateParameterNode(const Span& span_, IdentifierNode* id_) : Node(NodeType::templateParameterNode, span_), id(id_)
{
    id->SetParent(this);
}

Node* TemplateParameterNode::Clone(CloneContext& cloneContext) const
{
    return new TemplateParameterNode(GetSpan(), static_cast<IdentifierNode*>(id->Clone(cloneContext)));
}

void TemplateParameterNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void TemplateParameterNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(id.get());
}

void TemplateParameterNode::Read(AstReader& reader)
{
    Node::Read(reader);
    id.reset(reader.ReadIdentifierNode());
    id->SetParent(this);
}

} } // namespace cmajor::ast
