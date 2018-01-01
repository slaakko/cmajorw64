// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_EMITTER_EMITTER_INCLUDED
#define CMAJOR_EMITTER_EMITTER_INCLUDED
#include <cmajor/binder/BoundCompileUnit.hpp>

namespace cmajor { namespace emitter {

using namespace cmajor::binder;

class EmittingContextImpl;

class EmittingContext
{
public:
    EmittingContext();
    ~EmittingContext();
    EmittingContextImpl* GetEmittingContextImpl() { return emittingContextImpl; }
private:
    EmittingContextImpl* emittingContextImpl;
};

void GenerateCode(EmittingContext& emittingContext, BoundCompileUnit& boundCompileUnit);

} } // namespace cmajor::emitter

#endif // CMAJOR_EMITTER_EMITTER_INCLUDED
