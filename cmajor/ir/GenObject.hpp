// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_IR_GEN_OBJECT_INCLUDED
#define CMAJOR_IR_GEN_OBJECT_INCLUDED
#include <stdint.h>

namespace cmajor { namespace ir {

class Emitter;

enum class OperationFlags : uint8_t
{
    none = 0,
    addr = 1 << 0,
    deref = 1 << 1
};

inline OperationFlags operator|(OperationFlags left, OperationFlags right)
{
    return OperationFlags(uint8_t(left) | uint8_t(right));
}

inline OperationFlags operator&(OperationFlags left, OperationFlags right)
{
    return OperationFlags(uint8_t(left) & uint8_t(right));
}

class GenObject
{
public:
    virtual ~GenObject();
    virtual void Init(Emitter& emitter) {};
    virtual void Load(Emitter& emitter, OperationFlags flags) = 0;
    virtual void Store(Emitter& emitter, OperationFlags flags) = 0;
};

} } // namespace cmajor::ir

#endif // CMAJOR_IR_GEN_OBJECT_INCLUDED
