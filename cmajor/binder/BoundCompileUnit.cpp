// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/BoundNodeVisitor.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/util/Path.hpp>
#include <boost/filesystem.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::util;

BoundCompileUnit::BoundCompileUnit(Module& module_, CompileUnitNode* compileUnitNode_) : 
    BoundNode(Span(), BoundNodeType::boundCompileUnit), module(module_), symbolTable(module.GetSymbolTable()), compileUnitNode(compileUnitNode_), hasGotos(false)
{
    boost::filesystem::path fileName = boost::filesystem::path(compileUnitNode->FilePath()).filename();
    boost::filesystem::path directory = module.DirectoryPath();
    boost::filesystem::path llfp = (directory / fileName).replace_extension(".ll");
    boost::filesystem::path optllfp = (directory / fileName).replace_extension(".opt.ll");
    boost::filesystem::path objfp = (directory / fileName).replace_extension(".obj");
    llFilePath = GetFullPath(llfp.generic_string());
    optLLFilePath = GetFullPath(optllfp.generic_string());
    objectFilePath = GetFullPath(objfp.generic_string());
}

void BoundCompileUnit::Load(Emitter& emitter)
{
    throw Exception("cannot load from compile unit", GetSpan());
}

void BoundCompileUnit::Store(Emitter& emitter)
{
    throw Exception("cannot store to compile unit", GetSpan());
}

void BoundCompileUnit::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

void BoundCompileUnit::AddFileScope(FileScope* fileScope)
{
    fileScopes.push_back(std::unique_ptr<FileScope>(fileScope));
}

void BoundCompileUnit::AddBoundNode(std::unique_ptr<BoundNode>&& boundNode)
{
    boundNodes.push_back(std::move(boundNode));
}

FunctionSymbol* BoundCompileUnit::GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType)
{
    return symbolTable.GetConversion(sourceType, targetType);
}

} } // namespace cmajor::binder
