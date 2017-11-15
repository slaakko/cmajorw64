// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/DelegateSymbol.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/SymbolCollector.hpp>
#include <cmajor/util/Unicode.hpp>
#include <llvm/IR/Module.h>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

DelegateTypeSymbol::DelegateTypeSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::delegateTypeSymbol, span_, name_), returnType(), parameters(), irType(nullptr)
{
}

void DelegateTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    uint32_t returnTypeId = returnType->TypeId();
    writer.GetBinaryWriter().Write(returnTypeId);
    bool hasReturnParam = returnParam != nullptr;
    writer.GetBinaryWriter().Write(hasReturnParam);
    if (hasReturnParam)
    {
        writer.Write(returnParam.get());
    }
}

void DelegateTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    uint32_t returnTypeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, returnTypeId, 0);
    bool hasReturnParam = reader.GetBinaryReader().ReadBool();
    if (hasReturnParam)
    {
        returnParam.reset(reader.ReadParameterSymbol(this));
    }
}

void DelegateTypeSymbol::EmplaceType(TypeSymbol* typeSymbol_, int index)
{
    Assert(index == 0, "invalid emplace type index");
    returnType = typeSymbol_;
}

void DelegateTypeSymbol::AddMember(Symbol* member)
{
    TypeSymbol::AddMember(member);
    if (member->GetSymbolType() == SymbolType::parameterSymbol)
    {
        parameters.push_back(static_cast<ParameterSymbol*>(member));
    }
}

std::string DelegateTypeSymbol::Syntax() const
{
    std::string syntax = GetSpecifierStr();
    if (!syntax.empty())
    {
        syntax.append(1, ' ');
    }
    syntax.append("delegate ");
    syntax.append(ToUtf8(ReturnType()->DocName()));
    syntax.append(1, ' ');
    syntax.append(ToUtf8(DocName()));
    syntax.append(1, '(');
    bool first = true;
    for (ParameterSymbol* param : parameters)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            syntax.append(", ");
        }
        syntax.append(ToUtf8(param->GetType()->DocName()));
        syntax.append(1, ' ');
        syntax.append(ToUtf8(param->DocName()));
    }
    syntax.append(");");
    return syntax;
}

void DelegateTypeSymbol::ComputeExportClosure()
{
    if (IsProject())
    {
        for (ParameterSymbol* parameter : parameters)
        {
            if (!parameter->ExportComputed())
            {
                parameter->SetExportComputed();
                parameter->ComputeExportClosure();
            }
        }
        if (returnParam)
        {
            if (!returnParam->ExportComputed())
            {
                returnParam->SetExportComputed();
                returnParam->ComputeExportClosure();
            }
        }
        if (returnType)
        {
            if (!returnType->ExportComputed())
            {
                returnType->SetExportComputed();
                returnType->ComputeExportClosure();
            }
        }
    }
}

void DelegateTypeSymbol::Accept(SymbolCollector* collector)
{
    if (IsProject() && Access() == SymbolAccess::public_)
    {
        collector->AddDelegate(this);
    }
}

void DelegateTypeSymbol::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine(ToUtf8(Name()));
    formatter.WriteLine("full name: " + ToUtf8(FullNameWithSpecifiers()));
    formatter.WriteLine("typeid: " + std::to_string(TypeId()));
}

llvm::Type* DelegateTypeSymbol::IrType(Emitter& emitter)
{
    if (!irType)
    {
        llvm::Type* retType = llvm::Type::getVoidTy(emitter.Context());
        if (!returnType->IsVoidType() && !ReturnsClassOrClassDelegateByValue())
        {
            retType = returnType->IrType(emitter);
        }
        std::vector<llvm::Type*> paramTypes;
        int np = parameters.size();
        for (int i = 0; i < np; ++i)
        {
            ParameterSymbol* parameter = parameters[i];
            paramTypes.push_back(parameter->GetType()->IrType(emitter));
        }
        if (returnParam)
        {
            paramTypes.push_back(returnParam->GetType()->IrType(emitter));
        }
        irType = llvm::PointerType::get(llvm::FunctionType::get(retType, paramTypes, false), 0);
    }
    return irType;
}

llvm::Constant* DelegateTypeSymbol::CreateDefaultIrValue(Emitter& emitter)
{
    return llvm::Constant::getNullValue(IrType(emitter));
}

void DelegateTypeSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("delegate cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("delegate cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("delegate cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("delegate cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("delegate cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception("delegate cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("delegate cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception("delegate cannot be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        throw Exception("delegate cannot be default", GetSpan());
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        throw Exception("delegate cannot be constexpr", GetSpan());
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception("delegate cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        SetNothrow();
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        if (IsNothrow())
        {
            throw Exception("delegate cannot be throw and nothrow at the same time", GetSpan());
        }
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("delegate cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception("delegate cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("delegate cannot be unit_test", GetSpan());
    }
}

bool DelegateTypeSymbol::ReturnsClassOrClassDelegateByValue() const
{
    return returnType->IsClassTypeSymbol() || returnType->GetSymbolType() == SymbolType::classDelegateTypeSymbol;
}

void DelegateTypeSymbol::SetReturnParam(ParameterSymbol* returnParam_)
{
    returnParam.reset(returnParam_);
}

void DelegateTypeSymbol::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    llvm::Value* callee = nullptr;
    int na = genObjects.size();
    for (int i = 0; i < na; ++i)
    {
        GenObject* genObject = genObjects[i];
        genObject->Load(emitter, flags & OperationFlags::functionCallFlags);
        if (i == 0)
        {
            callee = emitter.Stack().Pop();
        }
    }
    ArgVector args;
    int n = parameters.size();
    if (ReturnsClassOrClassDelegateByValue())
    {
        ++n;
    }
    args.resize(n);
    for (int i = 0; i < n; ++i)
    {
        llvm::Value* arg = emitter.Stack().Pop();
        args[n - i - 1] = arg;
    }
    llvm::BasicBlock* handlerBlock = emitter.HandlerBlock();
    llvm::BasicBlock* cleanupBlock = emitter.CleanupBlock();
    bool newCleanupNeeded = emitter.NewCleanupNeeded();
    Pad* currentPad = emitter.CurrentPad();
    std::vector<llvm::OperandBundleDef> bundles;
    if (currentPad != nullptr)
    {
        std::vector<llvm::Value*> inputs;
        inputs.push_back(currentPad->value);
        bundles.push_back(llvm::OperandBundleDef("funclet", inputs));
    }
    if (returnType->GetSymbolType() != SymbolType::voidTypeSymbol && !ReturnsClassOrClassDelegateByValue())
    {
        if (IsNothrow() || (!handlerBlock && !cleanupBlock && !newCleanupNeeded))
        {
            if (currentPad == nullptr)
            {
                emitter.Stack().Push(emitter.Builder().CreateCall(callee, args));
            }
            else
            {
                emitter.Stack().Push(llvm::CallInst::Create(callee, args, bundles, "", emitter.CurrentBasicBlock()));
            }
        }
        else
        {
            llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(emitter.Context(), "next", emitter.Function());
            if (newCleanupNeeded)
            {
                emitter.CreateCleanup();
                cleanupBlock = emitter.CleanupBlock();
            }
            llvm::BasicBlock* unwindBlock = cleanupBlock;
            if (unwindBlock == nullptr)
            {
                unwindBlock = handlerBlock;
                Assert(unwindBlock, "no unwind block");
            }
            if (currentPad == nullptr)
            {
                emitter.Stack().Push(emitter.Builder().CreateInvoke(callee, nextBlock, unwindBlock, args));
            }
            else
            {
                emitter.Stack().Push(llvm::InvokeInst::Create(callee, nextBlock, unwindBlock, args, bundles, "", emitter.CurrentBasicBlock()));
            }
            emitter.SetCurrentBasicBlock(nextBlock);
        }
    }
    else
    {
        if (IsNothrow() || (!handlerBlock && !cleanupBlock && !newCleanupNeeded))
        {
            if (currentPad == nullptr)
            {
                emitter.Builder().CreateCall(callee, args);
            }
            else
            {
                llvm::CallInst::Create(callee, args, bundles, "", emitter.CurrentBasicBlock());
            }
        }
        else
        {
            llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(emitter.Context(), "next", emitter.Function());
            if (newCleanupNeeded)
            {
                emitter.CreateCleanup();
                cleanupBlock = emitter.CleanupBlock();
            }
            llvm::BasicBlock* unwindBlock = cleanupBlock;
            if (unwindBlock == nullptr)
            {
                unwindBlock = handlerBlock;
                Assert(unwindBlock, "no unwind block");
            }
            if (currentPad == nullptr)
            {
                emitter.Builder().CreateInvoke(callee, nextBlock, unwindBlock, args);
            }
            else
            {
                llvm::InvokeInst::Create(callee, nextBlock, unwindBlock, args, bundles, "", emitter.CurrentBasicBlock());
            }
            emitter.SetCurrentBasicBlock(nextBlock);
        }
    }
}

DelegateTypeDefaultConstructor::DelegateTypeDefaultConstructor(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::delegateTypeDefaultConstructor, span_, name_)
{
}

DelegateTypeDefaultConstructor::DelegateTypeDefaultConstructor(DelegateTypeSymbol* delegateType_) : 
    FunctionSymbol(SymbolType::delegateTypeDefaultConstructor, Span(), U"@constructor"), delegateType(delegateType_)
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(delegateType->AddPointer(Span()));
    AddMember(thisParam);
    ComputeName();
}

void DelegateTypeDefaultConstructor::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    writer.GetBinaryWriter().Write(delegateType->TypeId());
}

void DelegateTypeDefaultConstructor::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    uint32_t typeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, typeId, 1);
}

void DelegateTypeDefaultConstructor::EmplaceType(TypeSymbol* typeSymbol, int index)
{
    if (index == 1)
    {
        Assert(typeSymbol->GetSymbolType() == SymbolType::delegateTypeSymbol, "delegate type symbol expected");
        delegateType = static_cast<DelegateTypeSymbol*>(typeSymbol);
    }
    else
    {
        FunctionSymbol::EmplaceType(typeSymbol, index);
    }
}

void DelegateTypeDefaultConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 1, "default constructor needs one object");
    emitter.Stack().Push(delegateType->CreateDefaultIrValue(emitter));
    genObjects[0]->Store(emitter, OperationFlags::none);
}

DelegateTypeCopyConstructor::DelegateTypeCopyConstructor(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::delegateTypeCopyConstructor, span_, name_)
{
}

DelegateTypeCopyConstructor::DelegateTypeCopyConstructor(DelegateTypeSymbol* delegateType) : FunctionSymbol(SymbolType::delegateTypeCopyConstructor, Span(), U"@constructor")
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(delegateType->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(delegateType);
    AddMember(thatParam);
    ComputeName();
}

void DelegateTypeCopyConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 2, "copy constructor needs two objects");
    genObjects[1]->Load(emitter, OperationFlags::none);
    genObjects[0]->Store(emitter, OperationFlags::none);
}

DelegateTypeMoveConstructor::DelegateTypeMoveConstructor(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::delegateTypeMoveConstructor, span_, name_)
{
}

DelegateTypeMoveConstructor::DelegateTypeMoveConstructor(DelegateTypeSymbol* delegateType) : FunctionSymbol(SymbolType::delegateTypeMoveConstructor, Span(), U"@constructor")
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(delegateType->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(delegateType->AddRvalueReference(Span()));
    AddMember(thatParam);
    ComputeName();
}

void DelegateTypeMoveConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 2, "move constructor needs two objects");
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* rvalueRefValue = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateLoad(rvalueRefValue));
    genObjects[0]->Store(emitter, OperationFlags::none);
}

DelegateTypeCopyAssignment::DelegateTypeCopyAssignment(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::delegateTypeCopyAssignment, span_, name_)
{
}

DelegateTypeCopyAssignment::DelegateTypeCopyAssignment(DelegateTypeSymbol* delegateType, TypeSymbol* voidType) : FunctionSymbol(SymbolType::delegateTypeCopyAssignment, Span(), U"operator=")
{
    SetGroupName(U"operator=");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(delegateType->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(delegateType);
    AddMember(thatParam);
    SetReturnType(voidType);
    ComputeName();
}

void DelegateTypeCopyAssignment::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 2, "copy assignment needs two objects");
    genObjects[1]->Load(emitter, OperationFlags::none);
    genObjects[0]->Store(emitter, OperationFlags::none);
}

DelegateTypeMoveAssignment::DelegateTypeMoveAssignment(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::delegateTypeMoveAssignment, span_, name_)
{
}

DelegateTypeMoveAssignment::DelegateTypeMoveAssignment(DelegateTypeSymbol* delegateType, TypeSymbol* voidType) : FunctionSymbol(SymbolType::delegateTypeMoveAssignment, Span(), U"operator=")
{
    SetGroupName(U"operator=");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(delegateType->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(delegateType->AddRvalueReference(Span()));
    AddMember(thatParam);
    SetReturnType(voidType);
    ComputeName();
}

void DelegateTypeMoveAssignment::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 2, "move assignment needs two objects");
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* rvalueRefValue = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateLoad(rvalueRefValue));
    genObjects[0]->Store(emitter, OperationFlags::none);
}

DelegateTypeReturn::DelegateTypeReturn(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::delegateTypeReturn, span_, name_)
{
}

DelegateTypeReturn::DelegateTypeReturn(DelegateTypeSymbol* delegateType) : FunctionSymbol(SymbolType::delegateTypeReturn, Span(), U"@return")
{
    SetGroupName(U"@return");
    ParameterSymbol* valueParam = new ParameterSymbol(Span(), U"value");
    valueParam->SetType(delegateType);
    AddMember(valueParam);
    SetReturnType(delegateType);
    ComputeName();
}

void DelegateTypeReturn::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 1, "return needs one object");
    genObjects[0]->Load(emitter, OperationFlags::none);
}

DelegateTypeEquality::DelegateTypeEquality(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::delegateTypeEquality, span_, name_)
{
}

DelegateTypeEquality::DelegateTypeEquality(DelegateTypeSymbol* delegateType, TypeSymbol* boolType) : FunctionSymbol(SymbolType::delegateTypeEquality, Span(), U"operator==")
{
    SetGroupName(U"operator==");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* leftParam = new ParameterSymbol(Span(), U"left");
    leftParam->SetType(delegateType);
    AddMember(leftParam);
    ParameterSymbol* rightParam = new ParameterSymbol(Span(), U"right");
    rightParam->SetType(delegateType);
    AddMember(rightParam);
    SetReturnType(boolType);
    ComputeName();
}

void DelegateTypeEquality::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 2, "operator== needs two objects");
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* left = emitter.Stack().Pop();
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* right = emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Builder().CreateICmpEQ(left, right));
}

FunctionToDelegateConversion::FunctionToDelegateConversion(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::functionToDelegateSymbol, span_, name_)
{
}

FunctionToDelegateConversion::FunctionToDelegateConversion(TypeSymbol* sourceType_, TypeSymbol* targetType_, FunctionSymbol* function_) :
    FunctionSymbol(SymbolType::functionToDelegateSymbol, Span(), U"@conversion"), sourceType(sourceType_), targetType(targetType_), function(function_)
{
    SetConversion();
}

void FunctionToDelegateConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    emitter.Stack().Pop();
    emitter.Stack().Push(emitter.Module()->getOrInsertFunction(ToUtf8(function->MangledName()), function->IrType(emitter)));
}

ClassDelegateTypeSymbol::ClassDelegateTypeSymbol(const Span& span_, const std::u32string& name_) : 
    TypeSymbol(SymbolType::classDelegateTypeSymbol, span_, name_), returnType(nullptr), parameters(), delegateType(nullptr), objectDelegatePairType(nullptr), irType(nullptr), copyConstructor(nullptr)
{
}

void ClassDelegateTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    uint32_t returnTypeId = returnType->TypeId();
    writer.GetBinaryWriter().Write(returnTypeId);
}

void ClassDelegateTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    uint32_t returnTypeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, returnTypeId, -1);
}

void ClassDelegateTypeSymbol::EmplaceType(TypeSymbol* typeSymbol_, int index)
{
    if (index == -1)
    {
        returnType = typeSymbol_;
    }
    else
    {
        TypeSymbol::EmplaceType(typeSymbol_, index);
    }
}

void ClassDelegateTypeSymbol::ComputeExportClosure()
{
    if (IsProject())
    {
        for (ParameterSymbol* parameter : parameters)
        {
            if (!parameter->ExportComputed())
            {
                parameter->SetExportComputed();
                parameter->ComputeExportClosure();
            }
        }
        if (returnParam)
        {
            if (!returnParam->ExportComputed())
            {
                returnParam->SetExportComputed();
                returnParam->ComputeExportClosure();
            }
        }
        if (returnType)
        {
            if (!returnType->ExportComputed())
            {
                returnType->SetExportComputed();
                returnType->ComputeExportClosure();
            }
        }
    }
}

void ClassDelegateTypeSymbol::AddMember(Symbol* member)
{
    TypeSymbol::AddMember(member);
    if (member->GetSymbolType() == SymbolType::parameterSymbol)
    {
        parameters.push_back(static_cast<ParameterSymbol*>(member));
    }
    else if (member->GetSymbolType() == SymbolType::delegateTypeSymbol)
    {
        delegateType = static_cast<DelegateTypeSymbol*>(member);
    }
    else if (member->GetSymbolType() == SymbolType::classTypeSymbol)
    {
        objectDelegatePairType = static_cast<ClassTypeSymbol*>(member);
    }
    else if (member->IsFunctionSymbol())
    {
        FunctionSymbol* functionSymbol = static_cast<FunctionSymbol*>(member);
        if (functionSymbol->IsClassDelegateCopyConstructor())
        {
            copyConstructor = functionSymbol;
        }
    }
}

std::string ClassDelegateTypeSymbol::Syntax() const
{
    std::string syntax = GetSpecifierStr();
    if (!syntax.empty())
    {
        syntax.append(1, ' ');
    }
    syntax.append("class delegate ");
    syntax.append(ToUtf8(ReturnType()->DocName()));
    syntax.append(1, ' ');
    syntax.append(ToUtf8(DocName()));
    syntax.append(1, '(');
    bool first = true;
    for (ParameterSymbol* param : parameters)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            syntax.append(", ");
        }
        syntax.append(ToUtf8(param->GetType()->DocName()));
        syntax.append(1, ' ');
        syntax.append(ToUtf8(param->DocName()));
    }
    syntax.append(");");
    return syntax;
}

void ClassDelegateTypeSymbol::Accept(SymbolCollector* collector)
{
    if (IsProject() && Access() == SymbolAccess::public_)
    {
        collector->AddClassDelegate(this);
    }
}

void ClassDelegateTypeSymbol::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine(ToUtf8(Name()));
    formatter.WriteLine("full name: " + ToUtf8(FullNameWithSpecifiers()));
    formatter.WriteLine("typeid: " + std::to_string(TypeId()));
}

llvm::Type* ClassDelegateTypeSymbol::IrType(Emitter& emitter)
{
    if (!irType)
    {
        std::vector<llvm::Type*> elementTypes;
        elementTypes.push_back(emitter.Builder().getInt8PtrTy());
        elementTypes.push_back(delegateType->IrType(emitter));
        irType = llvm::StructType::get(emitter.Context(), elementTypes);
    }
    return irType;
}

llvm::Constant* ClassDelegateTypeSymbol::CreateDefaultIrValue(Emitter& emitter)
{
    std::vector<llvm::Constant*> constants;
    constants.push_back(llvm::Constant::getNullValue(emitter.Builder().getInt8PtrTy()));
    constants.push_back(delegateType->CreateDefaultIrValue(emitter));
    return llvm::ConstantStruct::get(llvm::cast<llvm::StructType>(IrType(emitter)), constants);
}

bool ClassDelegateTypeSymbol::ReturnsClassOrClassDelegateByValue() const
{
    return returnType->IsClassTypeSymbol() || returnType->GetSymbolType() == SymbolType::classDelegateTypeSymbol;
}

void ClassDelegateTypeSymbol::SetReturnParam(ParameterSymbol* returnParam_)
{
    returnParam.reset(returnParam_);
}

void ClassDelegateTypeSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be default", GetSpan());
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be constexpr", GetSpan());
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        SetNothrow();
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        if (IsNothrow())
        {
            throw Exception("class delegate cannot be throw and nothrow at the same time", GetSpan());
        }
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("class delegate cannot be unit_test", GetSpan());
    }
}

void ClassDelegateTypeSymbol::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(!genObjects.empty(), "gen objects is empty");
    genObjects[0]->Load(emitter, flags);
    llvm::Value* classDelegatePtr = emitter.Stack().Pop();
    ArgVector delegateIndeces;
    delegateIndeces.push_back(emitter.Builder().getInt32(0));
    delegateIndeces.push_back(emitter.Builder().getInt32(1));
    llvm::Value* delegatePtr = emitter.Builder().CreateGEP(classDelegatePtr, delegateIndeces);
    llvm::Value* callee = emitter.Builder().CreateLoad(delegatePtr);
    LlvmValue calleeValue(callee);
    ArgVector objectIndeces;
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    llvm::Value* objectPtr = emitter.Builder().CreateGEP(classDelegatePtr, objectIndeces);
    llvm::Value* object = emitter.Builder().CreateLoad(objectPtr);
    LlvmValue objectValue(object);
    std::vector<GenObject*> classDelegateCallObjects;
    classDelegateCallObjects.push_back(&calleeValue);
    classDelegateCallObjects.push_back(&objectValue);
    int na = genObjects.size();
    for (int i = 1; i < na; ++i)
    {
        GenObject* genObject = genObjects[i];
        classDelegateCallObjects.push_back(genObject);
    }
    delegateType->GenerateCall(emitter, classDelegateCallObjects, flags);
}

ClassDelegateTypeDefaultConstructor::ClassDelegateTypeDefaultConstructor(const Span& span_, const std::u32string& name_) : 
    FunctionSymbol(SymbolType::classDelegateTypeDefaultConstructor, span_, name_)
{
}

ClassDelegateTypeDefaultConstructor::ClassDelegateTypeDefaultConstructor(ClassDelegateTypeSymbol* classDelegateType_) : 
    FunctionSymbol(SymbolType::classDelegateTypeDefaultConstructor, Span(), U"@constructor"), classDelegateType(classDelegateType_)
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(classDelegateType->AddPointer(Span()));
    AddMember(thisParam);
    ComputeName();
}

void ClassDelegateTypeDefaultConstructor::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    writer.GetBinaryWriter().Write(classDelegateType->TypeId());
}

void ClassDelegateTypeDefaultConstructor::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    uint32_t typeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, typeId, 1);
}

void ClassDelegateTypeDefaultConstructor::EmplaceType(TypeSymbol* typeSymbol, int index)
{
    if (index == 1)
    {
        Assert(typeSymbol->GetSymbolType() == SymbolType::classDelegateTypeSymbol, "class delegate type symbol expected");
        classDelegateType = static_cast<ClassDelegateTypeSymbol*>(typeSymbol);
    }
    else
    {
        FunctionSymbol::EmplaceType(typeSymbol, index);
    }
}

void ClassDelegateTypeDefaultConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(genObjects.size() == 1, "default constructor needs one object");
    llvm::Value* objectValue = llvm::Constant::getNullValue(emitter.Builder().getInt8PtrTy());
    ArgVector objectIndeces;
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* ptr = emitter.Stack().Pop();
    llvm::Value* objectPtr = emitter.Builder().CreateGEP(ptr, objectIndeces);
    emitter.Builder().CreateStore(objectValue, objectPtr);
    llvm::Value* delegateValue = classDelegateType->DelegateType()->CreateDefaultIrValue(emitter);
    ArgVector delegateIndeces;
    delegateIndeces.push_back(emitter.Builder().getInt32(0));
    delegateIndeces.push_back(emitter.Builder().getInt32(1));
    llvm::Value* delegatePtr = emitter.Builder().CreateGEP(ptr, delegateIndeces);
    emitter.Builder().CreateStore(delegateValue, delegatePtr);
}

ClassDelegateTypeCopyConstructor::ClassDelegateTypeCopyConstructor(const Span& span_, const std::u32string& name_) :
    FunctionSymbol(SymbolType::classDelegateTypeCopyConstructor, span_, name_)
{
}

ClassDelegateTypeCopyConstructor::ClassDelegateTypeCopyConstructor(ClassDelegateTypeSymbol* classDelegateType) :
    FunctionSymbol(SymbolType::classDelegateTypeCopyConstructor, Span(), U"@constructor")
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(classDelegateType->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(classDelegateType->AddConst(Span())->AddLvalueReference(Span()));
    AddMember(thatParam);
    ComputeName();
}

void ClassDelegateTypeCopyConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    ArgVector objectIndeces;
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* thatPtr = emitter.Stack().Pop();
    llvm::Value* thatObjectPtr = emitter.Builder().CreateGEP(thatPtr, objectIndeces);
    llvm::Value* objectValue = emitter.Builder().CreateLoad(thatObjectPtr);
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* thisPtr = emitter.Stack().Pop();
    llvm::Value* thisObjectPtr = emitter.Builder().CreateGEP(thisPtr, objectIndeces);
    emitter.Builder().CreateStore(objectValue, thisObjectPtr);
    ArgVector delegateIndeces;
    delegateIndeces.push_back(emitter.Builder().getInt32(0));
    delegateIndeces.push_back(emitter.Builder().getInt32(1));
    llvm::Value* thatDelegatePtr = emitter.Builder().CreateGEP(thatPtr, delegateIndeces);
    llvm::Value* delegateValue = emitter.Builder().CreateLoad(thatDelegatePtr);
    llvm::Value* thisDelegatePtr = emitter.Builder().CreateGEP(thisPtr, delegateIndeces);
    emitter.Builder().CreateStore(delegateValue, thisDelegatePtr);
}

ClassDelegateTypeMoveConstructor::ClassDelegateTypeMoveConstructor(const Span& span_, const std::u32string& name_) :
    FunctionSymbol(SymbolType::classDelegateTypeMoveConstructor, span_, name_)
{
}

ClassDelegateTypeMoveConstructor::ClassDelegateTypeMoveConstructor(ClassDelegateTypeSymbol* classDelegateType) :
    FunctionSymbol(SymbolType::classDelegateTypeCopyConstructor, Span(), U"@constructor")
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(classDelegateType->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(classDelegateType->AddRvalueReference(Span()));
    AddMember(thatParam);
    ComputeName();
}

void ClassDelegateTypeMoveConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    ArgVector objectIndeces;
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* thatPtr = emitter.Stack().Pop();
    llvm::Value* thatObjectPtr = emitter.Builder().CreateGEP(thatPtr, objectIndeces);
    llvm::Value* objectValue = emitter.Builder().CreateLoad(thatObjectPtr);
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* thisPtr = emitter.Stack().Pop();
    llvm::Value* thisObjectPtr = emitter.Builder().CreateGEP(thisPtr, objectIndeces);
    emitter.Builder().CreateStore(objectValue, thisObjectPtr);
    ArgVector delegateIndeces;
    delegateIndeces.push_back(emitter.Builder().getInt32(0));
    delegateIndeces.push_back(emitter.Builder().getInt32(1));
    llvm::Value* thatDelegatePtr = emitter.Builder().CreateGEP(thatPtr, delegateIndeces);
    llvm::Value* delegateValue = emitter.Builder().CreateLoad(thatDelegatePtr);
    llvm::Value* thisDelegatePtr = emitter.Builder().CreateGEP(thisPtr, delegateIndeces);
    emitter.Builder().CreateStore(delegateValue, thisDelegatePtr);
}

ClassDelegateTypeCopyAssignment::ClassDelegateTypeCopyAssignment(const Span& span_, const std::u32string& name_) :
    FunctionSymbol(SymbolType::classDelegateTypeCopyAssignment, span_, name_)
{
}

ClassDelegateTypeCopyAssignment::ClassDelegateTypeCopyAssignment(ClassDelegateTypeSymbol* classDelegateType, TypeSymbol* voidType) :
    FunctionSymbol(SymbolType::classDelegateTypeCopyAssignment, Span(), U"operator=")
{
    SetGroupName(U"operator=");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(classDelegateType->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(classDelegateType->AddConst(Span())->AddLvalueReference(Span()));
    AddMember(thatParam);
    SetReturnType(voidType);
    ComputeName();
}

void ClassDelegateTypeCopyAssignment::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    ArgVector objectIndeces;
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* thatPtr = emitter.Stack().Pop();
    llvm::Value* thatObjectPtr = emitter.Builder().CreateGEP(thatPtr, objectIndeces);
    llvm::Value* objectValue = emitter.Builder().CreateLoad(thatObjectPtr);
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* thisPtr = emitter.Stack().Pop();
    llvm::Value* thisObjectPtr = emitter.Builder().CreateGEP(thisPtr, objectIndeces);
    emitter.Builder().CreateStore(objectValue, thisObjectPtr);
    ArgVector delegateIndeces;
    delegateIndeces.push_back(emitter.Builder().getInt32(0));
    delegateIndeces.push_back(emitter.Builder().getInt32(1));
    llvm::Value* thatDelegatePtr = emitter.Builder().CreateGEP(thatPtr, delegateIndeces);
    llvm::Value* delegateValue = emitter.Builder().CreateLoad(thatDelegatePtr);
    llvm::Value* thisDelegatePtr = emitter.Builder().CreateGEP(thisPtr, delegateIndeces);
    emitter.Builder().CreateStore(delegateValue, thisDelegatePtr);
}

ClassDelegateTypeMoveAssignment::ClassDelegateTypeMoveAssignment(const Span& span_, const std::u32string& name_) :
    FunctionSymbol(SymbolType::classDelegateTypeMoveAssignment, span_, name_)
{
}

ClassDelegateTypeMoveAssignment::ClassDelegateTypeMoveAssignment(ClassDelegateTypeSymbol* classDelegateType, TypeSymbol* voidType) :
    FunctionSymbol(SymbolType::classDelegateTypeMoveAssignment, Span(), U"operator=")
{
    SetGroupName(U"operator=");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(classDelegateType->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(classDelegateType->AddRvalueReference(Span()));
    AddMember(thatParam);
    SetReturnType(voidType);
    ComputeName();
}

void ClassDelegateTypeMoveAssignment::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    ArgVector objectIndeces;
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* thatPtr = emitter.Stack().Pop();
    llvm::Value* thatObjectPtr = emitter.Builder().CreateGEP(thatPtr, objectIndeces);
    llvm::Value* objectValue = emitter.Builder().CreateLoad(thatObjectPtr);
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* thisPtr = emitter.Stack().Pop();
    llvm::Value* thisObjectPtr = emitter.Builder().CreateGEP(thisPtr, objectIndeces);
    emitter.Builder().CreateStore(objectValue, thisObjectPtr);
    ArgVector delegateIndeces;
    delegateIndeces.push_back(emitter.Builder().getInt32(0));
    delegateIndeces.push_back(emitter.Builder().getInt32(1));
    llvm::Value* thatDelegatePtr = emitter.Builder().CreateGEP(thatPtr, delegateIndeces);
    llvm::Value* delegateValue = emitter.Builder().CreateLoad(thatDelegatePtr);
    llvm::Value* thisDelegatePtr = emitter.Builder().CreateGEP(thisPtr, delegateIndeces);
    emitter.Builder().CreateStore(delegateValue, thisDelegatePtr);
}

ClassDelegateTypeEquality::ClassDelegateTypeEquality(const Span& span_, const std::u32string& name_) :
    FunctionSymbol(SymbolType::classDelegateTypeEquality, span_, name_)
{
}

ClassDelegateTypeEquality::ClassDelegateTypeEquality(ClassDelegateTypeSymbol* classDelegateType, TypeSymbol* boolType) :
    FunctionSymbol(SymbolType::classDelegateTypeEquality, Span(), U"operator==")
{
    SetGroupName(U"operator==");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* leftParam = new ParameterSymbol(Span(), U"left");
    leftParam->SetType(classDelegateType->AddConst(Span())->AddLvalueReference(Span()));
    AddMember(leftParam);
    ParameterSymbol* rightParam = new ParameterSymbol(Span(), U"right");
    rightParam->SetType(classDelegateType->AddConst(Span())->AddLvalueReference(Span()));
    AddMember(rightParam);
    SetReturnType(boolType);
    ComputeName();
}

void ClassDelegateTypeEquality::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    ArgVector objectIndeces;
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    genObjects[0]->Load(emitter, OperationFlags::none);
    llvm::Value* leftPtr = emitter.Stack().Pop();
    llvm::Value* leftObjectPtr = emitter.Builder().CreateGEP(leftPtr, objectIndeces);
    llvm::Value* leftObjectValue = emitter.Builder().CreateLoad(leftObjectPtr);
    genObjects[1]->Load(emitter, OperationFlags::none);
    llvm::Value* rightPtr = emitter.Stack().Pop();
    llvm::Value* rightObjectPtr = emitter.Builder().CreateGEP(rightPtr, objectIndeces);
    llvm::Value* rightObjectValue = emitter.Builder().CreateLoad(rightObjectPtr);
    llvm::Value* objectsEqual = emitter.Builder().CreateICmpEQ(leftObjectValue, rightObjectValue);
    ArgVector delegateIndeces;
    delegateIndeces.push_back(emitter.Builder().getInt32(0));
    delegateIndeces.push_back(emitter.Builder().getInt32(1));
    llvm::Value* leftDelegatePtr = emitter.Builder().CreateGEP(leftPtr, delegateIndeces);
    llvm::Value* leftDelegateValue = emitter.Builder().CreateLoad(leftDelegatePtr);
    llvm::Value* rightDelegatePtr = emitter.Builder().CreateGEP(rightPtr, delegateIndeces);
    llvm::Value* rightDelegateValue = emitter.Builder().CreateLoad(rightDelegatePtr);
    llvm::Value* delegatesEqual = emitter.Builder().CreateICmpEQ(leftDelegateValue, rightDelegateValue);
    llvm::Value* equal = emitter.Builder().CreateAnd(objectsEqual, delegatesEqual);
    emitter.Stack().Push(equal);
}

MemberFunctionToClassDelegateConversion::MemberFunctionToClassDelegateConversion(const Span& span_, const std::u32string& name_) :
    FunctionSymbol(SymbolType::memberFunctionToClassDelegateSymbol, span_, name_)
{
}

MemberFunctionToClassDelegateConversion::MemberFunctionToClassDelegateConversion(const Span& span_, TypeSymbol* sourceType_, ClassDelegateTypeSymbol* targetType_, FunctionSymbol* function_,
    LocalVariableSymbol* objectDelegatePairVariable_) :
    FunctionSymbol(SymbolType::memberFunctionToClassDelegateSymbol, span_, U"@conversion"), sourceType(sourceType_), targetType(targetType_), function(function_), 
    objectDelegatePairVariable(objectDelegatePairVariable_)
{
    SetConversion();
}

void MemberFunctionToClassDelegateConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    llvm::Value* objectValue = emitter.Stack().Pop();
    if (!objectValue)
    {
        throw Exception("cannot construct class delegate because expression has no this pointer", GetSpan());
    }
    llvm::Value* objectValueAsVoidPtr = emitter.Builder().CreateBitCast(objectValue, emitter.Builder().getInt8PtrTy());
    llvm::Value* memFunPtrValue = emitter.Module()->getOrInsertFunction(ToUtf8(function->MangledName()), function->IrType(emitter));
    llvm::Value* ptr = objectDelegatePairVariable->IrObject();
    ArgVector objectIndeces;
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    objectIndeces.push_back(emitter.Builder().getInt32(0));
    llvm::Value* objectPtr = emitter.Builder().CreateGEP(ptr, objectIndeces);
    emitter.Builder().CreateStore(objectValueAsVoidPtr, objectPtr);
    ArgVector delegateIndeces;
    delegateIndeces.push_back(emitter.Builder().getInt32(0));
    delegateIndeces.push_back(emitter.Builder().getInt32(1));
    llvm::Value* delegatePtr = emitter.Builder().CreateGEP(ptr, delegateIndeces);
    llvm::Value* delegateValue = emitter.Builder().CreateBitCast(memFunPtrValue, targetType->DelegateType()->IrType(emitter));
    emitter.Builder().CreateStore(delegateValue, delegatePtr);
    emitter.Stack().Push(ptr);
}

} } // namespace cmajor::symbols
