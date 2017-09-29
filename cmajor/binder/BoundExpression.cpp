// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundExpression.hpp>
#include <cmajor/binder/TypeResolver.hpp>
#include <cmajor/binder/BoundNodeVisitor.hpp>
#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/EnumSymbol.hpp>
#include <cmajor/symbols/ClassTypeSymbol.hpp>
#include <cmajor/ir/Emitter.hpp>
#include <cmajor/util/Unicode.hpp>
#include <llvm/IR/Module.h>

namespace cmajor { namespace binder {

using namespace cmajor::unicode;

BoundExpression::BoundExpression(const Span& span_, BoundNodeType boundNodeType_, TypeSymbol* type_) : BoundNode(span_, boundNodeType_), type(type_), flags(BoundExpressionFlags::none)
{
}

void BoundExpression::AddTemporaryDestructorCall(std::unique_ptr<BoundFunctionCall>&& destructorCall)
{
    temporaryDestructorCalls.push_back(std::move(destructorCall));
}

void BoundExpression::DestroyTemporaries(Emitter& emitter)
{
    for (const std::unique_ptr<BoundFunctionCall>& destructorCall : temporaryDestructorCalls)
    {
        destructorCall->Load(emitter, OperationFlags::none);
    }
}

BoundParameter::BoundParameter(ParameterSymbol* parameterSymbol_) : 
    BoundExpression(parameterSymbol_->GetSpan(), BoundNodeType::boundParameter, parameterSymbol_->GetType()), parameterSymbol(parameterSymbol_)
{
}

BoundExpression* BoundParameter::Clone()
{
    return new BoundParameter(parameterSymbol);
}

void BoundParameter::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        emitter.Stack().Push(parameterSymbol->IrObject());
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        llvm::Value* value = emitter.Builder().CreateLoad(parameterSymbol->IrObject());
        uint8_t n = GetDerefCount(flags);
        for (uint8_t i = 0; i < n; ++i)
        {
            value = emitter.Builder().CreateLoad(value);
        }
        emitter.Stack().Push(value);
    }
    else
    {
        emitter.Stack().Push(emitter.Builder().CreateLoad(parameterSymbol->IrObject()));
    }
    DestroyTemporaries(emitter);
}

void BoundParameter::Store(Emitter& emitter, OperationFlags flags)
{
    llvm::Value* value = emitter.Stack().Pop();
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a parameter", GetSpan());
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        llvm::Value* ptr = emitter.Builder().CreateLoad(parameterSymbol->IrObject());
        uint8_t n = GetDerefCount(flags);
        for (uint8_t i = 1; i < n; ++i)
        {
            ptr = emitter.Builder().CreateLoad(ptr);
        }
        emitter.Builder().CreateStore(value, ptr);
    }
    else
    {
        emitter.Builder().CreateStore(value, parameterSymbol->IrObject());
    }
    DestroyTemporaries(emitter);
}

void BoundParameter::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundLocalVariable::BoundLocalVariable(LocalVariableSymbol* localVariableSymbol_) : 
    BoundExpression(localVariableSymbol_->GetSpan(), BoundNodeType::boundLocalVariable, localVariableSymbol_->GetType()), localVariableSymbol(localVariableSymbol_)
{
}

BoundExpression* BoundLocalVariable::Clone()
{
    return new BoundLocalVariable(localVariableSymbol);
}

void BoundLocalVariable::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        emitter.Stack().Push(localVariableSymbol->IrObject());
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        llvm::Value* value = emitter.Builder().CreateLoad(localVariableSymbol->IrObject());
        uint8_t n = GetDerefCount(flags);
        for (uint8_t i = 0; i < n; ++i)
        {
            value = emitter.Builder().CreateLoad(value);
        }
        emitter.Stack().Push(value);
    }
    else
    {
        emitter.Stack().Push(emitter.Builder().CreateLoad(localVariableSymbol->IrObject()));
    }
    DestroyTemporaries(emitter);
}

void BoundLocalVariable::Store(Emitter& emitter, OperationFlags flags)
{
    llvm::Value* value = emitter.Stack().Pop();
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot store to address of a local variable", GetSpan());
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        llvm::Value* ptr = emitter.Builder().CreateLoad(localVariableSymbol->IrObject());
        uint8_t n = GetDerefCount(flags);
        for (uint8_t i = 1; i < n; ++i)
        {
            ptr = emitter.Builder().CreateLoad(ptr);
        }
        emitter.Builder().CreateStore(value, ptr);
    }
    else
    {
        emitter.Builder().CreateStore(value, localVariableSymbol->IrObject());
    }
    DestroyTemporaries(emitter);
}

void BoundLocalVariable::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundMemberVariable::BoundMemberVariable(MemberVariableSymbol* memberVariableSymbol_) :
    BoundExpression(memberVariableSymbol_->GetSpan(), BoundNodeType::boundMemberVariable, memberVariableSymbol_->GetType()), memberVariableSymbol(memberVariableSymbol_), staticInitNeeded(false)
{
}

BoundExpression* BoundMemberVariable::Clone()
{
    BoundMemberVariable* clone = new BoundMemberVariable(memberVariableSymbol);
    if (classPtr)
    {
        clone->classPtr.reset(classPtr->Clone());
    }
    if (staticInitNeeded)
    {
        clone->staticInitNeeded = true;
    }
    return clone;
}

void BoundMemberVariable::Load(Emitter& emitter, OperationFlags flags)
{
    Assert(memberVariableSymbol->LayoutIndex() != -1, "layout index of the member variable not set");
    if (memberVariableSymbol->IsStatic())
    {
        ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(memberVariableSymbol->Parent());
        if (staticInitNeeded)
        {
            if (classType->StaticConstructor())
            {
                BoundFunctionCall staticConstructorCall(GetSpan(), classType->StaticConstructor());
                staticConstructorCall.Load(emitter, OperationFlags::none);
            }
        }
        emitter.Stack().Push(classType->StaticObject(emitter, false));
    }
    else
    {
        classPtr->Load(emitter, OperationFlags::none);
    }
    llvm::Value* ptr = emitter.Stack().Pop();
    ArgVector indeces;
    indeces.push_back(emitter.Builder().getInt32(0));
    indeces.push_back(emitter.Builder().getInt32(memberVariableSymbol->LayoutIndex()));
    llvm::Value* memberVariablePtr = emitter.Builder().CreateGEP(ptr, indeces);
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        emitter.Stack().Push(memberVariablePtr);
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        llvm::Value* value = emitter.Builder().CreateLoad(memberVariablePtr);
        uint8_t n = GetDerefCount(flags);
        for (uint8_t i = 0; i < n; ++i)
        {
            value = emitter.Builder().CreateLoad(value);
        }
        emitter.Stack().Push(value);
    }
    else
    {
        emitter.Stack().Push(emitter.Builder().CreateLoad(memberVariablePtr));
    }
    DestroyTemporaries(emitter);
}

void BoundMemberVariable::Store(Emitter& emitter, OperationFlags flags)
{
    Assert(memberVariableSymbol->LayoutIndex() != -1, "layout index of the member variable not set");
    llvm::Value* value = emitter.Stack().Pop();
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot store to the address of a member variable", GetSpan());
    }
    else 
    {
        if (memberVariableSymbol->IsStatic())
        {
            ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(memberVariableSymbol->Parent());
            if (staticInitNeeded)
            {
                if (classType->StaticConstructor())
                {
                    BoundFunctionCall staticConstructorCall(GetSpan(), classType->StaticConstructor());
                    staticConstructorCall.Load(emitter, OperationFlags::none);
                }
            }
            emitter.Stack().Push(classType->StaticObject(emitter, false));
        }
        else
        {
            classPtr->Load(emitter, OperationFlags::none);
        }
        llvm::Value* ptr = emitter.Stack().Pop();
        ArgVector indeces;
        indeces.push_back(emitter.Builder().getInt32(0));
        indeces.push_back(emitter.Builder().getInt32(memberVariableSymbol->LayoutIndex()));
        llvm::Value* memberVariablePtr = emitter.Builder().CreateGEP(ptr, indeces);
        if ((flags & OperationFlags::deref) != OperationFlags::none)
        {
            llvm::Value* ptr = emitter.Builder().CreateLoad(memberVariablePtr);
            uint8_t n = GetDerefCount(flags);
            for (uint8_t i = 1; i < n; ++i)
            {
                ptr = emitter.Builder().CreateLoad(ptr);
            }
            emitter.Builder().CreateStore(value, ptr);
        }
        else
        {
            emitter.Builder().CreateStore(value, memberVariablePtr);
        }
    }
    DestroyTemporaries(emitter);
}

void BoundMemberVariable::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

void BoundMemberVariable::SetClassPtr(std::unique_ptr<BoundExpression>&& classPtr_)
{
    classPtr = std::move(classPtr_);
}

BoundConstant::BoundConstant(ConstantSymbol* constantSymbol_) : BoundExpression(constantSymbol_->GetSpan(), BoundNodeType::boundConstant, constantSymbol_->GetType()), constantSymbol(constantSymbol_)
{
}

BoundExpression* BoundConstant::Clone()
{
    return new BoundConstant(constantSymbol);
}

void BoundConstant::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a constant", GetSpan());
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        throw Exception("cannot dereference a constant", GetSpan());
    }
    else
    {
        emitter.Stack().Push(constantSymbol->GetValue()->IrValue(emitter));
    }
    DestroyTemporaries(emitter);
}

void BoundConstant::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a constant", GetSpan());
}

void BoundConstant::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundEnumConstant::BoundEnumConstant(EnumConstantSymbol* enumConstantSymbol_) : BoundExpression(enumConstantSymbol_->GetSpan(), BoundNodeType::boundEnumConstant, enumConstantSymbol_->GetType()),
    enumConstantSymbol(enumConstantSymbol_)
{
}

BoundExpression* BoundEnumConstant::Clone()
{
    return new BoundEnumConstant(enumConstantSymbol);
}

void BoundEnumConstant::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of an enumeration constant", GetSpan());
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        throw Exception("cannot dereference an enumeration constant", GetSpan());
    }
    else
    {
        emitter.Stack().Push(enumConstantSymbol->GetValue()->IrValue(emitter));
    }
    DestroyTemporaries(emitter);
}

void BoundEnumConstant::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to an enumeration constant", GetSpan());
}

void BoundEnumConstant::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundLiteral::BoundLiteral(std::unique_ptr<Value>&& value_, TypeSymbol* type_) : BoundExpression(value_->GetSpan(), BoundNodeType::boundLiteral, type_), value(std::move(value_))
{
}

BoundExpression* BoundLiteral::Clone()
{
    std::unique_ptr<Value> clonedValue;
    clonedValue.reset(value->Clone());
    return new BoundLiteral(std::move(clonedValue), GetType());
}

void BoundLiteral::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a literal", GetSpan());
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        throw Exception("cannot dereference a literal", GetSpan());
    }
    else
    {
        emitter.Stack().Push(value->IrValue(emitter));
    }
    DestroyTemporaries(emitter);
}

void BoundLiteral::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a literal", GetSpan());
}

void BoundLiteral::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundTemporary::BoundTemporary(std::unique_ptr<BoundExpression>&& rvalueExpr_, std::unique_ptr<BoundLocalVariable>&& backingStore_) :
    BoundExpression(rvalueExpr_->GetSpan(), BoundNodeType::boundTemporary, rvalueExpr_->GetType()), rvalueExpr(std::move(rvalueExpr_)), backingStore(std::move(backingStore_))
{
}

BoundExpression* BoundTemporary::Clone()
{
    std::unique_ptr<BoundExpression> clonedRvalueExpr;
    clonedRvalueExpr.reset(rvalueExpr->Clone());
    std::unique_ptr<BoundLocalVariable> clonedBackingStore;
    clonedBackingStore.reset(static_cast<BoundLocalVariable*>(backingStore->Clone()));
    return new BoundTemporary(std::move(clonedRvalueExpr), std::move(clonedBackingStore));
}

void BoundTemporary::Load(Emitter& emitter, OperationFlags flags)
{
    rvalueExpr->Load(emitter, OperationFlags::none);
    backingStore->Store(emitter, OperationFlags::none);
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        backingStore->Load(emitter, OperationFlags::addr);
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        backingStore->Load(emitter, SetDerefCount(OperationFlags::deref, GetDerefCount(flags) + 1));
    }
    else
    {
        backingStore->Load(emitter, OperationFlags::none);
    }
    DestroyTemporaries(emitter);
}

void BoundTemporary::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a temporary", GetSpan());
}

void BoundTemporary::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundSizeOfExpression::BoundSizeOfExpression(const Span& span_, TypeSymbol* type_, TypeSymbol* pointerType_) : 
    BoundExpression(span_, BoundNodeType::boundSizeOfExpression, type_), pointerType(pointerType_)
{
}

BoundExpression* BoundSizeOfExpression::Clone()
{
    return new BoundSizeOfExpression(GetSpan(), GetType(), pointerType);
}

void BoundSizeOfExpression::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a sizeof expression", GetSpan());
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        throw Exception("cannot dereference a sizeof expression", GetSpan());
    }
    else
    {
        llvm::Value* nullPtr = llvm::Constant::getNullValue(pointerType->IrType(emitter));
        llvm::Value* gep = emitter.Builder().CreateGEP(nullPtr, emitter.Builder().getInt64(1));
        llvm::Value* size = emitter.Builder().CreatePtrToInt(gep, emitter.Builder().getInt64Ty());
        emitter.Stack().Push(size);
    }
    DestroyTemporaries(emitter);
}

void BoundSizeOfExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a sizeof expression", GetSpan());
}

void BoundSizeOfExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundAddressOfExpression::BoundAddressOfExpression(std::unique_ptr<BoundExpression>&& subject_, TypeSymbol* type_)  : 
    BoundExpression(subject_->GetSpan(), BoundNodeType::boundAddressOfExpression, type_), subject(std::move(subject_))
{
}

BoundExpression* BoundAddressOfExpression::Clone()
{
    std::unique_ptr<BoundExpression> clonedSubject;
    clonedSubject.reset(subject->Clone());
    return new BoundAddressOfExpression(std::move(clonedSubject), GetType());
}

void BoundAddressOfExpression::Load(Emitter& emitter, OperationFlags flags)
{
    if (subject->GetBoundNodeType() != BoundNodeType::boundDereferenceExpression)
    {
        subject->Load(emitter, OperationFlags::addr);
    }
    else
    {
        BoundDereferenceExpression* derefExpr = static_cast<BoundDereferenceExpression*>(subject.get());
        derefExpr->Subject()->Load(emitter, flags);
    }
    DestroyTemporaries(emitter);
}

void BoundAddressOfExpression::Store(Emitter& emitter, OperationFlags flags)
{
    if (subject->GetBoundNodeType() != BoundNodeType::boundDereferenceExpression)
    {
        subject->Store(emitter, flags);
    }
    else
    {
        BoundDereferenceExpression* derefExpr = static_cast<BoundDereferenceExpression*>(subject.get());
        derefExpr->Subject()->Store(emitter, flags);
    }
    DestroyTemporaries(emitter);
}

void BoundAddressOfExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundDereferenceExpression::BoundDereferenceExpression(std::unique_ptr<BoundExpression>&& subject_, TypeSymbol* type_) :
    BoundExpression(subject_->GetSpan(), BoundNodeType::boundDereferenceExpression, type_), subject(std::move(subject_))
{
}

BoundExpression* BoundDereferenceExpression::Clone()
{
    std::unique_ptr<BoundExpression> clonedSubject;
    clonedSubject.reset(subject->Clone());
    return new BoundDereferenceExpression(std::move(clonedSubject), GetType());
}

void BoundDereferenceExpression::Load(Emitter& emitter, OperationFlags flags)
{
    if (subject->GetBoundNodeType() != BoundNodeType::boundAddressOfExpression)
    {
        subject->Load(emitter, SetDerefCount(OperationFlags::deref, GetDerefCount(flags) + 1));
    }
    else
    {
        BoundAddressOfExpression* addressOfExpr = static_cast<BoundAddressOfExpression*>(subject.get());
        addressOfExpr->Subject()->Load(emitter, flags);
    }
    DestroyTemporaries(emitter);
}

void BoundDereferenceExpression::Store(Emitter& emitter, OperationFlags flags)
{
    if (subject->GetBoundNodeType() != BoundNodeType::boundAddressOfExpression)
    {
        subject->Store(emitter, SetDerefCount(OperationFlags::deref | (flags & OperationFlags::functionCallFlags), GetDerefCount(flags) + 1));
    }
    else
    {
        BoundAddressOfExpression* addressOfExpr = static_cast<BoundAddressOfExpression*>(subject.get());
        addressOfExpr->Subject()->Store(emitter, flags | (flags & OperationFlags::functionCallFlags));
    }
    DestroyTemporaries(emitter);
}

void BoundDereferenceExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundReferenceToPointerExpression::BoundReferenceToPointerExpression(std::unique_ptr<BoundExpression>&& subject_, TypeSymbol* type_) :
    BoundExpression(subject_->GetSpan(), BoundNodeType::boundReferenceToPointerExpression, type_), subject(std::move(subject_))
{
}

BoundExpression* BoundReferenceToPointerExpression::Clone()
{
    std::unique_ptr<BoundExpression> clonedSubject;
    clonedSubject.reset(subject->Clone());
    return new BoundReferenceToPointerExpression(std::move(clonedSubject), GetType());
}

void BoundReferenceToPointerExpression::Load(Emitter& emitter, OperationFlags flags)
{
    subject->Load(emitter, flags);
    DestroyTemporaries(emitter);
}

void BoundReferenceToPointerExpression::Store(Emitter& emitter, OperationFlags flags)
{
    subject->Store(emitter, flags);
    DestroyTemporaries(emitter);
}

void BoundReferenceToPointerExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundFunctionCall::BoundFunctionCall(const Span& span_, FunctionSymbol* functionSymbol_) : BoundExpression(span_, BoundNodeType::boundFunctionCall, functionSymbol_->ReturnType()), functionSymbol(functionSymbol_)
{
}

BoundExpression* BoundFunctionCall::Clone()
{
    BoundFunctionCall* clone = new BoundFunctionCall(GetSpan(), functionSymbol);
    for (std::unique_ptr<BoundExpression>& argument : arguments)
    {
        clone->AddArgument(std::unique_ptr<BoundExpression>(argument->Clone()));
    }
    return clone;
}

void BoundFunctionCall::AddArgument(std::unique_ptr<BoundExpression>&& argument)
{
    arguments.push_back(std::move(argument));
}

void BoundFunctionCall::SetArguments(std::vector<std::unique_ptr<BoundExpression>>&& arguments_)
{
    arguments = std::move(arguments_);
}

void BoundFunctionCall::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a function call", GetSpan());
    }
    else
    {
        std::vector<GenObject*> genObjects;
        for (const std::unique_ptr<BoundExpression>& argument : arguments)
        {
            genObjects.push_back(argument.get());
            genObjects.back()->SetType(argument->GetType());
        }
        OperationFlags callFlags = flags & OperationFlags::functionCallFlags;
        if (GetFlag(BoundExpressionFlags::virtualCall))
        {
            Assert(!arguments.empty(), "nonempty argument list expected");
            genObjects[0]->SetType(arguments[0]->GetType());
            callFlags = callFlags | OperationFlags::virtualCall;
        }
        if (!functionSymbol->DontThrow())
        {
            emitter.SetLineNumber(GetSpan().LineNumber());
        }
        functionSymbol->GenerateCall(emitter, genObjects, callFlags);
        if ((flags & OperationFlags::deref) != OperationFlags::none)
        {
            llvm::Value* value = emitter.Stack().Pop();
            uint8_t n = GetDerefCount(flags);
            for (uint8_t i = 0; i < n; ++i)
            {
                value = emitter.Builder().CreateLoad(value);
            }
            emitter.Stack().Push(value);
        }
    }
    DestroyTemporaries(emitter);
}

void BoundFunctionCall::Store(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a function call", GetSpan());
    }
    else
    {
        llvm::Value* value = emitter.Stack().Pop();
        std::vector<GenObject*> genObjects;
        for (const std::unique_ptr<BoundExpression>& argument : arguments)
        {
            genObjects.push_back(argument.get());
            genObjects.back()->SetType(argument->GetType());
        }
        OperationFlags callFlags = OperationFlags::none;
        if (GetFlag(BoundExpressionFlags::virtualCall))
        {
            callFlags = callFlags | OperationFlags::virtualCall;
        }
        if (!functionSymbol->DontThrow())
        {
            emitter.SetLineNumber(GetSpan().LineNumber());
        }
        functionSymbol->GenerateCall(emitter, genObjects, callFlags);
        llvm::Value* ptr = emitter.Stack().Pop();
        if ((flags & OperationFlags::leaveFirstArg) != OperationFlags::none)
        {
            emitter.SaveObjectPointer(ptr);
        }
        if ((flags & OperationFlags::deref) != OperationFlags::none || GetFlag(BoundExpressionFlags::deref))
        {
            uint8_t n = GetDerefCount(flags);
            for (uint8_t i = 1; i < n; ++i)
            {
                ptr = emitter.Builder().CreateLoad(ptr);
            }
            emitter.Builder().CreateStore(value, ptr);
        }
        else
        {
            emitter.Builder().CreateStore(emitter.Builder().CreateLoad(value), ptr);
        }
    }
    DestroyTemporaries(emitter);
}

void BoundFunctionCall::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

bool BoundFunctionCall::HasValue() const
{ 
    return functionSymbol->ReturnType() && functionSymbol->ReturnType()->GetSymbolType() != SymbolType::voidTypeSymbol; 
}

bool BoundFunctionCall::IsLvalueExpression() const
{
    TypeSymbol* returnType = functionSymbol->ReturnType();
    if (returnType && returnType->GetSymbolType() != SymbolType::voidTypeSymbol)
    {
        return !returnType->IsConstType() && returnType->IsLvalueReferenceType();
    }
    return false;
}

BoundDelegateCall::BoundDelegateCall(const Span& span_, DelegateTypeSymbol* delegateType_) :
    BoundExpression(span_, BoundNodeType::boundDelegateCall, delegateType_->ReturnType()), delegateTypeSymbol(delegateType_), arguments()
{
}

BoundExpression* BoundDelegateCall::Clone()
{
    return new BoundDelegateCall(GetSpan(), delegateTypeSymbol);
}

void BoundDelegateCall::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a delegate call", GetSpan());
    }
    else
    {
        std::vector<GenObject*> genObjects;
        for (const std::unique_ptr<BoundExpression>& argument : arguments)
        {
            genObjects.push_back(argument.get());
            genObjects.back()->SetType(argument->GetType());
        }
        OperationFlags callFlags = flags & OperationFlags::functionCallFlags;
        if (!delegateTypeSymbol->IsNothrow())
        {
            emitter.SetLineNumber(GetSpan().LineNumber());
        }
        delegateTypeSymbol->GenerateCall(emitter, genObjects, callFlags);
        if ((flags & OperationFlags::deref) != OperationFlags::none)
        {
            llvm::Value* value = emitter.Stack().Pop();
            uint8_t n = GetDerefCount(flags);
            for (uint8_t i = 0; i < n; ++i)
            {
                value = emitter.Builder().CreateLoad(value);
            }
            emitter.Stack().Push(value);
        }
    }
    DestroyTemporaries(emitter);
}

void BoundDelegateCall::Store(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a function call", GetSpan());
    }
    else
    {
        llvm::Value* value = emitter.Stack().Pop();
        std::vector<GenObject*> genObjects;
        for (const std::unique_ptr<BoundExpression>& argument : arguments)
        {
            genObjects.push_back(argument.get());
            genObjects.back()->SetType(argument->GetType());
        }
        OperationFlags callFlags = OperationFlags::none;
        if (GetFlag(BoundExpressionFlags::virtualCall))
        {
            callFlags = callFlags | OperationFlags::virtualCall;
        }
        if (!delegateTypeSymbol->IsNothrow())
        {
            emitter.SetLineNumber(GetSpan().LineNumber());
        }
        delegateTypeSymbol->GenerateCall(emitter, genObjects, callFlags);
        llvm::Value* ptr = emitter.Stack().Pop();
        if ((flags & OperationFlags::leaveFirstArg) != OperationFlags::none)
        {
            emitter.SaveObjectPointer(ptr);
        }
        if ((flags & OperationFlags::deref) != OperationFlags::none || GetFlag(BoundExpressionFlags::deref))
        {
            uint8_t n = GetDerefCount(flags);
            for (uint8_t i = 1; i < n; ++i)
            {
                ptr = emitter.Builder().CreateLoad(ptr);
            }
            emitter.Builder().CreateStore(value, ptr);
        }
        else
        {
            emitter.Builder().CreateStore(emitter.Builder().CreateLoad(value), ptr);
        }
    }
    DestroyTemporaries(emitter);
}

void BoundDelegateCall::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

bool BoundDelegateCall::HasValue() const
{
    return delegateTypeSymbol->ReturnType()->GetSymbolType() != SymbolType::voidTypeSymbol;
}

bool BoundDelegateCall::IsLvalueExpression() const
{
    TypeSymbol* returnType = delegateTypeSymbol->ReturnType();
    if (returnType->GetSymbolType() != SymbolType::voidTypeSymbol)
    {
        return !returnType->IsConstType() && returnType->IsLvalueReferenceType();
    }
    return false;
}

void BoundDelegateCall::AddArgument(std::unique_ptr<BoundExpression>&& argument)
{
    arguments.push_back(std::move(argument));
}

BoundClassDelegateCall::BoundClassDelegateCall(const Span& span_, ClassDelegateTypeSymbol* classDelegateType_) :
    BoundExpression(span_, BoundNodeType::boundClassDelegateCall, classDelegateType_->ReturnType()), classDelegateTypeSymbol(classDelegateType_), arguments()
{
}

BoundExpression* BoundClassDelegateCall::Clone()
{
    return new BoundClassDelegateCall(GetSpan(), classDelegateTypeSymbol);
}

void BoundClassDelegateCall::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a delegate call", GetSpan());
    }
    else
    {
        std::vector<GenObject*> genObjects;
        for (const std::unique_ptr<BoundExpression>& argument : arguments)
        {
            genObjects.push_back(argument.get());
            genObjects.back()->SetType(argument->GetType());
        }
        OperationFlags callFlags = flags & OperationFlags::functionCallFlags;
        if (!classDelegateTypeSymbol->IsNothrow())
        {
            emitter.SetLineNumber(GetSpan().LineNumber());
        }
        classDelegateTypeSymbol->GenerateCall(emitter, genObjects, callFlags);
        if ((flags & OperationFlags::deref) != OperationFlags::none)
        {
            llvm::Value* value = emitter.Stack().Pop();
            uint8_t n = GetDerefCount(flags);
            for (uint8_t i = 0; i < n; ++i)
            {
                value = emitter.Builder().CreateLoad(value);
            }
            emitter.Stack().Push(value);
        }
    }
    DestroyTemporaries(emitter);
}

void BoundClassDelegateCall::Store(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a function call", GetSpan());
    }
    else
    {
        llvm::Value* value = emitter.Stack().Pop();
        std::vector<GenObject*> genObjects;
        for (const std::unique_ptr<BoundExpression>& argument : arguments)
        {
            genObjects.push_back(argument.get());
            genObjects.back()->SetType(argument->GetType());
        }
        OperationFlags callFlags = OperationFlags::none;
        if (GetFlag(BoundExpressionFlags::virtualCall))
        {
            callFlags = callFlags | OperationFlags::virtualCall;
        }
        if (!classDelegateTypeSymbol->IsNothrow())
        {
            emitter.SetLineNumber(GetSpan().LineNumber());
        }
        classDelegateTypeSymbol->GenerateCall(emitter, genObjects, callFlags);
        llvm::Value* ptr = emitter.Stack().Pop();
        if ((flags & OperationFlags::leaveFirstArg) != OperationFlags::none)
        {
            emitter.SaveObjectPointer(ptr);
        }
        if ((flags & OperationFlags::deref) != OperationFlags::none || GetFlag(BoundExpressionFlags::deref))
        {
            uint8_t n = GetDerefCount(flags);
            for (uint8_t i = 1; i < n; ++i)
            {
                ptr = emitter.Builder().CreateLoad(ptr);
            }
            emitter.Builder().CreateStore(value, ptr);
        }
        else
        {
            emitter.Builder().CreateStore(emitter.Builder().CreateLoad(value), ptr);
        }
    }
    DestroyTemporaries(emitter);
}

void BoundClassDelegateCall::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

bool BoundClassDelegateCall::HasValue() const
{
    return classDelegateTypeSymbol->ReturnType()->GetSymbolType() != SymbolType::voidTypeSymbol;
}

bool BoundClassDelegateCall::IsLvalueExpression() const
{
    TypeSymbol* returnType = classDelegateTypeSymbol->ReturnType();
    if (returnType->GetSymbolType() != SymbolType::voidTypeSymbol)
    {
        return !returnType->IsConstType() && returnType->IsLvalueReferenceType();
    }
    return false;
}

void BoundClassDelegateCall::AddArgument(std::unique_ptr<BoundExpression>&& argument)
{
    arguments.push_back(std::move(argument));
}

BoundConstructExpression::BoundConstructExpression(std::unique_ptr<BoundExpression>&& constructorCall_, TypeSymbol* resultType_) :
    BoundExpression(constructorCall_->GetSpan(), BoundNodeType::boundConstructExpression, resultType_), constructorCall(std::move(constructorCall_))
{
}

BoundExpression* BoundConstructExpression::Clone()
{
    std::unique_ptr<BoundExpression> clonedConstructorCall;
    clonedConstructorCall.reset(constructorCall->Clone());
    return new BoundConstructExpression(std::move(clonedConstructorCall), GetType());
}

void BoundConstructExpression::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a construct expression", GetSpan());
    }
    else
    {
        constructorCall->Load(emitter, OperationFlags::leaveFirstArg);
        llvm::Value* objectPointer = emitter.GetObjectPointer();
        if (!objectPointer)
        {
            throw Exception("does not have object pointer", GetSpan());
        }
        else
        {
            emitter.Stack().Push(objectPointer);
            emitter.ResetObjectPointer();
        }
    }
    DestroyTemporaries(emitter);
}

void BoundConstructExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to construct expression", GetSpan());
}

void BoundConstructExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundConstructAndReturnTemporaryExpression::BoundConstructAndReturnTemporaryExpression(std::unique_ptr<BoundExpression>&& constructorCall_, std::unique_ptr<BoundExpression>&& boundTemporary_) : 
    BoundExpression(constructorCall_->GetSpan(), BoundNodeType::boundConstructAndReturnTemporary, boundTemporary_->GetType()), constructorCall(std::move(constructorCall_)), 
    boundTemporary(std::move(boundTemporary_))
{
}

BoundExpression* BoundConstructAndReturnTemporaryExpression::Clone()
{
    return new BoundConstructAndReturnTemporaryExpression(std::unique_ptr<BoundExpression>(constructorCall->Clone()), std::unique_ptr<BoundExpression>(boundTemporary->Clone()));
}

void BoundConstructAndReturnTemporaryExpression::Load(Emitter& emitter, OperationFlags flags)
{
    constructorCall->Load(emitter, OperationFlags::none);
    boundTemporary->Load(emitter, flags);
    DestroyTemporaries(emitter);
}

void BoundConstructAndReturnTemporaryExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to construct and return temporary expression", GetSpan());
}

void BoundConstructAndReturnTemporaryExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundConversion::BoundConversion(std::unique_ptr<BoundExpression>&& sourceExpr_, FunctionSymbol* conversionFun_) :
    BoundExpression(sourceExpr_->GetSpan(), BoundNodeType::boundConversion, conversionFun_->ConversionTargetType()), sourceExpr(std::move(sourceExpr_)), conversionFun(conversionFun_)
{
}

BoundExpression* BoundConversion::Clone()
{
    std::unique_ptr<BoundExpression> clonedSourceExpr;
    clonedSourceExpr.reset(sourceExpr->Clone());
    return new BoundConversion(std::move(clonedSourceExpr), conversionFun);
}

void BoundConversion::Load(Emitter& emitter, OperationFlags flags)
{
    sourceExpr->Load(emitter, flags);
    std::vector<GenObject*> emptyObjects;
    conversionFun->GenerateCall(emitter, emptyObjects, OperationFlags::none);
    DestroyTemporaries(emitter);
}

void BoundConversion::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a conversion", GetSpan());
}

bool BoundConversion::IsLvalueExpression() const
{
    if (conversionFun->GetSymbolType() == SymbolType::conversionFunctionSymbol) return true;
    return false;
}

void BoundConversion::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundIsExpression::BoundIsExpression(std::unique_ptr<BoundExpression>&& expr_, ClassTypeSymbol* rightClassType_, TypeSymbol* boolType_) :
    BoundExpression(expr_->GetSpan(), BoundNodeType::boundIsExpression, boolType_), expr(std::move(expr_)), rightClassType(rightClassType_)
{
}

BoundExpression* BoundIsExpression::Clone()
{
    std::unique_ptr<BoundExpression> clonedExpr;
    clonedExpr.reset(expr->Clone());
    return new BoundIsExpression(std::move(clonedExpr), rightClassType, GetType());
}

void BoundIsExpression::Load(Emitter& emitter, OperationFlags flags)
{
    expr->Load(emitter, OperationFlags::none);
    llvm::Value* thisPtr = emitter.Stack().Pop();
    TypeSymbol* exprType = static_cast<TypeSymbol*>(expr->GetType());
    Assert(exprType->IsPointerType(), "pointer type expected");
    TypeSymbol* leftType = exprType->RemovePointer(GetSpan());
    Assert(leftType->IsClassTypeSymbol(), "class type expected");
    ClassTypeSymbol* leftClassType = static_cast<ClassTypeSymbol*>(leftType);
    ClassTypeSymbol* leftVmtPtrHolderClass = leftClassType->VmtPtrHolderClass();
    if (leftClassType != leftVmtPtrHolderClass)
    {
        thisPtr = emitter.Builder().CreateBitCast(thisPtr, leftVmtPtrHolderClass->AddPointer(GetSpan())->IrType(emitter));
    }
    ArgVector vmtPtrIndeces;
    vmtPtrIndeces.push_back(emitter.Builder().getInt32(0));
    vmtPtrIndeces.push_back(emitter.Builder().getInt32(leftVmtPtrHolderClass->VmtPtrIndex()));
    llvm::Value* vmtPtrPtr = emitter.Builder().CreateGEP(thisPtr, vmtPtrIndeces);
    llvm::Value* vmtPtr = emitter.Builder().CreateBitCast(emitter.Builder().CreateLoad(vmtPtrPtr), leftClassType->VmtPtrType(emitter));
    ArgVector indeces;
    indeces.push_back(emitter.Builder().getInt32(0));
    indeces.push_back(emitter.Builder().getInt32(0));
    llvm::Value* leftClassIdPtr = emitter.Builder().CreateGEP(vmtPtr, indeces);
    llvm::Value* leftClassId = emitter.Builder().CreatePtrToInt(emitter.Builder().CreateLoad(leftClassIdPtr), emitter.Builder().getInt64Ty());
    llvm::Value* rightClassTypeVmtObject = rightClassType->VmtObject(emitter, false);
    llvm::Value* rightClassIdPtr = emitter.Builder().CreateGEP(rightClassTypeVmtObject, indeces);
    llvm::Value* rightClassId = emitter.Builder().CreatePtrToInt(emitter.Builder().CreateLoad(rightClassIdPtr), emitter.Builder().getInt64Ty());
    llvm::Value* remainder = emitter.Builder().CreateURem(leftClassId, rightClassId);
    llvm::Value* remainderIsZero = emitter.Builder().CreateICmpEQ(remainder, emitter.Builder().getInt64(0));
    emitter.Stack().Push(remainderIsZero);
    DestroyTemporaries(emitter);
}

void BoundIsExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a 'is' expression", GetSpan());
}

void BoundIsExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundAsExpression::BoundAsExpression(std::unique_ptr<BoundExpression>&& expr_, ClassTypeSymbol* rightClassType_, std::unique_ptr<BoundLocalVariable>&& variable_) :
    BoundExpression(expr_->GetSpan(), BoundNodeType::boundAsExpression, rightClassType_->AddPointer(expr_->GetSpan())), 
    expr(std::move(expr_)), rightClassType(rightClassType_), variable(std::move(variable_))
{
}

BoundExpression* BoundAsExpression::Clone()
{
    std::unique_ptr<BoundExpression> clonedExpr;
    clonedExpr.reset(expr->Clone());
    std::unique_ptr<BoundLocalVariable> clonedVariable;
    clonedVariable.reset(static_cast<BoundLocalVariable*>(variable->Clone()));
    return new BoundAsExpression(std::move(clonedExpr), rightClassType, std::move(clonedVariable));
}

void BoundAsExpression::Load(Emitter& emitter, OperationFlags flags)
{
    expr->Load(emitter, OperationFlags::none);
    llvm::Value* thisPtr = emitter.Stack().Pop();
    TypeSymbol* exprType = static_cast<TypeSymbol*>(expr->GetType());
    Assert(exprType->IsPointerType(), "pointer type expected");
    TypeSymbol* leftType = exprType->RemovePointer(GetSpan());
    Assert(leftType->IsClassTypeSymbol(), "class type expected");
    ClassTypeSymbol* leftClassType = static_cast<ClassTypeSymbol*>(leftType);
    ClassTypeSymbol* leftVmtPtrHolderClass = leftClassType->VmtPtrHolderClass();
    if (leftClassType != leftVmtPtrHolderClass)
    {
        thisPtr = emitter.Builder().CreateBitCast(thisPtr, leftVmtPtrHolderClass->AddPointer(GetSpan())->IrType(emitter));
    }
    ArgVector vmtPtrIndeces;
    vmtPtrIndeces.push_back(emitter.Builder().getInt32(0));
    vmtPtrIndeces.push_back(emitter.Builder().getInt32(leftVmtPtrHolderClass->VmtPtrIndex()));
    llvm::Value* vmtPtrPtr = emitter.Builder().CreateGEP(thisPtr, vmtPtrIndeces);
    llvm::Value* vmtPtr = emitter.Builder().CreateBitCast(emitter.Builder().CreateLoad(vmtPtrPtr), leftClassType->VmtPtrType(emitter));
    ArgVector indeces;
    indeces.push_back(emitter.Builder().getInt32(0));
    indeces.push_back(emitter.Builder().getInt32(0));
    llvm::Value* leftClassIdPtr = emitter.Builder().CreateGEP(vmtPtr, indeces);
    llvm::Value* leftClassId = emitter.Builder().CreatePtrToInt(emitter.Builder().CreateLoad(leftClassIdPtr), emitter.Builder().getInt64Ty());
    llvm::Value* rightClassTypeVmtObject = rightClassType->VmtObject(emitter, false);
    llvm::Value* rightClassIdPtr = emitter.Builder().CreateGEP(rightClassTypeVmtObject, indeces);
    llvm::Value* rightClassId = emitter.Builder().CreatePtrToInt(emitter.Builder().CreateLoad(rightClassIdPtr), emitter.Builder().getInt64Ty());
    llvm::Value* remainder = emitter.Builder().CreateURem(leftClassId, rightClassId);
    llvm::Value* remainderIsZero = emitter.Builder().CreateICmpEQ(remainder, emitter.Builder().getInt64(0));
    llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(emitter.Context(), "true", emitter.Function());
    llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(emitter.Context(), "false", emitter.Function());
    llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(emitter.Context(), "continue", emitter.Function());
    emitter.Builder().CreateCondBr(remainderIsZero, trueBlock, falseBlock);
    emitter.SetCurrentBasicBlock(trueBlock);
    emitter.Stack().Push(emitter.Builder().CreateBitCast(thisPtr, rightClassType->AddPointer(GetSpan())->IrType(emitter)));
    variable->Store(emitter, OperationFlags::none);
    emitter.Builder().CreateBr(continueBlock);
    emitter.SetCurrentBasicBlock(falseBlock);
    emitter.Stack().Push(llvm::Constant::getNullValue(rightClassType->AddPointer(GetSpan())->IrType(emitter)));
    variable->Store(emitter, OperationFlags::none);
    emitter.Builder().CreateBr(continueBlock);
    emitter.SetCurrentBasicBlock(continueBlock);
    variable->Load(emitter, OperationFlags::none);
    DestroyTemporaries(emitter);
}

void BoundAsExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to an 'as' expression", GetSpan());
}

void BoundAsExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundTypeNameExpression::BoundTypeNameExpression(std::unique_ptr<BoundExpression>&& classPtr_, TypeSymbol* constCharPtrType_) :
    BoundExpression(classPtr_->GetSpan(), BoundNodeType::boundTypeNameExpression, constCharPtrType_), classPtr(std::move(classPtr_))
{
}

BoundExpression* BoundTypeNameExpression::Clone()
{
    std::unique_ptr<BoundExpression> clonedClassPtr;
    clonedClassPtr.reset(classPtr->Clone());
    return new BoundTypeNameExpression(std::move(clonedClassPtr), GetType());
}

void BoundTypeNameExpression::Load(Emitter& emitter, OperationFlags flags)
{
    classPtr->Load(emitter, OperationFlags::none);
    llvm::Value* thisPtr = emitter.Stack().Pop();
    TypeSymbol* classPtrType = static_cast<TypeSymbol*>(classPtr->GetType());
    Assert(classPtrType->IsPointerType(), "pointer type expected");
    TypeSymbol* type = classPtrType->BaseType();
    Assert(type->IsClassTypeSymbol(), "class type expected");
    ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(type);
    ClassTypeSymbol* vmtPtrHolderClass = classType->VmtPtrHolderClass();
    if (classType != vmtPtrHolderClass)
    {
        thisPtr = emitter.Builder().CreateBitCast(thisPtr, vmtPtrHolderClass->AddPointer(GetSpan())->IrType(emitter));
    }
    ArgVector vmtPtrIndeces;
    vmtPtrIndeces.push_back(emitter.Builder().getInt32(0));
    vmtPtrIndeces.push_back(emitter.Builder().getInt32(vmtPtrHolderClass->VmtPtrIndex()));
    llvm::Value* vmtPtrPtr = emitter.Builder().CreateGEP(thisPtr, vmtPtrIndeces);
    llvm::Value* vmtPtr = emitter.Builder().CreateBitCast(emitter.Builder().CreateLoad(vmtPtrPtr), classType->VmtPtrType(emitter));
    ArgVector indeces;
    indeces.push_back(emitter.Builder().getInt32(0));
    indeces.push_back(emitter.Builder().getInt32(1));
    llvm::Value* classNamePtr = emitter.Builder().CreateGEP(vmtPtr, indeces);
    llvm::Value* className = emitter.Builder().CreateLoad(classNamePtr);
    emitter.Stack().Push(className);
    DestroyTemporaries(emitter);
}

void BoundTypeNameExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to typename expression", GetSpan());
}

void BoundTypeNameExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundBitCast::BoundBitCast(std::unique_ptr<BoundExpression>&& expr_, TypeSymbol* type_) : BoundExpression(expr_->GetSpan(), BoundNodeType::boundBitCast, type_), expr(std::move(expr_))
{
}

BoundExpression* BoundBitCast::Clone()
{
    return new BoundBitCast(std::unique_ptr<BoundExpression>(expr->Clone()), GetType());
}

void BoundBitCast::Load(Emitter& emitter, OperationFlags flags)
{
    expr->Load(emitter, OperationFlags::none);
    llvm::Value* value = emitter.Stack().Pop();
    llvm::Value* casted = emitter.Builder().CreateBitCast(value, GetType()->IrType(emitter));
    emitter.Stack().Push(casted);
    DestroyTemporaries(emitter);
}

void BoundBitCast::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to bit cast", GetSpan());
}

void BoundBitCast::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundFunctionPtr::BoundFunctionPtr(const Span& span_, FunctionSymbol* function_, TypeSymbol* type_) : BoundExpression(span_, BoundNodeType::boundFunctionPtr, type_), function(function_)
{
}

BoundExpression* BoundFunctionPtr::Clone()
{
    return new BoundFunctionPtr(GetSpan(), function, GetType());
}

void BoundFunctionPtr::Load(Emitter& emitter, OperationFlags flags)
{
    llvm::Value* irObject = emitter.Module()->getOrInsertFunction(ToUtf8(function->MangledName()), function->IrType(emitter));
    emitter.Stack().Push(irObject);
    DestroyTemporaries(emitter);
}

void BoundFunctionPtr::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to function ptr expression", GetSpan());
}

void BoundFunctionPtr::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundDisjunction::BoundDisjunction(const Span& span_, std::unique_ptr<BoundExpression>&& left_, std::unique_ptr<BoundExpression>&& right_, TypeSymbol* boolType_) :
    BoundExpression(span_, BoundNodeType::boundDisjunction, boolType_), left(std::move(left_)), right(std::move(right_))
{
}

BoundExpression* BoundDisjunction::Clone()
{
    return new BoundDisjunction(GetSpan(), std::unique_ptr<BoundExpression>(left->Clone()), std::unique_ptr<BoundExpression>(right->Clone()), GetType());
}

void BoundDisjunction::Load(Emitter& emitter, OperationFlags flags)
{
    temporary->Load(emitter, OperationFlags::addr);
    llvm::Value* temp = emitter.Stack().Pop();
    left->Load(emitter, OperationFlags::none);
    llvm::Value* leftValue = emitter.Stack().Pop();
    llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(emitter.Context(), "true", emitter.Function());
    llvm::BasicBlock* rightBlock = llvm::BasicBlock::Create(emitter.Context(), "right", emitter.Function());
    llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(emitter.Context(), "false", emitter.Function());
    llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(emitter.Context(), "next", emitter.Function());
    emitter.Builder().CreateCondBr(leftValue, trueBlock, rightBlock);
    emitter.SetCurrentBasicBlock(rightBlock);
    right->Load(emitter, OperationFlags::none);
    llvm::Value* rightValue = emitter.Stack().Pop();
    emitter.Builder().CreateCondBr(rightValue, trueBlock, falseBlock);
    emitter.SetCurrentBasicBlock(trueBlock);
    emitter.Builder().CreateStore(emitter.Builder().getInt1(true), temp);
    emitter.Builder().CreateBr(nextBlock);
    emitter.SetCurrentBasicBlock(falseBlock);
    emitter.Builder().CreateStore(emitter.Builder().getInt1(false), temp);
    emitter.Builder().CreateBr(nextBlock);
    emitter.SetCurrentBasicBlock(nextBlock);
    llvm::Value* value = emitter.Builder().CreateLoad(temp);
    emitter.Stack().Push(value);
    DestroyTemporaries(emitter);
}

void BoundDisjunction::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to disjunction", GetSpan());
}

void BoundDisjunction::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

void BoundDisjunction::SetTemporary(BoundLocalVariable* temporary_)
{
    temporary.reset(temporary_);
}

BoundConjunction::BoundConjunction(const Span& span_, std::unique_ptr<BoundExpression>&& left_, std::unique_ptr<BoundExpression>&& right_, TypeSymbol* boolType_) :
    BoundExpression(span_, BoundNodeType::boundConjunction, boolType_), left(std::move(left_)), right(std::move(right_))
{
}

BoundExpression* BoundConjunction::Clone()
{
    return new BoundConjunction(GetSpan(), std::unique_ptr<BoundExpression>(left->Clone()), std::unique_ptr<BoundExpression>(right->Clone()), GetType());
}

void BoundConjunction::Load(Emitter& emitter, OperationFlags flags)
{
    temporary->Load(emitter, OperationFlags::addr);
    llvm::Value* temp = emitter.Stack().Pop();
    left->Load(emitter, OperationFlags::none);
    llvm::Value* leftValue = emitter.Stack().Pop();
    llvm::BasicBlock* trueBlock = llvm::BasicBlock::Create(emitter.Context(), "true", emitter.Function());
    llvm::BasicBlock* rightBlock = llvm::BasicBlock::Create(emitter.Context(), "right", emitter.Function());
    llvm::BasicBlock* falseBlock = llvm::BasicBlock::Create(emitter.Context(), "false", emitter.Function());
    llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(emitter.Context(), "next", emitter.Function());
    emitter.Builder().CreateCondBr(leftValue, rightBlock, falseBlock);
    emitter.SetCurrentBasicBlock(rightBlock);
    right->Load(emitter, OperationFlags::none);
    llvm::Value* rightValue = emitter.Stack().Pop();
    emitter.Builder().CreateCondBr(rightValue, trueBlock, falseBlock);
    emitter.SetCurrentBasicBlock(trueBlock);
    emitter.Builder().CreateStore(emitter.Builder().getInt1(true), temp);
    emitter.Builder().CreateBr(nextBlock);
    emitter.SetCurrentBasicBlock(falseBlock);
    emitter.Builder().CreateStore(emitter.Builder().getInt1(false), temp);
    emitter.Builder().CreateBr(nextBlock);
    emitter.SetCurrentBasicBlock(nextBlock);
    llvm::Value* value = emitter.Builder().CreateLoad(temp);
    emitter.Stack().Push(value);
    DestroyTemporaries(emitter);
}

void BoundConjunction::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to conjunction", GetSpan());
}

void BoundConjunction::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

void BoundConjunction::SetTemporary(BoundLocalVariable* temporary_)
{
    temporary.reset(temporary_);
}

BoundTypeExpression::BoundTypeExpression(const Span& span_, TypeSymbol* type_) : BoundExpression(span_, BoundNodeType::boundTypeExpression, type_)
{
}

BoundExpression* BoundTypeExpression::Clone()
{
    return new BoundTypeExpression(GetSpan(), GetType());
}

void BoundTypeExpression::Load(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot load from a type", GetSpan());
}

void BoundTypeExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a type", GetSpan());
}

void BoundTypeExpression::Accept(BoundNodeVisitor& visitor)
{
    throw Exception("cannot visit a type", GetSpan());
}

BoundNamespaceExpression::BoundNamespaceExpression(const Span& span_, NamespaceSymbol* ns_) : BoundExpression(span_, BoundNodeType::boundNamespaceExpression, new NamespaceTypeSymbol(ns_)), ns(ns_)
{
    nsType.reset(GetType());
}

BoundExpression* BoundNamespaceExpression::Clone()
{
    return new BoundNamespaceExpression(GetSpan(), ns);
}

void BoundNamespaceExpression::Load(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot load from a namespace", GetSpan());
}

void BoundNamespaceExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a namespace", GetSpan());
}

void BoundNamespaceExpression::Accept(BoundNodeVisitor& visitor)
{
    throw Exception("cannot visit a namespace", GetSpan());
}

BoundFunctionGroupExpression::BoundFunctionGroupExpression(const Span& span_, FunctionGroupSymbol* functionGroupSymbol_) : 
    BoundExpression(span_, BoundNodeType::boundFunctionGroupExpression, new FunctionGroupTypeSymbol(functionGroupSymbol_, this)), 
    functionGroupSymbol(functionGroupSymbol_), scopeQualified(false), qualifiedScope(nullptr)
{
    functionGroupType.reset(GetType());
}

BoundExpression* BoundFunctionGroupExpression::Clone()
{
    BoundFunctionGroupExpression* clone = new BoundFunctionGroupExpression(GetSpan(), functionGroupSymbol);
    if (classPtr)
    {
        clone->classPtr.reset(classPtr->Clone());
    }
    clone->scopeQualified = scopeQualified;
    clone->qualifiedScope = qualifiedScope;
    return clone;
}

void BoundFunctionGroupExpression::Load(Emitter& emitter, OperationFlags flags)
{
    if (classPtr)
    {
        classPtr->Load(emitter, flags);
    }
    else
    {
        emitter.Stack().Push(nullptr);
    }
}

void BoundFunctionGroupExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a function group", GetSpan());
}

void BoundFunctionGroupExpression::Accept(BoundNodeVisitor& visitor)
{
    throw Exception("cannot visit a function group", GetSpan());
}

void BoundFunctionGroupExpression::SetClassPtr(std::unique_ptr<BoundExpression>&& classPtr_)
{
    classPtr = std::move(classPtr_);
}

void BoundFunctionGroupExpression::SetTemplateArgumentTypes(const std::vector<TypeSymbol*>& templateArgumentTypes_)
{
    templateArgumentTypes = templateArgumentTypes_;
}

BoundMemberExpression::BoundMemberExpression(const Span& span_, std::unique_ptr<BoundExpression>&& classPtr_, std::unique_ptr<BoundExpression>&& member_) :
    BoundExpression(span_, BoundNodeType::boundMemberExpression, new MemberExpressionTypeSymbol(span_, member_->GetType()->Name(), this)), classPtr(std::move(classPtr_)), member(std::move(member_))
{
    memberExpressionType.reset(GetType());
}

BoundExpression* BoundMemberExpression::Clone()
{
    return new BoundMemberExpression(GetSpan(), std::unique_ptr<BoundExpression>(classPtr->Clone()), std::unique_ptr<BoundExpression>(member->Clone()));
}

void BoundMemberExpression::Load(Emitter& emitter, OperationFlags flags)
{
    if (classPtr)
    {
        classPtr->Load(emitter, flags);
    }
    else
    {
        emitter.Stack().Push(nullptr);
    }
}

void BoundMemberExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a member expression", GetSpan());
}

void BoundMemberExpression::Accept(BoundNodeVisitor& visitor)
{
    throw Exception("cannot visit a member expression", GetSpan());
}

} } // namespace cmajor::binder
