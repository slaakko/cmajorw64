// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/Identifier.hpp>
#include <cmajor/ast/Visitor.hpp>
#include <cmajor/ast/AstWriter.hpp>
#include <cmajor/ast/AstReader.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace ast {

using namespace cmajor::unicode;

IdentifierNode::IdentifierNode(const Span& span_) : Node(NodeType::identifierNode, span_), identifier()
{
}

IdentifierNode::IdentifierNode(const Span& span_, const std::u32string& identifier_) : Node(NodeType::identifierNode, span_), identifier(identifier_)
{
}

Node* IdentifierNode::Clone(CloneContext& cloneContext) const
{
    return new IdentifierNode(GetSpan(), identifier);
}

void IdentifierNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

void IdentifierNode::Write(AstWriter& writer)
{
    Node::Write(writer);
    writer.GetBinaryWriter().Write(identifier);
}

void IdentifierNode::Read(AstReader& reader)
{
    Node::Read(reader);
    identifier = reader.GetBinaryReader().ReadUtf32String();
}

std::string IdentifierNode::ToString() const
{
    return ToUtf8(identifier);
}

} } // namespace cmajor::ast
