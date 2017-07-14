// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_BOUND_NODE_INCLUDED
#define CMAJOR_BINDER_BOUND_NODE_INCLUDED
#include <cmajor/parsing/Scanner.hpp>
#include <cmajor/ir/GenObject.hpp>

namespace cmajor { namespace binder {

using cmajor::parsing::Span;
using namespace cmajor::ir;

class BoundNode : public GenObject
{
public:
    BoundNode(const Span& span_);
    const Span& GetSpan() const { return span; }
private:
    Span span;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_BOUND_NODE_INCLUDED
