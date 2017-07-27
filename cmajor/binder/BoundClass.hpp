// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_BOUND_CLASS_INCLUDED
#define CMAJOR_BINDER_BOUND_CLASS_INCLUDED
#include <cmajor/binder/BoundNode.hpp>
#include <cmajor/symbols/ClassTypeSymbol.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::symbols;

class BoundClass : public BoundNode
{
public:
    BoundClass(ClassTypeSymbol* classTypeSymbol_);
    void Accept(BoundNodeVisitor& visitor) override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void AddMember(std::unique_ptr<BoundNode>&& member);
    const std::vector<std::unique_ptr<BoundNode>>& Members() const { return members; }
private:
    ClassTypeSymbol* classTypeSymbol;
    std::vector<std::unique_ptr<BoundNode>> members;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_BOUND_CLASS_INCLUDED
