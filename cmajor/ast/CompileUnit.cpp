// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/CompileUnit.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/ast/Visitor.hpp>

namespace cmajor { namespace ast {

CompileUnitNode::CompileUnitNode(const Span& span_) : Node(NodeType::compileUnitNode, span_), globalNs()
{
}

CompileUnitNode::CompileUnitNode(const Span& span_, const std::string& filePath_) : 
    Node(NodeType::compileUnitNode, span_), filePath(filePath_), globalNs(new NamespaceNode(span_, new IdentifierNode(span_, U"")))
{
}

Node* CompileUnitNode::Clone(CloneContext& cloneContext) const
{
    CompileUnitNode* clone = new CompileUnitNode(GetSpan(), filePath);
    clone->globalNs.reset(static_cast<NamespaceNode*>(globalNs->Clone(cloneContext)));
    return clone;
}

void CompileUnitNode::Accept(Visitor& visitor)
{
    visitor.Visit(*this);
}

} } // namespace cmajor::ast
