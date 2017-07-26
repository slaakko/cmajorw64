// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_OPERATION_REPOSITORY_INCLUDED
#define CMAJOR_BINDER_OPERATION_REPOSITORY_INCLUDED
#include <cmajor/binder/BoundExpression.hpp>

namespace cmajor { namespace binder {

class BoundCompileUnit;

class Operation
{
public:
    Operation(const std::u32string& groupName_, int arity_, BoundCompileUnit& boundCompileUnit_);
    virtual ~Operation();
    virtual void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) = 0;
    const std::u32string& GroupName() const { return groupName; }
    int Arity() const { return arity; }
    SymbolTable* GetSymbolTable();
    BoundCompileUnit& GetBoundCompileUnit();
private:
    std::u32string groupName;
    int arity;
    BoundCompileUnit& boundCompileUnit;
};

class ArityOperation
{
public:
    void Add(Operation* operation);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions);
private:
    std::vector<Operation*> operations;
};

class OperationGroup
{
public:
    void Add(Operation* operation);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions);
private:
    std::vector<std::unique_ptr<ArityOperation>> arityOperations;
};

class OperationRepository
{
public:
    OperationRepository(BoundCompileUnit& boundCompileUnit_);
    void Add(Operation* operation);
    void CollectViableFunctions(const std::u32string& groupName, const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions);
private:
    std::unordered_map<std::u32string, OperationGroup*> operationGroupMap;
    std::vector<std::unique_ptr<OperationGroup>> operationGroups;
    std::vector<std::unique_ptr<Operation>> operations;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_OPERATION_REPOSITORY_INCLUDED
