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
    deref = 1 << 1,
    virtualCall = 1 << 2,
    leaveFirstArg = 1 << 3,
    functionCallFlags = leaveFirstArg
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
    GenObject();
    virtual ~GenObject();
    virtual void Load(Emitter& emitter, OperationFlags flags) = 0;
    virtual void Store(Emitter& emitter, OperationFlags flags) = 0;
    void SetType(void* type_) { type = type_; }
    void* GetType() { return type; }
private:
    void* type;
};

} } // namespace cmajor::ir

#endif // CMAJOR_IR_GEN_OBJECT_INCLUDED
