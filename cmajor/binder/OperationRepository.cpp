// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/OperationRepository.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/symbols/BasicTypeOperation.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <llvm/IR/Constant.h>

namespace cmajor { namespace binder {

class PointerDefaultCtor : public FunctionSymbol
{
public:
    PointerDefaultCtor(TypeSymbol* type_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    TypeSymbol* type;
    llvm::Value* nullValue;
};

PointerDefaultCtor::PointerDefaultCtor(TypeSymbol* type_) : FunctionSymbol(Span(), U"@constructor"), type(type_), nullValue(nullptr)
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(type);
    AddMember(thisParam);
    ComputeName();
}

void PointerDefaultCtor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 1, "default constructor needs one object");
    if (!nullValue)
    {
        nullValue = llvm::Constant::getNullValue(type->IrType(emitter));
    }
    emitter.Stack().Push(nullValue);
    genObjects[0]->Store(emitter, OperationFlags::none);
}

class PointerDefaultConstructorOperation : public Operation
{
public:
    PointerDefaultConstructorOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

PointerDefaultConstructorOperation::PointerDefaultConstructorOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"@constructor", 1, boundCompileUnit_)
{
}

void PointerDefaultConstructorOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* type = arguments[0]->GetType();
    if (!type->IsPointerType()) return;
    FunctionSymbol* function = functionMap[type];
    if (!function)
    {
        function = new PointerDefaultCtor(type);
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[type] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class PointerCopyCtor : public FunctionSymbol
{
public:
    PointerCopyCtor(TypeSymbol* type_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    TypeSymbol* type;
};

PointerCopyCtor::PointerCopyCtor(TypeSymbol* type_) : FunctionSymbol(Span(), U"@constructor"), type(type_)
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(type);
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(type);
    AddMember(thatParam);
    ComputeName();
}

void PointerCopyCtor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 2, "copy constructor needs two objects");
    genObjects[1]->Load(emitter, OperationFlags::none);
    genObjects[0]->Store(emitter, OperationFlags::none);
}

class PointerCopyConstructorOperation : public Operation
{
public:
    PointerCopyConstructorOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

PointerCopyConstructorOperation::PointerCopyConstructorOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"@constructor", 2, boundCompileUnit_)
{
}

void PointerCopyConstructorOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* type = arguments[0]->GetType();
    if (!type->IsPointerType()) return;
    FunctionSymbol* function = functionMap[type];
    if (!function)
    {
        function = new PointerCopyCtor(type);
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[type] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class PointerCopyAssignment : public FunctionSymbol
{
public:
    PointerCopyAssignment(TypeSymbol* type_, TypeSymbol* voidType_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    TypeSymbol* type;
};

PointerCopyAssignment::PointerCopyAssignment(TypeSymbol* type_, TypeSymbol* voidType_) : FunctionSymbol(Span(), U"operator="), type(type_)
{
    SetGroupName(U"operator=");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(type);
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(type);
    AddMember(thatParam);
    SetReturnType(voidType_);
    ComputeName();
}

void PointerCopyAssignment::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 2, "copy assignment needs two objects");
    genObjects[1]->Load(emitter, OperationFlags::none);
    genObjects[0]->Store(emitter, OperationFlags::none);
}

class PointerCopyAssignmentOperation : public Operation
{
public:
    PointerCopyAssignmentOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

PointerCopyAssignmentOperation::PointerCopyAssignmentOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"operator=", 2, boundCompileUnit_)
{
}

void PointerCopyAssignmentOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* type = arguments[0]->GetType();
    if (!type->IsPointerType()) return;
    FunctionSymbol* function = functionMap[type];
    if (!function)
    {
        function = new PointerCopyAssignment(type, GetSymbolTable()->GetTypeByName(U"void"));
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[type] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class PointerReturn : public FunctionSymbol
{
public:
    PointerReturn(TypeSymbol* type_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    TypeSymbol* type;
};

PointerReturn::PointerReturn(TypeSymbol* type_) : FunctionSymbol(Span(), U"@return"), type(type_)
{
    SetGroupName(U"@return");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* valueParam = new ParameterSymbol(Span(), U"value");
    valueParam->SetType(type);
    AddMember(valueParam);
    SetReturnType(type);
    ComputeName();
}

void PointerReturn::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 1, "return needs one object");
    genObjects[0]->Load(emitter, OperationFlags::none);
}

class PointerReturnOperation : public Operation
{
public:
    PointerReturnOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

PointerReturnOperation::PointerReturnOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"@return", 1, boundCompileUnit_)
{
}

void PointerReturnOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* type = arguments[0]->GetType();
    if (!type->IsPointerType()) return;
    FunctionSymbol* function = functionMap[type];
    if (!function)
    {
        function = new PointerReturn(type);
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[type] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class PointerPlusOffset : public FunctionSymbol
{
public:
    PointerPlusOffset(TypeSymbol* pointerType_, TypeSymbol* longType_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
};

PointerPlusOffset::PointerPlusOffset(TypeSymbol* pointerType_, TypeSymbol* longType_) : FunctionSymbol(Span(), U"operator+")
{
    SetGroupName(U"operator+");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* leftParam = new ParameterSymbol(Span(), U"left");
    leftParam->SetType(pointerType_);
    AddMember(leftParam);
    ParameterSymbol* rightParam = new ParameterSymbol(Span(), U"right");
    rightParam->SetType(longType_);
    AddMember(rightParam);
    SetReturnType(pointerType_);
    ComputeName();
}

void PointerPlusOffset::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 2, "operator+ needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* left = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* right = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateGEP(left, right));
}

class PointerPlusOffsetOperation : public Operation
{
public:
    PointerPlusOffsetOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

PointerPlusOffsetOperation::PointerPlusOffsetOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"operator+", 2, boundCompileUnit_)
{
}

void PointerPlusOffsetOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* leftType = arguments[0]->GetType();
    if (!leftType->IsPointerType()) return;
    FunctionSymbol* function = functionMap[leftType];
    if (!function)
    {
        function = new PointerPlusOffset(leftType, GetSymbolTable()->GetTypeByName(U"long"));
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[leftType] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class OffsetPlusPointer : public FunctionSymbol
{
public:
    OffsetPlusPointer(TypeSymbol* longType_, TypeSymbol* pointerType_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
};

OffsetPlusPointer::OffsetPlusPointer(TypeSymbol* longType_, TypeSymbol* pointerType_) : FunctionSymbol(Span(), U"operator+")
{
    SetGroupName(U"operator+");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* leftParam = new ParameterSymbol(Span(), U"left");
    leftParam->SetType(longType_);
    AddMember(leftParam);
    ParameterSymbol* rightParam = new ParameterSymbol(Span(), U"right");
    rightParam->SetType(pointerType_);
    AddMember(rightParam);
    SetReturnType(pointerType_);
    ComputeName();
}

void OffsetPlusPointer::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 2, "operator+ needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* left = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* right = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateGEP(right, left));
}

class OffsetPlusPointerOperation : public Operation
{
public:
    OffsetPlusPointerOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

OffsetPlusPointerOperation::OffsetPlusPointerOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"operator+", 2, boundCompileUnit_)
{
}

void OffsetPlusPointerOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* rightType = arguments[1]->GetType();
    if (!rightType->IsPointerType()) return;
    TypeSymbol* leftType = GetSymbolTable()->GetTypeByName(U"long");
    FunctionSymbol* function = functionMap[rightType];
    if (!function)
    {
        function = new OffsetPlusPointer(leftType, rightType);
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[rightType] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class PointerMinusOffset : public FunctionSymbol
{
public:
    PointerMinusOffset(TypeSymbol* pointerType_, TypeSymbol* longType_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
};

PointerMinusOffset::PointerMinusOffset(TypeSymbol* pointerType_, TypeSymbol* longType_) : FunctionSymbol(Span(), U"operator-")
{
    SetGroupName(U"operator-");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* leftParam = new ParameterSymbol(Span(), U"left");
    leftParam->SetType(pointerType_);
    AddMember(leftParam);
    ParameterSymbol* rightParam = new ParameterSymbol(Span(), U"right");
    rightParam->SetType(longType_);
    AddMember(rightParam);
    SetReturnType(pointerType_);
    ComputeName();
}

void PointerMinusOffset::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 2, "operator- needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* left = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* right = emitter.Stack().Pop();
    llvm::Value* offset = emitter.Builder().CreateNeg(right);
    emitter.Stack().Push(emitter.Builder().CreateGEP(left, offset));
}

class PointerMinusOffsetOperation : public Operation
{
public:
    PointerMinusOffsetOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

PointerMinusOffsetOperation::PointerMinusOffsetOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"operator-", 2, boundCompileUnit_)
{
}

void PointerMinusOffsetOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* leftType = arguments[0]->GetType();
    if (!leftType->IsPointerType()) return;
    FunctionSymbol* function = functionMap[leftType];
    if (!function)
    {
        function = new PointerMinusOffset(leftType, GetSymbolTable()->GetTypeByName(U"long"));
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[leftType] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class PointerMinusPointer : public FunctionSymbol
{
public:
    PointerMinusPointer(TypeSymbol* pointerType_, TypeSymbol* longType_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
};

PointerMinusPointer::PointerMinusPointer(TypeSymbol* pointerType_, TypeSymbol* longType_) : FunctionSymbol(Span(), U"operator-")
{
    SetGroupName(U"operator-");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* leftParam = new ParameterSymbol(Span(), U"left");
    leftParam->SetType(pointerType_);
    AddMember(leftParam);
    ParameterSymbol* rightParam = new ParameterSymbol(Span(), U"right");
    rightParam->SetType(pointerType_);
    AddMember(rightParam);
    SetReturnType(longType_);
    ComputeName();
}

void PointerMinusPointer::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 2, "operator- needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* left = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* right = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreatePtrDiff(left, right));
}

class PointerMinusPointerOperation : public Operation
{
public:
    PointerMinusPointerOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

PointerMinusPointerOperation::PointerMinusPointerOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"operator-", 2, boundCompileUnit_)
{
}

void PointerMinusPointerOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* leftType = arguments[0]->GetType();
    if (!leftType->IsPointerType()) return;
    TypeSymbol* rightType = arguments[1]->GetType();
    if (!rightType->IsPointerType()) return;
    FunctionSymbol* function = functionMap[leftType];
    if (!function)
    {
        function = new PointerMinusPointer(leftType, GetSymbolTable()->GetTypeByName(U"long"));
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[leftType] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class PointerEqual : public FunctionSymbol
{
public:
    PointerEqual(TypeSymbol* pointerType_, TypeSymbol* boolType_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
};

PointerEqual::PointerEqual(TypeSymbol* pointerType_, TypeSymbol* boolType_) : FunctionSymbol(Span(), U"operator==")
{
    SetGroupName(U"operator==");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* leftParam = new ParameterSymbol(Span(), U"left");
    leftParam->SetType(pointerType_);
    AddMember(leftParam);
    ParameterSymbol* rightParam = new ParameterSymbol(Span(), U"right");
    rightParam->SetType(pointerType_);
    AddMember(rightParam);
    SetReturnType(boolType_);
    ComputeName();
}

void PointerEqual::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 2, "operator== needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* left = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* right = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateICmpEQ(left, right));
}

class PointerEqualOperation : public Operation
{
public:
    PointerEqualOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

PointerEqualOperation::PointerEqualOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"operator==", 2, boundCompileUnit_)
{
}

void PointerEqualOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* leftType = arguments[0]->GetType();
    if (!leftType->IsPointerType()) return;
    TypeSymbol* rightType = arguments[1]->GetType();
    if (!rightType->IsPointerType()) return;
    FunctionSymbol* function = functionMap[leftType];
    if (!function)
    {
        function = new PointerEqual(leftType, GetSymbolTable()->GetTypeByName(U"bool"));
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[leftType] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class PointerLess : public FunctionSymbol
{
public:
    PointerLess(TypeSymbol* pointerType_, TypeSymbol* boolType_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
};

PointerLess::PointerLess(TypeSymbol* pointerType_, TypeSymbol* boolType_) : FunctionSymbol(Span(), U"operator<")
{
    SetGroupName(U"operator<");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* leftParam = new ParameterSymbol(Span(), U"left");
    leftParam->SetType(pointerType_);
    AddMember(leftParam);
    ParameterSymbol* rightParam = new ParameterSymbol(Span(), U"right");
    rightParam->SetType(pointerType_);
    AddMember(rightParam);
    SetReturnType(boolType_);
    ComputeName();
}

void PointerLess::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 2, "operator< needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* left = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* right = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateICmpULT(left, right));
}

class PointerLessOperation : public Operation
{
public:
    PointerLessOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

PointerLessOperation::PointerLessOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"operator<", 2, boundCompileUnit_)
{
}

void PointerLessOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* leftType = arguments[0]->GetType();
    if (!leftType->IsPointerType()) return;
    TypeSymbol* rightType = arguments[1]->GetType();
    if (!rightType->IsPointerType()) return;
    FunctionSymbol* function = functionMap[leftType];
    if (!function)
    {
        function = new PointerEqual(leftType, GetSymbolTable()->GetTypeByName(U"bool"));
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[leftType] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class LvalueRefefenceCopyCtor : public FunctionSymbol
{
public:
    LvalueRefefenceCopyCtor(TypeSymbol* type_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    TypeSymbol* type;
};

LvalueRefefenceCopyCtor::LvalueRefefenceCopyCtor(TypeSymbol* type_) : FunctionSymbol(Span(), U"@constructor"), type(type_)
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(type);
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(type);
    AddMember(thatParam);
    ComputeName();
}

void LvalueRefefenceCopyCtor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 2, "reference constructor needs two objects");
    genObjects[1]->Load(emitter, OperationFlags::none);
    genObjects[0]->Store(emitter, OperationFlags::none);
}

class LvalueReferenceCopyConstructorOperation : public Operation
{
public:
    LvalueReferenceCopyConstructorOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

LvalueReferenceCopyConstructorOperation::LvalueReferenceCopyConstructorOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"@constructor", 2, boundCompileUnit_)
{
}

void LvalueReferenceCopyConstructorOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* type = arguments[0]->GetType();
    if (!type->IsLvalueReferenceType()) return;
    FunctionSymbol* function = functionMap[type];
    if (!function)
    {
        function = new LvalueRefefenceCopyCtor(type);
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[type] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class LvalueReferenceCopyAssignment : public FunctionSymbol
{
public:
    LvalueReferenceCopyAssignment(TypeSymbol* type_, TypeSymbol* voidType_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    TypeSymbol* type;
};

LvalueReferenceCopyAssignment::LvalueReferenceCopyAssignment(TypeSymbol* type_, TypeSymbol* voidType_) : FunctionSymbol(Span(), U"operator="), type(type_)
{
    SetGroupName(U"operator=");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(type);
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(type);
    AddMember(thatParam);
    SetReturnType(voidType_);
    ComputeName();
}

void LvalueReferenceCopyAssignment::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 2, "copy assignment needs two objects");
    genObjects[1]->Load(emitter, OperationFlags::none);
    genObjects[0]->Store(emitter, OperationFlags::none);
}

class LvalueReferenceCopyAssignmentOperation : public Operation
{
public:
    LvalueReferenceCopyAssignmentOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

LvalueReferenceCopyAssignmentOperation::LvalueReferenceCopyAssignmentOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"operator=", 2, boundCompileUnit_)
{
}

void LvalueReferenceCopyAssignmentOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* type = arguments[0]->GetType();
    if (!type->IsLvalueReferenceType()) return;
    FunctionSymbol* function = functionMap[type];
    if (!function)
    {
        function = new LvalueReferenceCopyAssignment(type, GetSymbolTable()->GetTypeByName(U"void"));
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[type] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

class LvalueReferenceReturn : public FunctionSymbol
{
public:
    LvalueReferenceReturn(TypeSymbol* type_);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    TypeSymbol* type;
};

LvalueReferenceReturn::LvalueReferenceReturn(TypeSymbol* type_) : FunctionSymbol(Span(), U"@return"), type(type_)
{
    SetGroupName(U"@return");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* valueParam = new ParameterSymbol(Span(), U"value");
    valueParam->SetType(type);
    AddMember(valueParam);
    SetReturnType(type);
    ComputeName();
}

void LvalueReferenceReturn::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 1, "return needs one object");
    genObjects[0]->Load(emitter, OperationFlags::none);
}

class LvalueReferenceReturnOperation : public Operation
{
public:
    LvalueReferenceReturnOperation(BoundCompileUnit& boundCompileUnit_);
    void CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions) override;
private:
    std::unordered_map<TypeSymbol*, FunctionSymbol*> functionMap;
    std::vector<std::unique_ptr<FunctionSymbol>> functions;
};

LvalueReferenceReturnOperation::LvalueReferenceReturnOperation(BoundCompileUnit& boundCompileUnit_) : Operation(U"@return", 1, boundCompileUnit_)
{
}

void LvalueReferenceReturnOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    TypeSymbol* type = arguments[0]->GetType();
    if (!type->IsLvalueReferenceType()) return;
    FunctionSymbol* function = functionMap[type];
    if (!function)
    {
        function = new LvalueReferenceReturn(type);
        function->SetSymbolTable(GetSymbolTable());
        function->SetParent(&GetSymbolTable()->GlobalNs());
        functionMap[type] = function;
        functions.push_back(std::unique_ptr<FunctionSymbol>(function));
    }
    viableFunctions.insert(function);
}

Operation::Operation(const std::u32string& groupName_, int arity_, BoundCompileUnit& boundCompileUnit_) : groupName(groupName_), arity(arity_), boundCompileUnit(boundCompileUnit_)
{
}

Operation::~Operation()
{
}

SymbolTable* Operation::GetSymbolTable()
{
    return &boundCompileUnit.GetSymbolTable();
}

BoundCompileUnit& Operation::GetBoundCompileUnit()
{
    return boundCompileUnit;
}

void ArityOperation::Add(Operation* operation)
{
    operations.push_back(operation);
}

void ArityOperation::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
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

void OperationGroup::CollectViableFunctions(const std::vector<std::unique_ptr<BoundExpression>>& arguments, std::unordered_set<FunctionSymbol*>& viableFunctions)
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

OperationRepository::OperationRepository(BoundCompileUnit& boundCompileUnit_)
{
    Add(new LvalueReferenceCopyConstructorOperation(boundCompileUnit_));
    Add(new LvalueReferenceCopyAssignmentOperation(boundCompileUnit_));
    Add(new LvalueReferenceReturnOperation(boundCompileUnit_));
    Add(new PointerDefaultConstructorOperation(boundCompileUnit_));
    Add(new PointerCopyConstructorOperation(boundCompileUnit_));
    Add(new PointerCopyAssignmentOperation(boundCompileUnit_));
    Add(new PointerReturnOperation(boundCompileUnit_));
    Add(new PointerPlusOffsetOperation(boundCompileUnit_));
    Add(new OffsetPlusPointerOperation(boundCompileUnit_));
    Add(new PointerMinusOffsetOperation(boundCompileUnit_));
    Add(new PointerMinusPointerOperation(boundCompileUnit_));
    Add(new PointerEqualOperation(boundCompileUnit_));
    Add(new PointerLessOperation(boundCompileUnit_));
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
        operationGroupMap.insert(std::make_pair(operation->GroupName(), group));
        operationGroups.push_back(std::unique_ptr<OperationGroup>(group));
    }
    group->Add(operation);
    operations.push_back(std::unique_ptr<Operation>(operation));
}

void OperationRepository::CollectViableFunctions(const std::u32string& groupName, const std::vector<std::unique_ptr<BoundExpression>>& arguments, 
    std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    auto it = operationGroupMap.find(groupName);
    if (it != operationGroupMap.cend())
    {
        OperationGroup* operationGroup = it->second;
        operationGroup->CollectViableFunctions(arguments, viableFunctions);
    }
}

} } // namespace cmajor::binder
