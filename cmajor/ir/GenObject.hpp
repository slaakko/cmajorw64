// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_IR_GEN_OBJECT_INCLUDED
#define CMAJOR_IR_GEN_OBJECT_INCLUDED

namespace cmajor { namespace ir {

class Emitter;

class GenObject
{
public:
    virtual ~GenObject();
    virtual void Init(Emitter& emitter) {};
    virtual void Load(Emitter& emitter) = 0;
    virtual void Store(Emitter& emitter) = 0;
};

} } // namespace cmajor::ir

#endif // CMAJOR_IR_GEN_OBJECT_INCLUDED
