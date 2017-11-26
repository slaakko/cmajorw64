// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/ArrayTypeSymbol.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

ArrayTypeSymbol::ArrayTypeSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::arrayTypeSymbol, span_, name_), elementType(nullptr), size(0), irType(nullptr)
{
}

ArrayTypeSymbol::ArrayTypeSymbol(const Span& span_, const std::u32string& name_, TypeSymbol* elementType_, uint64_t size_) : 
    TypeSymbol(SymbolType::arrayTypeSymbol, span_, name_), elementType(elementType_), size(size_), irType(nullptr)
{
}

void ArrayTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    uint32_t elementTypeId = elementType->TypeId();
    writer.GetBinaryWriter().Write(elementTypeId);
    writer.GetBinaryWriter().Write(size);
}

void ArrayTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    uint32_t elementTypeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, elementTypeId, 0);
    size = reader.GetBinaryReader().ReadULong();
}

void ArrayTypeSymbol::EmplaceType(TypeSymbol* typeSymbol, int index)
{
    if (index == 0)
    {
        elementType = typeSymbol;
    }
    else
    {
        throw Exception("internal error: invalid array emplace type index " + std::to_string(index), GetSpan());
    }
}

llvm::Type* ArrayTypeSymbol::IrType(Emitter& emitter)
{
    if (size == 0)
    {
        throw Exception("array '" + ToUtf8(FullName()) + "' size not defined", GetSpan());
    }
    if (!irType)
    {
        irType = llvm::ArrayType::get(elementType->IrType(emitter), size);
    }
    return irType;
}

llvm::Constant* ArrayTypeSymbol::CreateDefaultIrValue(Emitter& emitter)
{
    if (size == 0)
    {
        throw Exception("array '" + ToUtf8(FullName()) + "' size not defined", GetSpan());
    }
    llvm::Type* irType = IrType(emitter);
    std::vector<llvm::Constant*> arrayOfDefaults;
    for (uint64_t i = 0; i < size; ++i)
    {
        arrayOfDefaults.push_back(elementType->CreateDefaultIrValue(emitter));
    }
    return llvm::ConstantArray::get(llvm::cast<llvm::ArrayType>(irType), arrayOfDefaults);
}

ArrayTypeDefaultConstructor::ArrayTypeDefaultConstructor(ArrayTypeSymbol* arrayType_, LocalVariableSymbol* loopVar_, FunctionSymbol* elementTypeDefaultConstructor_, const Span& span_) :
    FunctionSymbol(arrayType_->GetSpan(), U"@arrayDefaultCtor"), arrayType(arrayType_), loopVar(loopVar_), elementTypeDefaultConstructor(elementTypeDefaultConstructor_)
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_); 
    ParameterSymbol* thisParam = new ParameterSymbol(span_, U"this");
    thisParam->SetType(arrayType->AddPointer(span_));
    AddMember(thisParam);
    ComputeName();
}

void ArrayTypeDefaultConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) 
{
    Assert(genObjects.size() == 1, "default constructor needs one object");
    genObjects[0]->Load(emitter, OperationFlags::addr);
    llvm::Value* ptr = emitter.Stack().Pop();
    emitter.Builder().CreateStore(emitter.Builder().getInt64(0), loopVar->IrObject());
    llvm::Value* size = emitter.Builder().getInt64(arrayType->Size());
    llvm::BasicBlock* loop = llvm::BasicBlock::Create(emitter.Context(), "loop", emitter.Function());
    llvm::BasicBlock* init = llvm::BasicBlock::Create(emitter.Context(), "init", emitter.Function());
    llvm::BasicBlock* next = llvm::BasicBlock::Create(emitter.Context(), "next", emitter.Function());
    emitter.Builder().CreateBr(loop);
    emitter.SetCurrentBasicBlock(loop);
    llvm::Value* index = emitter.Builder().CreateLoad(loopVar->IrObject());
    llvm::Value* less = emitter.Builder().CreateICmpULT(index, size);
    emitter.Builder().CreateCondBr(less, init, next);
    emitter.SetCurrentBasicBlock(init);
    ArgVector elementIndeces;
    elementIndeces.push_back(emitter.Builder().getInt64(0));
    elementIndeces.push_back(index);
    llvm::Value* elementPtr = emitter.Builder().CreateGEP(ptr, elementIndeces);
    LlvmValue elementPtrValue(elementPtr);
    std::vector<GenObject*> elementGenObjects;
    elementGenObjects.push_back(&elementPtrValue);
    elementTypeDefaultConstructor->GenerateCall(emitter, elementGenObjects, OperationFlags::none);
    llvm::Value* nextI = emitter.Builder().CreateAdd(index, emitter.Builder().getInt64(1));
    emitter.Builder().CreateStore(nextI, loopVar->IrObject());
    emitter.Builder().CreateBr(loop);
    emitter.SetCurrentBasicBlock(next);
}

ArrayTypeCopyConstructor::ArrayTypeCopyConstructor(ArrayTypeSymbol* arrayType_, LocalVariableSymbol* loopVar_, FunctionSymbol* elementTypeCopyConstructor_, const Span& span_) :
    FunctionSymbol(arrayType_->GetSpan(), U"@arrayCopyCtor"), arrayType(arrayType_), loopVar(loopVar_), elementTypeCopyConstructor(elementTypeCopyConstructor_)
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(span_, U"this");
    thisParam->SetType(arrayType->AddPointer(span_));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(span_, U"that");
    thatParam->SetType(arrayType->AddConst(span_)->AddLvalueReference(span_));
    AddMember(thatParam);
    ComputeName();
}

void ArrayTypeCopyConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 2, "copy constructor needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::addr);
    llvm::Value* ptr = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* sourcePtr = emitter.Stack().Pop();
    emitter.Builder().CreateStore(emitter.Builder().getInt64(0), loopVar->IrObject());
    llvm::Value* size = emitter.Builder().getInt64(arrayType->Size());
    llvm::BasicBlock* loop = llvm::BasicBlock::Create(emitter.Context(), "loop", emitter.Function());
    llvm::BasicBlock* init = llvm::BasicBlock::Create(emitter.Context(), "init", emitter.Function());
    llvm::BasicBlock* next = llvm::BasicBlock::Create(emitter.Context(), "next", emitter.Function());
    emitter.Builder().CreateBr(loop);
    emitter.SetCurrentBasicBlock(loop);
    llvm::Value* index = emitter.Builder().CreateLoad(loopVar->IrObject());
    llvm::Value* less = emitter.Builder().CreateICmpULT(index, size);
    emitter.Builder().CreateCondBr(less, init, next);
    emitter.SetCurrentBasicBlock(init);
    ArgVector elementIndeces;
    elementIndeces.push_back(emitter.Builder().getInt64(0));
    elementIndeces.push_back(index);
    llvm::Value* elementPtr = emitter.Builder().CreateGEP(ptr, elementIndeces);
    LlvmValue elementPtrValue(elementPtr);
    std::vector<GenObject*> elementGenObjects;
    elementGenObjects.push_back(&elementPtrValue);
    llvm::Value* sourceElementPtr = emitter.Builder().CreateGEP(sourcePtr, elementIndeces);
    llvm::Value* sourceElementValue = sourceElementPtr;
    TypeSymbol* elementType = arrayType->ElementType();
    if (elementType->IsBasicTypeSymbol() || elementType->IsPointerType() || elementType->GetSymbolType() == SymbolType::delegateTypeSymbol)
    {
        sourceElementValue = emitter.Builder().CreateLoad(sourceElementPtr);
    }
    LlvmValue sourceValue(sourceElementValue);
    elementGenObjects.push_back(&sourceValue);
    elementTypeCopyConstructor->GenerateCall(emitter, elementGenObjects, OperationFlags::none);
    llvm::Value* nextI = emitter.Builder().CreateAdd(index, emitter.Builder().getInt64(1));
    emitter.Builder().CreateStore(nextI, loopVar->IrObject());
    emitter.Builder().CreateBr(loop);
    emitter.SetCurrentBasicBlock(next);
}

ArrayTypeMoveConstructor::ArrayTypeMoveConstructor(ArrayTypeSymbol* arrayType_, LocalVariableSymbol* loopVar_, FunctionSymbol* elementTypeMoveConstructor_, const Span& span_) :
    FunctionSymbol(arrayType_->GetSpan(), U"@arrayMoveCtor"), arrayType(arrayType_), loopVar(loopVar_), elementTypeMoveConstructor(elementTypeMoveConstructor_)
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(span_, U"this");
    thisParam->SetType(arrayType->AddPointer(span_));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(span_, U"that");
    thatParam->SetType(arrayType->AddRvalueReference(span_));
    AddMember(thatParam);
    ComputeName();
}

void ArrayTypeMoveConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 2, "move constructor needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::addr);
    llvm::Value* ptr = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* sourcePtr = emitter.Stack().Pop();
    emitter.Builder().CreateStore(emitter.Builder().getInt64(0), loopVar->IrObject());
    llvm::Value* size = emitter.Builder().getInt64(arrayType->Size());
    llvm::BasicBlock* loop = llvm::BasicBlock::Create(emitter.Context(), "loop", emitter.Function());
    llvm::BasicBlock* init = llvm::BasicBlock::Create(emitter.Context(), "init", emitter.Function());
    llvm::BasicBlock* next = llvm::BasicBlock::Create(emitter.Context(), "next", emitter.Function());
    emitter.Builder().CreateBr(loop);
    emitter.SetCurrentBasicBlock(loop);
    llvm::Value* index = emitter.Builder().CreateLoad(loopVar->IrObject());
    llvm::Value* less = emitter.Builder().CreateICmpULT(index, size);
    emitter.Builder().CreateCondBr(less, init, next);
    emitter.SetCurrentBasicBlock(init);
    ArgVector elementIndeces;
    elementIndeces.push_back(emitter.Builder().getInt64(0));
    elementIndeces.push_back(index);
    llvm::Value* elementPtr = emitter.Builder().CreateGEP(ptr, elementIndeces);
    LlvmValue elementPtrValue(elementPtr);
    std::vector<GenObject*> elementGenObjects;
    elementGenObjects.push_back(&elementPtrValue);
    llvm::Value* sourceElementPtr = emitter.Builder().CreateGEP(sourcePtr, elementIndeces);
    LlvmValue sourcePtrValue(sourceElementPtr);
    elementGenObjects.push_back(&sourcePtrValue);
    elementTypeMoveConstructor->GenerateCall(emitter, elementGenObjects, OperationFlags::none);
    llvm::Value* nextI = emitter.Builder().CreateAdd(index, emitter.Builder().getInt64(1));
    emitter.Builder().CreateStore(nextI, loopVar->IrObject());
    emitter.Builder().CreateBr(loop);
    emitter.SetCurrentBasicBlock(next);
}

ArrayTypeCopyAssignment::ArrayTypeCopyAssignment(ArrayTypeSymbol* arrayType_, LocalVariableSymbol* loopVar_, FunctionSymbol* elementTypeCopyAssignment_, const Span& span_) :
    FunctionSymbol(arrayType_->GetSpan(), U"@arrayCopyAssignment"), arrayType(arrayType_), loopVar(loopVar_), elementTypeCopyAssignment(elementTypeCopyAssignment_)
{
    SetGroupName(U"operator=");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(span_, U"this");
    thisParam->SetType(arrayType->AddPointer(span_));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(span_, U"that");
    thatParam->SetType(arrayType->AddConst(span_)->AddLvalueReference(span_));
    AddMember(thatParam);
    TypeSymbol* voidType = arrayType->GetSymbolTable()->GetTypeByName(U"void");
    SetReturnType(voidType);
    ComputeName();
}

void ArrayTypeCopyAssignment::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 2, "copy assignment needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::addr);
    llvm::Value* ptr = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* sourcePtr = emitter.Stack().Pop();
    emitter.Builder().CreateStore(emitter.Builder().getInt64(0), loopVar->IrObject());
    llvm::Value* size = emitter.Builder().getInt64(arrayType->Size());
    llvm::BasicBlock* loop = llvm::BasicBlock::Create(emitter.Context(), "loop", emitter.Function());
    llvm::BasicBlock* init = llvm::BasicBlock::Create(emitter.Context(), "init", emitter.Function());
    llvm::BasicBlock* next = llvm::BasicBlock::Create(emitter.Context(), "next", emitter.Function());
    emitter.Builder().CreateBr(loop);
    emitter.SetCurrentBasicBlock(loop);
    llvm::Value* index = emitter.Builder().CreateLoad(loopVar->IrObject());
    llvm::Value* less = emitter.Builder().CreateICmpULT(index, size);
    emitter.Builder().CreateCondBr(less, init, next);
    emitter.SetCurrentBasicBlock(init);
    ArgVector elementIndeces;
    elementIndeces.push_back(emitter.Builder().getInt64(0));
    elementIndeces.push_back(index);
    llvm::Value* elementPtr = emitter.Builder().CreateGEP(ptr, elementIndeces);
    LlvmValue elementPtrValue(elementPtr);
    std::vector<GenObject*> elementGenObjects;
    elementGenObjects.push_back(&elementPtrValue);
    llvm::Value* sourceElementPtr = emitter.Builder().CreateGEP(sourcePtr, elementIndeces);
    llvm::Value* sourceElementValue = sourceElementPtr;
    TypeSymbol* elementType = arrayType->ElementType();
    if (elementType->IsBasicTypeSymbol() || elementType->IsPointerType() || elementType->GetSymbolType() == SymbolType::delegateTypeSymbol)
    {
        sourceElementValue = emitter.Builder().CreateLoad(sourceElementPtr);
    }
    LlvmValue sourceValue(sourceElementValue);
    elementGenObjects.push_back(&sourceValue);
    elementTypeCopyAssignment->GenerateCall(emitter, elementGenObjects, OperationFlags::none);
    llvm::Value* nextI = emitter.Builder().CreateAdd(index, emitter.Builder().getInt64(1));
    emitter.Builder().CreateStore(nextI, loopVar->IrObject());
    emitter.Builder().CreateBr(loop);
    emitter.SetCurrentBasicBlock(next);
}

ArrayTypeMoveAssignment::ArrayTypeMoveAssignment(ArrayTypeSymbol* arrayType_, LocalVariableSymbol* loopVar_, FunctionSymbol* elementTypeMoveAssignment_, const Span& span_) :
    FunctionSymbol(arrayType_->GetSpan(), U"@arrayMoveAssignment"), arrayType(arrayType_), loopVar(loopVar_), elementTypeMoveAssignment(elementTypeMoveAssignment_)
{
    SetGroupName(U"operator=");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(span_, U"this");
    thisParam->SetType(arrayType->AddPointer(span_));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(span_, U"that");
    thatParam->SetType(arrayType->AddRvalueReference(span_));
    AddMember(thatParam);
    TypeSymbol* voidType = arrayType->GetSymbolTable()->GetTypeByName(U"void");
    SetReturnType(voidType);
    ComputeName();
}

void ArrayTypeMoveAssignment::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 2, "move assignment needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::addr);
    llvm::Value* ptr = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* sourcePtr = emitter.Stack().Pop();
    emitter.Builder().CreateStore(emitter.Builder().getInt64(0), loopVar->IrObject());
    llvm::Value* size = emitter.Builder().getInt64(arrayType->Size());
    llvm::BasicBlock* loop = llvm::BasicBlock::Create(emitter.Context(), "loop", emitter.Function());
    llvm::BasicBlock* init = llvm::BasicBlock::Create(emitter.Context(), "init", emitter.Function());
    llvm::BasicBlock* next = llvm::BasicBlock::Create(emitter.Context(), "next", emitter.Function());
    emitter.Builder().CreateBr(loop);
    emitter.SetCurrentBasicBlock(loop);
    llvm::Value* index = emitter.Builder().CreateLoad(loopVar->IrObject());
    llvm::Value* less = emitter.Builder().CreateICmpULT(index, size);
    emitter.Builder().CreateCondBr(less, init, next);
    emitter.SetCurrentBasicBlock(init);
    ArgVector elementIndeces;
    elementIndeces.push_back(emitter.Builder().getInt64(0));
    elementIndeces.push_back(index);
    llvm::Value* elementPtr = emitter.Builder().CreateGEP(ptr, elementIndeces);
    LlvmValue elementPtrValue(elementPtr);
    std::vector<GenObject*> elementGenObjects;
    elementGenObjects.push_back(&elementPtrValue);
    llvm::Value* sourceElementPtr = emitter.Builder().CreateGEP(sourcePtr, elementIndeces);
    TypeSymbol* elementType = arrayType->ElementType();
    LlvmValue sourcePtrValue(sourceElementPtr);
    elementGenObjects.push_back(&sourcePtrValue);
    elementTypeMoveAssignment->GenerateCall(emitter, elementGenObjects, OperationFlags::none);
    llvm::Value* nextI = emitter.Builder().CreateAdd(index, emitter.Builder().getInt64(1));
    emitter.Builder().CreateStore(nextI, loopVar->IrObject());
    emitter.Builder().CreateBr(loop);
    emitter.SetCurrentBasicBlock(next);
}

ArrayTypeElementAccess::ArrayTypeElementAccess(ArrayTypeSymbol* arrayType_, const Span& span_) : FunctionSymbol(arrayType_->GetSpan(), U"@arrayElementAccess"), arrayType(arrayType_)
{
    SetGroupName(U"operator[]");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* arrayParam = new ParameterSymbol(span_, U"array");
    arrayParam->SetType(arrayType);
    AddMember(arrayParam);
    ParameterSymbol* indexParam = new ParameterSymbol(span_, U"index");
    indexParam->SetType(arrayType->GetSymbolTable()->GetTypeByName(U"long"));
    AddMember(indexParam);
    TypeSymbol* returnType = arrayType->ElementType();
    if (!returnType->IsBasicTypeSymbol() && !returnType->IsPointerType() && returnType->GetSymbolType() != SymbolType::delegateTypeSymbol)
    {
        returnType = returnType->AddLvalueReference(span_);
    }
    SetReturnType(returnType);
    ComputeName();
}

void ArrayTypeElementAccess::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 2, "element access needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::addr);
    llvm::Value* ptr = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* indexValue = emitter.Stack().Pop();
    ArgVector elementIndeces;
    elementIndeces.push_back(emitter.Builder().getInt64(0));
    elementIndeces.push_back(indexValue);
    llvm::Value* elementPtr = emitter.Builder().CreateGEP(ptr, elementIndeces);
    TypeSymbol* elementType = arrayType->ElementType();
    if ((flags & OperationFlags::addr) == OperationFlags::none && (elementType->IsBasicTypeSymbol() || elementType->IsPointerType() || elementType->GetSymbolType() == SymbolType::delegateTypeSymbol))
    {
        llvm::Value* elementValue = emitter.Builder().CreateLoad(elementPtr);
        emitter.Stack().Push(elementValue);
    }
    else
    {
        emitter.Stack().Push(elementPtr);
    }
}

} } // namespace cmajor::symbols
