// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_IR_EMITTER_INCLUDED
#define CMAJOR_IR_EMITTER_INCLUDED
#include <cmajor/ast/CompileUnit.hpp>
#include <cmajor/parsing/Scanner.hpp>
#include <cmajor/ir/ValueStack.hpp>
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/DIBuilder.h>
#include <unordered_map>

namespace cmajor { namespace ir {

using cmajor::parsing::Span;
using cmajor::ast::CompileUnitNode;

typedef llvm::SmallVector<llvm::Value*, 4> ArgVector;

struct Pad
{
    Pad() : parent(nullptr), value(nullptr) {}
    Pad* parent;
    llvm::Value* value;
};

class Emitter
{
public:
    Emitter(llvm::LLVMContext& context_);
    virtual ~Emitter();
    llvm::LLVMContext& Context() { return context; }
    llvm::IRBuilder<>& Builder() { return builder; }
    llvm::Module* Module() { return module; }
    llvm::DataLayout* DataLayout() { return dataLayout; }
    llvm::DICompileUnit* DICompileUnit() { return diCompileUnit; }
    llvm::DIFile* DIFile() { return diFile; }
    llvm::DIBuilder* DIBuilder() { return diBuilder;  }
    ValueStack& Stack() { return stack; }
    void SetModule(llvm::Module* module_) { module = module_; }
    void SetDataLayout(llvm::DataLayout* dataLayout_) { dataLayout = dataLayout_; }
    void SetDICompileUnit(llvm::DICompileUnit* diCompileUnit_) { diCompileUnit = diCompileUnit_; }
    void SetDIFile(llvm::DIFile* diFile_);
    void SetCurrentCompileUnitNode(CompileUnitNode* currentCompileUnitNode_) { currentCompileUnitNode = currentCompileUnitNode_;  }
    void SetDIBuilder(llvm::DIBuilder* diBuilder_) { diBuilder = diBuilder_; }
    void SaveObjectPointer(llvm::Value* objectPointer_);
    void SetObjectPointer(llvm::Value* objectPointer_) { objectPointer = objectPointer_;  }
    llvm::Value* GetObjectPointer() { return objectPointer; }
    void SetFunction(llvm::Function* function_) { function = function_;  }
    llvm::Function* Function() { return function; }
    virtual llvm::Value* GetGlobalStringPtr(int stringId) = 0;
    virtual llvm::Value* GetGlobalWStringConstant(int stringId) = 0;
    virtual llvm::Value* GetGlobalUStringConstant(int stringId) = 0;
    virtual llvm::Value* GetGlobalUuidConstant(int uuidId) = 0;
    virtual void SetLineNumber(int32_t lineNumber) = 0;
    virtual llvm::BasicBlock* HandlerBlock() = 0;
    virtual llvm::BasicBlock* CleanupBlock() = 0;
    virtual bool NewCleanupNeeded() = 0;
    virtual void CreateCleanup() = 0;
    virtual std::string GetSourceFilePath(int32_t fileIndex) = 0;
    llvm::BasicBlock* CurrentBasicBlock() const { return currentBasicBlock; }
    void SetCurrentBasicBlock(llvm::BasicBlock* currentBasicBlock_) { currentBasicBlock = currentBasicBlock_; builder.SetInsertPoint(currentBasicBlock); }
    virtual Pad* CurrentPad() = 0;
    void SetInPrologue(bool inPrologue_) { inPrologue = inPrologue_; }
    void PushScope(llvm::DIScope* scope);
    void PopScope();
    llvm::DIScope* CurrentScope();
    int GetColumn(const Span& span) const;
    llvm::DebugLoc GetDebugLocation(const Span& span);
    void SetCurrentDebugLocation(const Span& span);
    llvm::DebugLoc GetCurrentDebugLocation() { return currentDebugLocation; }
    llvm::DIFile* GetFile(int32_t fileIndex);
    void SetCompileUnitIndex(int32_t compileUnitIndex_) { compileUnitIndex = compileUnitIndex_; }
    int32_t CompileUnitIndex() const { return compileUnitIndex; }
    llvm::DIType* GetDIType(void* symbol) const;
    void SetDIType(void* symbol, llvm::DIType* diType);
private:
    llvm::LLVMContext& context;
    llvm::IRBuilder<> builder;
    llvm::Module* module;
    llvm::DataLayout* dataLayout;
    llvm::DICompileUnit* diCompileUnit;
    llvm::DIFile* diFile;
    llvm::DIBuilder* diBuilder;
    CompileUnitNode* currentCompileUnitNode;
    int32_t compileUnitIndex;
    ValueStack stack;
    llvm::Value* objectPointer;
    llvm::Function* function;
    llvm::BasicBlock* currentBasicBlock;
    std::vector<llvm::DIScope*> scopes;
    llvm::DebugLoc currentDebugLocation;
    bool inPrologue;
    std::unordered_map<int32_t, llvm::DIFile*> fileMap;
    std::unordered_map<void*, llvm::DIType*> diTypeMap;
};

} } // namespace cmajor::ir

#endif // CMAJOR_IR_EMITTER_INCLUDED
