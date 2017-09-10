// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_IR_EMITTER_INCLUDED
#define CMAJOR_IR_EMITTER_INCLUDED
#include <cmajor/ir/ValueStack.hpp>
#include <llvm/IR/IRBuilder.h>

namespace cmajor { namespace ir {

typedef llvm::SmallVector<llvm::Value*, 4> ArgVector;

class Emitter
{
public:
    Emitter(llvm::LLVMContext& context_);
    virtual ~Emitter();
    llvm::LLVMContext& Context() { return context; }
    llvm::IRBuilder<>& Builder() { return builder; }
    llvm::Module* Module() { return module; }
    ValueStack& Stack() { return stack; }
    void SetModule(llvm::Module* module_) { module = module_; }
    void SaveObjectPointer(llvm::Value* objectPointer_);
    void ResetObjectPointer() { objectPointer = nullptr; }
    llvm::Value* GetObjectPointer() { return objectPointer; }
    void SetFunction(llvm::Function* function_) { function = function_;  }
    llvm::Function* Function() { return function; }
    virtual llvm::Value* GetGlobalStringPtr(int stringId) = 0;
    virtual llvm::Value* GetGlobalWStringConstant(int stringId) = 0;
    virtual llvm::Value* GetGlobalUStringConstant(int stringId) = 0;
    virtual void SetLineNumber(int32_t lineNumber) = 0;
    virtual llvm::BasicBlock* HandlerBlock() = 0;
    llvm::BasicBlock* CurrentBasicBlock() const { return currentBasicBlock; }
    void SetCurrentBasicBlock(llvm::BasicBlock* currentBasicBlock_) { currentBasicBlock = currentBasicBlock_; builder.SetInsertPoint(currentBasicBlock); }
    virtual llvm::Value* CurrentPad() = 0;
private:
    llvm::LLVMContext& context;
    llvm::IRBuilder<> builder;
    llvm::Module* module;
    ValueStack stack;
    llvm::Value* objectPointer;
    llvm::Function* function;
    llvm::BasicBlock* currentBasicBlock;
};

} } // namespace cmajor::ir

#endif // CMAJOR_IR_EMITTER_INCLUDED
