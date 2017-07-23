// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ir/Emitter.hpp>

namespace cmajor { namespace ir {

Emitter::Emitter(llvm::LLVMContext& context_) : context(context_), builder(context), module(nullptr), stack()
{
}

Emitter::~Emitter()
{
}

} } // namespace cmajor::ir
