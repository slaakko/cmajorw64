// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
#define CMAJOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
#include <cmajor/symbols/Module.hpp>
#include <cmajor/ast/CompileUnit.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::symbols;

class BoundCompileUnit
{
public:
    BoundCompileUnit(Module& mod_, CompileUnitNode* compileUnitNode_);
    Module& GetModule() { return mod; }
    SymbolTable& GetSymbolTable() { return symbolTable; }
    CompileUnitNode* GetCompileUnitNode() const { return compileUnitNode; }
    void AddFileScope(FileScope* fileScope);
    FileScope* FirstFileScope() const { Assert(!fileScopes.empty(), "file scopes empty");  return fileScopes.front().get(); }
    const std::vector<std::unique_ptr<FileScope>>& FileScopes() const { return fileScopes; }
private:
    Module& mod;
    SymbolTable& symbolTable;
    CompileUnitNode* compileUnitNode;
    std::vector<std::unique_ptr<FileScope>> fileScopes;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
