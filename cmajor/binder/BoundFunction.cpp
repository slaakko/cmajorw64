// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/binder/BoundStatement.hpp>
#include <cmajor/binder/BoundNodeVisitor.hpp>
#include <cmajor/symbols/Exception.hpp>

namespace cmajor { namespace binder {

BoundFunction::BoundFunction(FunctionSymbol* functionSymbol_) : BoundNode(functionSymbol_->GetSpan(), BoundNodeType::boundFunction), functionSymbol(functionSymbol_), hasGotos(false)
{
}

void BoundFunction::Load(Emitter& emitter)
{
    throw Exception("cannot load from function", GetSpan());
}

void BoundFunction::Store(Emitter& emitter)
{
    throw Exception("cannot store to function", GetSpan());
}

void BoundFunction::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

void BoundFunction::SetBody(std::unique_ptr<BoundCompoundStatement>&& body_)
{
    body = std::move(body_);
}

} } // namespace cmajor::binder
