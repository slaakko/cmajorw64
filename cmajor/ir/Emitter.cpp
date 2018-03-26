// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ir/Emitter.hpp>
#include <cmajor/util/Path.hpp>

namespace cmajor { namespace ir {

using namespace cmajor::util;

Emitter::Emitter(llvm::LLVMContext& context_) : 
    context(context_), builder(context), module(nullptr), dataLayout(nullptr), diCompileUnit(nullptr), diFile(nullptr), diBuilder(nullptr), currentCompileUnitNode(nullptr), compileUnitIndex(-1),
    stack(), objectPointer(nullptr), function(nullptr), currentBasicBlock(nullptr), inPrologue(false)
{
}

Emitter::~Emitter()
{
}

void Emitter::SaveObjectPointer(llvm::Value* objectPointer_)
{
    if (objectPointer == nullptr)
    {
        objectPointer = objectPointer_;
    }
}

void Emitter::PushScope(llvm::DIScope* scope)
{
    scopes.push_back(scope);
}

void Emitter::PopScope()
{
    scopes.pop_back();
}

llvm::DIScope* Emitter::CurrentScope()
{
    llvm::DIScope* currentScope = diCompileUnit;
    if (!scopes.empty())
    {
        currentScope = scopes.back();
    }
    return currentScope;
}

int Emitter::GetColumn(const Span& span) const
{
    int column = 1;
    if (currentCompileUnitNode)
    {
        column = currentCompileUnitNode->GetColumn(span);
    }
    return column;
}

llvm::DebugLoc Emitter::GetDebugLocation(const Span& span) 
{
    if (!diCompileUnit || !span.Valid()) return llvm::DebugLoc();
    int column = GetColumn(span);
    return llvm::DebugLoc::get(span.LineNumber(), column, CurrentScope());
}

void Emitter::SetCurrentDebugLocation(const Span& span)
{
    if (!diCompileUnit) return;
    if (inPrologue || !span.Valid())
    {
        currentDebugLocation = llvm::DebugLoc();
        builder.SetCurrentDebugLocation(currentDebugLocation);
    }
    else
    {
        currentDebugLocation = GetDebugLocation(span);
        builder.SetCurrentDebugLocation(currentDebugLocation);
    }
}

void Emitter::SetDIFile(llvm::DIFile* diFile_)
{
    diFile = diFile_;
}

llvm::DIFile* Emitter::GetFile(int32_t fileIndex)
{
    if (fileIndex == -1)
    {
        return diFile;
    }
    auto it = fileMap.find(fileIndex);
    if (it != fileMap.cend())
    {
        return it->second;
    }
    std::string sourceFilePath = GetSourceFilePath(fileIndex);
    if (sourceFilePath.empty())
    {
        return diFile;
    }
    llvm::DIFile* file = diBuilder->createFile(Path::GetFileName(sourceFilePath), Path::GetDirectoryName(sourceFilePath));
    fileMap[fileIndex] = file;
    return file;
}

llvm::DIType* Emitter::GetDIType(void* symbol) const
{
    auto it = diTypeMap.find(symbol);
    if (it != diTypeMap.cend())
    {
        return it->second;
    }
    return nullptr;
}

void Emitter::SetDIType(void* symbol, llvm::DIType* diType)
{
    diTypeMap[symbol] = diType;
}

} } // namespace cmajor::ir
