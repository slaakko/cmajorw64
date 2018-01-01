// =================================
// Copyright (c) 2018 Seppo Laakko
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
    void SetInstantiateClassNode() { instantiateClassNode = true; }
    bool InstantiateClassNode() const { return instantiateClassNode; }
private:
    bool instantiateFunctionNode;
    bool instantiateClassNode;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_CLONE_INCLUDED
