// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_CLONE_INCLUDED
#define CMAJOR_AST_CLONE_INCLUDED

namespace cmajor { namespace ast {

class CloneContext
{
public:     
    CloneContext();
    void SetInstantiateFunctionNode() { instantiateFunctionNode = true; }
    bool InstantiateFunctionNode() const { return instantiateFunctionNode; }
private:
    bool instantiateFunctionNode;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_CLONE_INCLUDED
