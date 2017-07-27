// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundClass.hpp>
#include <cmajor/binder/BoundNodeVisitor.hpp>
#include <cmajor/symbols/Exception.hpp>

namespace cmajor { namespace binder {

BoundClass::BoundClass(ClassTypeSymbol* classTypeSymbol_) : BoundNode(classTypeSymbol->GetSpan(), BoundNodeType::boundClass), classTypeSymbol(classTypeSymbol_)
{
}

void BoundClass::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

void BoundClass::Load(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot load from class", GetSpan());
}

void BoundClass::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to class", GetSpan());
}

void BoundClass::AddMember(std::unique_ptr<BoundNode>&& member)
{
    members.push_back(std::move(member));
}

} } // namespace cmajor::binder
