// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundCompileUnit.hpp>

namespace cmajor { namespace binder {

BoundCompileUnit::BoundCompileUnit(Module& mod_, CompileUnitNode* compileUnitNode_) : mod(mod_), symbolTable(mod.GetSymbolTable()), compileUnitNode(compileUnitNode_)
{
}

void BoundCompileUnit::AddFileScope(FileScope* fileScope)
{
    fileScopes.push_back(std::unique_ptr<FileScope>(fileScope));
}

} } // namespace cmajor::binder
