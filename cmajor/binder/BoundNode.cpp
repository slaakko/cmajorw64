// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundNode.hpp>

namespace cmajor { namespace binder {

BoundNode::BoundNode(const Span& span_, BoundNodeType boundNodeType_) : span(span_), boundNodeType(boundNodeType_)
{
}

} } // namespace cmajor::binder
