// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_COMPILE_UNIT_INCLUDED
#define CMAJOR_AST_COMPILE_UNIT_INCLUDED
#include <cmajor/ast/Namespace.hpp>

namespace cmajor { namespace ast {

class CompileUnitNode : public Node
{
public:
    CompileUnitNode(const Span& span_);
    CompileUnitNode(const Span& span_, const std::string& filePath_);
    Node* Clone(CloneContext& cloneContext) const override;
    void Accept(Visitor& visitor) override;
    const std::string& FilePath() const { return filePath; }
    const NamespaceNode* GlobalNs() const { return globalNs.get(); }
    NamespaceNode* GlobalNs() { return globalNs.get(); }
private:
    std::string filePath;
    std::unique_ptr<NamespaceNode> globalNs;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_COMPILE_UNIT_INCLUDED
