// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundClass.hpp>
#include <cmajor/binder/BoundNodeVisitor.hpp>
#include <cmajor/symbols/Exception.hpp>

namespace cmajor { namespace binder {

BoundClass::BoundClass(Module* module_, ClassTypeSymbol* classTypeSymbol_) : BoundNode(module_, classTypeSymbol_->GetSpan(), BoundNodeType::boundClass), classTypeSymbol(classTypeSymbol_)
{
}

void BoundClass::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

void BoundClass::Load(Emitter& emitter, OperationFlags flags)
{
    throw Exception(GetModule(),  "cannot load from class", GetSpan());
}

void BoundClass::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception(GetModule(), "cannot store to class", GetSpan());
}

void BoundClass::AddMember(std::unique_ptr<BoundNode>&& member)
{
    members.push_back(std::move(member));
}

} } // namespace cmajor::binder
