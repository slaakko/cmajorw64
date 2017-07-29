// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/BoundExpression.hpp>
#include <cmajor/binder/TypeResolver.hpp>
#include <cmajor/binder/BoundNodeVisitor.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/EnumSymbol.hpp>
#include <cmajor/ir/Emitter.hpp>

namespace cmajor { namespace binder {

BoundExpression::BoundExpression(const Span& span_, BoundNodeType boundNodeType_, TypeSymbol* type_) : BoundNode(span_, boundNodeType_), type(type_)
{
}

BoundParameter::BoundParameter(ParameterSymbol* parameterSymbol_) : 
    BoundExpression(parameterSymbol_->GetSpan(), BoundNodeType::boundParameter, parameterSymbol_->GetType()), parameterSymbol(parameterSymbol_)
{
}

void BoundParameter::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        throw Exception("cannot take address of a parameter", GetSpan());
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        emitter.Stack().Push(emitter.Builder().CreateLoad(emitter.Builder().CreateLoad(parameterSymbol->IrObject())));
    }
    else
    {
        emitter.Stack().Push(emitter.Builder().CreateLoad(parameterSymbol->IrObject()));
    }
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
        emitter.Builder().CreateStore(value, emitter.Builder().CreateLoad(parameterSymbol->IrObject()));
    }
    else
    {
        emitter.Builder().CreateStore(value, parameterSymbol->IrObject());
    }
}

void BoundParameter::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundLocalVariable::BoundLocalVariable(LocalVariableSymbol* localVariableSymbol_) : 
    BoundExpression(localVariableSymbol_->GetSpan(), BoundNodeType::boundLocalVariable, localVariableSymbol_->GetType()), localVariableSymbol(localVariableSymbol_)
{
}

void BoundLocalVariable::Load(Emitter& emitter, OperationFlags flags)
{
    if ((flags & OperationFlags::addr) != OperationFlags::none)
    {
        emitter.Stack().Push(localVariableSymbol->IrObject());
    }
    else if ((flags & OperationFlags::deref) != OperationFlags::none)
    {
        emitter.Stack().Push(emitter.Builder().CreateLoad(emitter.Builder().CreateLoad(localVariableSymbol->IrObject())));
    }
    else
    {
        emitter.Stack().Push(emitter.Builder().CreateLoad(localVariableSymbol->IrObject()));
    }
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
        emitter.Builder().CreateStore(value, emitter.Builder().CreateLoad(localVariableSymbol->IrObject()));
    }
    else
    {
        emitter.Builder().CreateStore(value, localVariableSymbol->IrObject());
    }
}

void BoundLocalVariable::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundMemberVariable::BoundMemberVariable(MemberVariableSymbol* memberVariableSymbol_) :
    BoundExpression(memberVariableSymbol_->GetSpan(), BoundNodeType::boundMemberVariable, memberVariableSymbol_->GetType()), memberVariableSymbol(memberVariableSymbol_), staticInitNeeded(false)
{
}

void BoundMemberVariable::Load(Emitter& emitter, OperationFlags flags)
{
    Assert(memberVariableSymbol->LayoutIndex() != -1, "layout index of the member variable not set");
    classPtr->Load(emitter, OperationFlags::none);
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
        emitter.Stack().Push(emitter.Builder().CreateLoad(emitter.Builder().CreateLoad(memberVariablePtr)));
    }
    else
    {
        emitter.Stack().Push(emitter.Builder().CreateLoad(memberVariablePtr));
    }
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
        classPtr->Load(emitter, OperationFlags::none);
        llvm::Value* ptr = emitter.Stack().Pop();
        ArgVector indeces;
        indeces.push_back(emitter.Builder().getInt32(0));
        indeces.push_back(emitter.Builder().getInt32(memberVariableSymbol->LayoutIndex()));
        llvm::Value* memberVariablePtr = emitter.Builder().CreateGEP(ptr, indeces);
        if ((flags & OperationFlags::deref) != OperationFlags::none)
        {
            emitter.Builder().CreateStore(value, emitter.Builder().CreateLoad(memberVariablePtr));
        }
        else
        {
            emitter.Builder().CreateStore(value, memberVariablePtr);
        }
    }
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
        backingStore->Load(emitter, OperationFlags::deref);
    }
    else
    {
        backingStore->Load(emitter, OperationFlags::none);
    }
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
}

void BoundAddressOfExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundDereferenceExpression::BoundDereferenceExpression(std::unique_ptr<BoundExpression>&& subject_, TypeSymbol* type_) :
    BoundExpression(subject_->GetSpan(), BoundNodeType::boundDereferenceExpression, type_), subject(std::move(subject_))
{
}

void BoundDereferenceExpression::Load(Emitter& emitter, OperationFlags flags)
{
    if (subject->GetBoundNodeType() != BoundNodeType::boundAddressOfExpression)
    {
        subject->Load(emitter, OperationFlags::deref);
    }
    else
    {
        BoundAddressOfExpression* addressOfExpr = static_cast<BoundAddressOfExpression*>(subject.get());
        addressOfExpr->Subject()->Load(emitter, flags);
    }
}

void BoundDereferenceExpression::Store(Emitter& emitter, OperationFlags flags)
{
    if (subject->GetBoundNodeType() != BoundNodeType::boundAddressOfExpression)
    {
        subject->Store(emitter, OperationFlags::deref | (flags & OperationFlags::functionCallFlags));
    }
    else
    {
        BoundAddressOfExpression* addressOfExpr = static_cast<BoundAddressOfExpression*>(subject.get());
        addressOfExpr->Subject()->Store(emitter, flags | (flags & OperationFlags::functionCallFlags));
    }
}

void BoundDereferenceExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundFunctionCall::BoundFunctionCall(const Span& span_, FunctionSymbol* functionSymbol_) : BoundExpression(span_, BoundNodeType::boundFunctionCall, functionSymbol_->ReturnType()), functionSymbol(functionSymbol_)
{
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
        }
        functionSymbol->GenerateCall(emitter, genObjects, flags & OperationFlags::functionCallFlags);
        if ((flags & OperationFlags::deref) != OperationFlags::none)
        {
            emitter.Stack().Push(emitter.Builder().CreateLoad(emitter.Stack().Pop()));
        }
    }
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
        }
        functionSymbol->GenerateCall(emitter, genObjects, OperationFlags::none);
        llvm::Value* ptr = emitter.Stack().Pop();
        if ((flags & OperationFlags::leaveFirstArg) != OperationFlags::none)
        {
            emitter.SaveObjectPointer(ptr);
        }
        if ((flags & OperationFlags::deref) != OperationFlags::none)
        {
            emitter.Builder().CreateStore(value, ptr);
        }
        else
        {
            emitter.Builder().CreateStore(emitter.Builder().CreateLoad(value), ptr);
        }
    }
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

BoundConstructExpression::BoundConstructExpression(std::unique_ptr<BoundExpression>&& constructorCall_, TypeSymbol* resultType_) :
    BoundExpression(constructorCall_->GetSpan(), BoundNodeType::boundConstructExpression, resultType_), constructorCall(std::move(constructorCall_))
{
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
}

void BoundConstructExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to construct expression", GetSpan());
}

void BoundConstructExpression::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundConversion::BoundConversion(std::unique_ptr<BoundExpression>&& sourceExpr_, FunctionSymbol* conversionFun_) :
    BoundExpression(sourceExpr_->GetSpan(), BoundNodeType::boundConversion, conversionFun_->ConversionTargetType()), sourceExpr(std::move(sourceExpr_)), conversionFun(conversionFun_)
{
}

void BoundConversion::Load(Emitter& emitter, OperationFlags flags)
{
    sourceExpr->Load(emitter, flags);
    std::vector<GenObject*> emptyObjects;
    conversionFun->GenerateCall(emitter, emptyObjects, OperationFlags::none);
}

void BoundConversion::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a conversion", GetSpan());
}

void BoundConversion::Accept(BoundNodeVisitor& visitor)
{
    visitor.Visit(*this);
}

BoundTypeExpression::BoundTypeExpression(const Span& span_, TypeSymbol* type_) : BoundExpression(span_, BoundNodeType::boundTypeExpression, type_)
{
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
    BoundExpression(span_, BoundNodeType::boundFunctionGroupExcpression, new FunctionGroupTypeSymbol(functionGroupSymbol_)), functionGroupSymbol(functionGroupSymbol_), scopeQualified(false), qualifiedScope(nullptr)
{
    functionGroupType.reset(GetType());
}

void BoundFunctionGroupExpression::Load(Emitter& emitter, OperationFlags flags)
{
    // Fun2Dlg conversion does not need source value, so this implementation is intentionally left empty.
}

void BoundFunctionGroupExpression::Store(Emitter& emitter, OperationFlags flags)
{
    throw Exception("cannot store to a function group", GetSpan());
}

void BoundFunctionGroupExpression::Accept(BoundNodeVisitor& visitor)
{
    throw Exception("cannot visit a function group", GetSpan());
}

void BoundFunctionGroupExpression::SetClassObject(std::unique_ptr<BoundExpression>&& classObject_)
{
    classObject = std::move(classObject_);
}

} } // namespace cmajor::binder
