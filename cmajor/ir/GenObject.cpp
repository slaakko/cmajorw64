// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ir/GenObject.hpp>
#include <cmajor/ir/Emitter.hpp>

namespace cmajor { namespace ir {

GenObject::GenObject() : type(nullptr)
{
}

GenObject::~GenObject()
{
}

void LlvmValue::Load(Emitter& emitter, OperationFlags flags)
{ 
    emitter.Stack().Push(value); 
}

void LlvmValue::Store(Emitter& emitter, OperationFlags flags)
{ 
    throw std::runtime_error("cannot store to llvm value"); 
}

} } // namespace cmajor::ir