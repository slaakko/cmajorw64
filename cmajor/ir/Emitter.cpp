// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ir/Emitter.hpp>

namespace cmajor { namespace ir {

Emitter::Emitter(llvm::LLVMContext& context_) : context(context_), builder(context), module(nullptr), stack(), objectPointer(nullptr), function(nullptr)
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

} } // namespace cmajor::ir
