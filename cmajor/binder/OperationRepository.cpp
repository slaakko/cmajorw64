// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/OperationRepository.hpp>

namespace cmajor { namespace binder {

Operation::Operation(std::u32string groupName_, int arity_) : groupName(groupName_), arity(arity_)
{
}

void Operation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*> viableFunctions)
{
}

void ArityOperation::Add(Operation* operation)
{
    operations.push_back(operation);
}

void ArityOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*> viableFunctions)
{
    for (Operation* operation : operations)
    {
        operation->CollectViableFunctions(arguments, viableFunctions);
    }
}

void OperationGroup::Add(Operation* operation)
{
    int arity = operation->Arity();
    if (arity >= arityOperations.size())
    {
        arityOperations.resize(arity + 1);
    }
    ArityOperation* arityOperation = arityOperations[arity].get();
    if (!arityOperation)
    {
        arityOperation = new ArityOperation();
        arityOperations[arity].reset(arityOperation);
    }
    arityOperation->Add(operation);
}

void OperationGroup::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*> viableFunctions)
{
    int arity = arguments.size();
    if (arity < arityOperations.size())
    {
        ArityOperation* arityOperation = arityOperations[arity].get();
        if (arityOperation)
        {
            arityOperation->CollectViableFunctions(arguments, viableFunctions);
        }
    }
}

OperationRepository::OperationRepository()
{
    Add(new Operation(U"operator+", 2));
}

void OperationRepository::Add(Operation* operation)
{
    OperationGroup* group = nullptr;
    auto it = operationGroupMap.find(operation->GroupName());
    if (it != operationGroupMap.cend())
    {
        group = it->second;
    }
    else
    {
        group = new OperationGroup();
        operationGroups.push_back(std::unique_ptr<OperationGroup>(group));
    }
    group->Add(operation);
    operations.push_back(std::unique_ptr<Operation>(operation));
}

void OperationRepository::CollectViableFunctions(const std::u32string& groupName, const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*> viableFunctions)
{
    auto it = operationGroupMap.find(groupName);
    if (it != operationGroupMap.cend())
    {
        OperationGroup* operationGroup = it->second;
        operationGroup->CollectViableFunctions(arguments, viableFunctions);
    }
}

} } // namespace cmajor::binder
