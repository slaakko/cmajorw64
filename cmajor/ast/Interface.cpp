// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/Interface.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/ast/Visitor.hpp>

namespace cmajor { namespace ast {

InterfaceNode::InterfaceNode(const Span& span_) : Node(NodeType::interfaceNode, span_), specifiers(), id(), members()
{
}

InterfaceNode::InterfaceNode(const Span& span_, Specifiers specifiers_, IdentifierNode* id_) : Node(NodeType::interfaceNode, span_), specifiers(specifiers_), id(id_), members()
{
    id->SetParent(this);
}

Node* InterfaceNode::Clone(CloneContext& cloneContext) const
{
    InterfaceNode* clone = new InterfaceNode(GetSpan(), specifiers, static_cast<IdentifierNode*>(id->Clone(cloneContext)));
    int n = members.Count();
    for (int i = 0; i < n; ++i)
    {
        clone->AddMember(members[i]->Clone(cloneContext));
    }
    return clone;
}

void InterfaceNode::Accept(Visitor& visitor) 
{
    visitor.Visit(*this);
}

void InterfaceNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.Write(id.get());
    members.Write(writer);
}

void InterfaceNode::Read(AstReader& reader)
{
    Node::Read(reader);
    id.reset(reader.ReadIdentifierNode());
    id->SetParent(this);
    members.Read(reader);
    members.SetParent(this);
}

void InterfaceNode::AddMember(Node* member)
{
    member->SetParent(this);
    members.Add(member);
}

} } // namespace cmajor::ast
