// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/ClassTypeSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/TemplateSymbol.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/Sha1.hpp>
#include <llvm/IR/Module.h>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

class OperatorMangleMap
{
public:
    static OperatorMangleMap& Instance() { Assert(instance, "operator mangle map not initialized");  return *instance; }
    static void Init();
    static void Done();
    std::u32string Mangle(const std::u32string& groupName);
private:
    static std::unique_ptr<OperatorMangleMap> instance;
    std::unordered_map<std::u32string, std::u32string> mangleMap;
    OperatorMangleMap();
};

std::unique_ptr<OperatorMangleMap> OperatorMangleMap::instance;

void OperatorMangleMap::Init()
{
    instance.reset(new OperatorMangleMap());
}

void OperatorMangleMap::Done()
{
    instance.reset();
}

OperatorMangleMap::OperatorMangleMap()
{
    mangleMap[U"operator<<"] = U"op_shl";
    mangleMap[U"operator>>"] = U"op_shr";
    mangleMap[U"operator=="] = U"op_eq";
    mangleMap[U"operator="] = U"op_assign";
    mangleMap[U"operator<"] = U"op_less";
    mangleMap[U"operator->"] = U"op_arrow";
    mangleMap[U"operator++"] = U"op_plusplus";
    mangleMap[U"operator--"] = U"op_minusminus";
    mangleMap[U"operator+"] = U"op_plus";
    mangleMap[U"operator-"] = U"op_minus";
    mangleMap[U"operator*"] = U"op_star";
    mangleMap[U"operator/"] = U"op_div";
    mangleMap[U"operator%"] = U"op_rem";
    mangleMap[U"operator&"] = U"op_and";
    mangleMap[U"operator|"] = U"op_or";
    mangleMap[U"operator^"] = U"op_xor";
    mangleMap[U"operator!"] = U"op_not";
    mangleMap[U"operator~"] = U"op_cpl";
    mangleMap[U"operator[]"] = U"op_index";
    mangleMap[U"operator()"] = U"op_apply";
}

std::u32string OperatorMangleMap::Mangle(const std::u32string& groupName)
{
    auto it = mangleMap.find(groupName);
    if (it != mangleMap.cend())
    {
        return it->second;
    }
    else
    {
        return U"operator";
    }
}

FunctionGroupSymbol::FunctionGroupSymbol(const Span& span_, const std::u32string& name_) : Symbol(SymbolType::functionGroupSymbol, span_, name_)
{
}

void FunctionGroupSymbol::AddFunction(FunctionSymbol* function)
{
    Assert(function->GroupName() == Name(), "wrong function group");
    int arity = function->Arity();
    std::vector<FunctionSymbol*>& functionList = arityFunctionListMap[arity];
    functionList.push_back(function);
}

void FunctionGroupSymbol::CollectViableFunctions(int arity, std::unordered_set<FunctionSymbol*>& viableFunctions)
{
    auto it = arityFunctionListMap.find(arity);
    if (it != arityFunctionListMap.cend())
    {
        std::vector<FunctionSymbol*>& functionList = it->second;
        for (FunctionSymbol* function : functionList)
        {
            viableFunctions.insert(function);
        }
    }
}

std::string FunctionSymbolFlagStr(FunctionSymbolFlags flags)
{
    std::string s;
    if ((flags & FunctionSymbolFlags::inline_) != FunctionSymbolFlags::none)
    {
        s.append("inline");
    }
    if ((flags & FunctionSymbolFlags::external_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("extern");
    }
    if ((flags & FunctionSymbolFlags::constExpr) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("constexpr");
    }
    if ((flags & FunctionSymbolFlags::cdecl_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("cdecl");
    }
    if ((flags & FunctionSymbolFlags::suppress) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("suppress");
    }
    if ((flags & FunctionSymbolFlags::default_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("default");
    }
    if ((flags & FunctionSymbolFlags::explicit_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("explicit");
    }
    if ((flags & FunctionSymbolFlags::virtual_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("virtual");
    }
    if ((flags & FunctionSymbolFlags::override_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("override");
    }
    if ((flags & FunctionSymbolFlags::abstract_) != FunctionSymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("abstract");
    }
    return s;
}

FunctionSymbol::FunctionSymbol(const Span& span_, const std::u32string& name_) : 
    ContainerSymbol(SymbolType::functionSymbol, span_, name_), groupName(), parameters(), localVariables(), returnType(), flags(FunctionSymbolFlags::none), vmtIndex(-1), imtIndex(-1), 
    irType(nullptr), nextTemporaryIndex(0)
{
}

FunctionSymbol::FunctionSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) : 
    ContainerSymbol(symbolType_, span_, name_), groupName(), parameters(), localVariables(), returnType(), flags(FunctionSymbolFlags::none), vmtIndex(-1), imtIndex(-1), 
    irType(nullptr), nextTemporaryIndex(0), sizeOfAstNodes(0), astNodesPos(0)
{
}

void FunctionSymbol::Write(SymbolWriter& writer)
{
    ContainerSymbol::Write(writer);
    writer.GetBinaryWriter().Write(groupName);
    if (IsFunctionTemplate())
    {
        uint32_t sizePos = writer.GetBinaryWriter().Pos();
        uint32_t sizeOfAstNodes = 0;
        writer.GetBinaryWriter().Write(sizeOfAstNodes);
        uint32_t startPos = writer.GetBinaryWriter().Pos();
        usingNodes.Write(writer.GetAstWriter());
        Node* node = GetSymbolTable()->GetNode(this);
        writer.GetAstWriter().Write(node);
        uint32_t endPos = writer.GetBinaryWriter().Pos();
        sizeOfAstNodes = endPos - startPos;
        writer.GetBinaryWriter().Seek(sizePos);
        writer.GetBinaryWriter().Write(sizeOfAstNodes);
        writer.GetBinaryWriter().Seek(endPos);
    }
    uint32_t returnTypeId = 0;
    if (returnType)
    {
        returnTypeId = returnType->TypeId();
    }
    writer.GetBinaryWriter().WriteEncodedUInt(returnTypeId);
    writer.GetBinaryWriter().Write(static_cast<uint16_t>(flags));
    writer.GetBinaryWriter().Write(vmtIndex);
    writer.GetBinaryWriter().Write(imtIndex);
    bool hasReturnParam = returnParam != nullptr;
    writer.GetBinaryWriter().Write(hasReturnParam);
    if (hasReturnParam)
    {
        writer.Write(returnParam.get());
    }
}

void FunctionSymbol::Read(SymbolReader& reader)
{
    ContainerSymbol::Read(reader);
    groupName = reader.GetBinaryReader().ReadUtf32String();
    if (IsFunctionTemplate())
    {
        sizeOfAstNodes = reader.GetBinaryReader().ReadUInt();
        astNodesPos = reader.GetBinaryReader().Pos();
        reader.GetBinaryReader().Skip(sizeOfAstNodes);
        filePathReadFrom = reader.GetBinaryReader().FileName();
    }
    uint32_t returnTypeId = reader.GetBinaryReader().ReadEncodedUInt();
    if (returnTypeId != 0)
    {
        GetSymbolTable()->EmplaceTypeRequest(this, returnTypeId, 0);
    }
    flags = static_cast<FunctionSymbolFlags>(reader.GetBinaryReader().ReadUShort());
    vmtIndex = reader.GetBinaryReader().ReadInt();
    imtIndex = reader.GetBinaryReader().ReadInt();
    if (IsConversion())
    {
        reader.AddConversion(this);
    }
    bool hasReturnParam = reader.GetBinaryReader().ReadBool();
    if (hasReturnParam)
    {
        returnParam.reset(reader.ReadParameterSymbol(this));
    }
}

void FunctionSymbol::ReadAstNodes()
{
    AstReader reader(filePathReadFrom);
    reader.GetBinaryReader().Skip(astNodesPos);
    usingNodes.Read(reader);
    Node* node = reader.ReadNode();
    Assert(node->GetNodeType() == NodeType::functionNode, "function node expected");
    FunctionNode* funNode = static_cast<FunctionNode*>(node);
    functionNode.reset(funNode);
    GetSymbolTable()->MapNode(funNode, this);
}

void FunctionSymbol::EmplaceType(TypeSymbol* typeSymbol_, int index)
{
    Assert(index == 0, "invalid emplace type index");
    returnType = typeSymbol_;
}

void FunctionSymbol::AddMember(Symbol* member)
{
    ContainerSymbol::AddMember(member);
    if (member->GetSymbolType() == SymbolType::templateParameterSymbol)
    {
        templateParameters.push_back(static_cast<TemplateParameterSymbol*>(member));
    }
    else if (member->GetSymbolType() == SymbolType::parameterSymbol)
    {
        parameters.push_back(static_cast<ParameterSymbol*>(member));
    }
}

bool FunctionSymbol::IsExportSymbol() const
{
    if (IsTemplateSpecialization()) return false;
    return ContainerSymbol::IsExportSymbol();
}

void FunctionSymbol::ComputeName()
{
    std::u32string name;
    name.append(groupName);
    name.append(1, U'(');
    int n = parameters.size();
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            name.append(U", ");
        }
        ParameterSymbol* parameter = parameters[i];
        if (i == 0 && (groupName == U"@constructor" || groupName == U"operator=" || IsConstructorDestructorOrNonstaticMemberFunction()))
        {
            name.append(parameter->GetType()->RemovePointer(GetSpan())->FullName());
        }
        else
        {
            name.append(parameter->GetType()->FullName());
        }
        name.append(1, U' ');
        name.append(std::u32string(parameter->Name()));
    }
    name.append(1, U')');
    SetName(name);
    if (!IsBasicTypeOperation() && !IsFunctionTemplate())
    {
        ComputeMangledName();
    }
}

void FunctionSymbol::ComputeMangledName()
{
    if (IsCDecl())
    {
        SetMangledName(GroupName());
        return;
    }
    std::u32string mangledName = ToUtf32(TypeString());
    if (groupName.find(U"operator") != std::u32string::npos)
    {
        mangledName.append(1, U'_').append(OperatorMangleMap::Instance().Mangle(GroupName()));
    }
    else if (groupName.find(U'@') == std::u32string::npos)
    {
        mangledName.append(1, U'_').append(GroupName());
    }
    SymbolType symbolType = GetSymbolType();
    switch (symbolType)
    {
        case SymbolType::staticConstructorSymbol: case SymbolType::constructorSymbol: case SymbolType::destructorSymbol: case SymbolType::memberFunctionSymbol:
        {
            Symbol* parentClass = Parent();
            mangledName.append(1, U'_').append(parentClass->SimpleName());
        }
    }
    mangledName.append(1, U'_').append(ToUtf32(GetSha1MessageDigest(ToUtf8(FullNameWithSpecifiers()))));
    SetMangledName(mangledName);
}    

std::u32string FunctionSymbol::FullName() const
{
    std::u32string fullName;
    std::u32string parentFullName = Parent()->FullName();
    fullName.append(parentFullName);
    if (!parentFullName.empty())
    {
        fullName.append(1, U'.');
    }
    fullName.append(groupName);
    fullName.append(1, U'(');
    int n = parameters.size();
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            fullName.append(U", ");
        }
        ParameterSymbol* parameter = parameters[i];
        if (i == 0 && (groupName == U"@constructor" || groupName == U"operator=" || IsConstructorDestructorOrNonstaticMemberFunction()))
        {
            fullName.append(parameter->GetType()->RemovePointer(GetSpan())->FullName());
        }
        else
        {
            fullName.append(parameter->GetType()->FullName());
        }
    }
    fullName.append(1, U')');
    return fullName;
}

std::u32string FunctionSymbol::FullNameWithSpecifiers() const
{
    std::u32string fullNameWithSpecifiers = ToUtf32(SymbolFlagStr(GetSymbolFlags()));
    std::u32string f = ToUtf32(FunctionSymbolFlagStr(flags));
    if (!f.empty())
    {
        if (!fullNameWithSpecifiers.empty())
        {
            fullNameWithSpecifiers.append(1, U' ');
        }
        fullNameWithSpecifiers.append(f);
    }
    if (!fullNameWithSpecifiers.empty())
    {
        fullNameWithSpecifiers.append(1, U' ');
    }
    fullNameWithSpecifiers.append(FullName());
    return fullNameWithSpecifiers;
}

void FunctionSymbol::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    if ((flags & OperationFlags::virtualCall) != OperationFlags::none)
    {
        GenerateVirtualCall(emitter, genObjects, flags);
        return;
    }
    int na = genObjects.size();
    for (int i = 0; i < na; ++i)
    {
        GenObject* genObject = genObjects[i];
        genObject->Load(emitter, flags & OperationFlags::functionCallFlags);
    }
    llvm::FunctionType* functionType = IrType(emitter);
    llvm::Function* callee = llvm::cast<llvm::Function>(emitter.Module()->getOrInsertFunction(ToUtf8(MangledName()), functionType));
    ArgVector args;
    int n = parameters.size();
    if (ReturnsClassByValue())
    {
        ++n;
    }
    args.resize(n);
    for (int i = 0; i < n; ++i)
    {
        llvm::Value* arg = emitter.Stack().Pop();
        args[n - i - 1] = arg;
    }
    if (ReturnType() && ReturnType()->GetSymbolType() != SymbolType::voidTypeSymbol && !ReturnsClassByValue())
    {
        emitter.Stack().Push(emitter.Builder().CreateCall(callee, args));
    }
    else
    {
        emitter.Builder().CreateCall(callee, args);
    }
}

void FunctionSymbol::GenerateVirtualCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    int na = genObjects.size();
    Assert(na > 0, "nonempty argument list expected");
    Assert(VmtIndex() != -1, "member function has invalid vmt index");
    GenObject* classPtrArg = genObjects[0];
    TypeSymbol* type = static_cast<TypeSymbol*>(classPtrArg->GetType());
    Assert(type, "type expected");
    Assert(type->BaseType()->IsClassTypeSymbol(), "class type pointer expected");
    ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(type->BaseType());
    ClassTypeSymbol* vmtPtrHolderClass = classType->VmtPtrHolderClass();
    llvm::Value* callee = nullptr;
    for (int i = 0; i < na; ++i)
    {
        GenObject* genObject = genObjects[i];
        genObject->Load(emitter, OperationFlags::none);
        if (i == 0)
        {
            emitter.Stack().Dup();
            llvm::Value* thisPtr = emitter.Stack().Pop();
            if (classType != vmtPtrHolderClass)
            {
                thisPtr = emitter.Builder().CreateBitCast(thisPtr, vmtPtrHolderClass->AddPointer(GetSpan())->IrType(emitter));
            }
            ArgVector vmtPtrIndeces;
            vmtPtrIndeces.push_back(emitter.Builder().getInt32(0));
            vmtPtrIndeces.push_back(emitter.Builder().getInt32(vmtPtrHolderClass->VmtPtrIndex()));
            llvm::Value* vmtPtrPtr = emitter.Builder().CreateGEP(thisPtr, vmtPtrIndeces);
            llvm::Value* vmtPtr = emitter.Builder().CreateBitCast(emitter.Builder().CreateLoad(vmtPtrPtr), classType->VmtPtrType(emitter));
            ArgVector funPtrIndeces;
            funPtrIndeces.push_back(emitter.Builder().getInt32(0));
            funPtrIndeces.push_back(emitter.Builder().getInt32(VmtIndex() + functionVmtIndexOffset));
            llvm::Value* funPtrPtr = emitter.Builder().CreateGEP(vmtPtr, funPtrIndeces);
            llvm::Value* funAsVoidPtr = emitter.Builder().CreateLoad(funPtrPtr);
            callee = emitter.Builder().CreateBitCast(funAsVoidPtr, llvm::PointerType::get(IrType(emitter), 0));
        }
    }
    ArgVector args;
    int n = Parameters().size();
    args.resize(n);
    for (int i = 0; i < n; ++i)
    {
        llvm::Value* arg = emitter.Stack().Pop();
        args[n - i - 1] = arg;
    }
    if (ReturnType() && !ReturnType()->IsVoidType() && !ReturnsClassByValue())
    {
        emitter.Stack().Push(emitter.Builder().CreateCall(IrType(emitter), callee, args));
    }
    else
    {
        emitter.Builder().CreateCall(IrType(emitter), callee, args);
    }
}

bool FunctionSymbol::IsDefaultConstructor() const
{
    return parameters.size() == 1 && groupName == U"@constructor" && parameters[0]->GetType()->PointerCount() == 1 && parameters[0]->GetType()->RemovePointer(GetSpan())->IsClassTypeSymbol();
}

bool FunctionSymbol::IsCopyConstructor() const
{
    return parameters.size() == 2 && groupName == U"@constructor" &&
        parameters[0]->GetType()->PointerCount() == 1 &&
        parameters[0]->GetType()->RemovePointer(GetSpan())->IsClassTypeSymbol() &&
        TypesEqual(parameters[0]->GetType()->BaseType()->AddConst(GetSpan())->AddLvalueReference(GetSpan()), parameters[1]->GetType());
}

bool FunctionSymbol::IsMoveConstructor() const
{
    return parameters.size() == 2 && groupName == U"@constructor" &&
        parameters[0]->GetType()->PointerCount() == 1 &&
        parameters[0]->GetType()->RemovePointer(GetSpan())->IsClassTypeSymbol() &&
        TypesEqual(parameters[0]->GetType()->BaseType()->AddRvalueReference(GetSpan()), parameters[1]->GetType());
}

bool FunctionSymbol::IsCopyAssignment() const
{
    return parameters.size() == 2 && groupName == U"operator=" &&
        parameters[0]->GetType()->PointerCount() == 1 &&
        parameters[0]->GetType()->RemovePointer(GetSpan())->IsClassTypeSymbol() &&
        TypesEqual(parameters[0]->GetType()->BaseType()->AddConst(GetSpan())->AddLvalueReference(GetSpan()), parameters[1]->GetType());
}

bool FunctionSymbol::IsMoveAssignment() const
{
    return parameters.size() == 2 && groupName == U"operator=" &&
        parameters[0]->GetType()->PointerCount() == 1 &&
        parameters[0]->GetType()->RemovePointer(GetSpan())->IsClassTypeSymbol() &&
        TypesEqual(parameters[0]->GetType()->BaseType()->AddRvalueReference(GetSpan()), parameters[1]->GetType());
}

void FunctionSymbol::AddLocalVariable(LocalVariableSymbol* localVariable)
{
    localVariables.push_back(localVariable);
}

bool FunctionSymbol::ReturnsClassByValue() const
{
    return returnType && returnType->IsClassTypeSymbol();
}

void FunctionSymbol::SetReturnParam(ParameterSymbol* returnParam_)
{
    returnParam.reset(returnParam_);
}

void FunctionSymbol::SetGroupName(const std::u32string& groupName_)
{
    groupName = groupName_;
}

void FunctionSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("only member functions can be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("only member functions can be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("only member functions can be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("only member functions can be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        SetInline();
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception("only constructors can be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        SetExternal();
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception("only special member functions can be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        throw Exception("only special member functions can be default", GetSpan());
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        SetConstExpr();
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        SetCDecl();
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        SetNothrow();
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        if (IsNothrow())
        {
            throw Exception("function symbol cannot be throw and nothrow at the same time", GetSpan());
        }
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("only member functions can be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception("only member functions can be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("function symbol cannot be unit_test", GetSpan());
    }
}

void FunctionSymbol::CloneUsingNodes(const std::vector<Node*>& usingNodes_)
{
    CloneContext cloneContext;
    for (Node* usingNode : usingNodes_)
    {
        usingNodes.Add(usingNode->Clone(cloneContext));
    }
}

LocalVariableSymbol* FunctionSymbol::CreateTemporary(TypeSymbol* type, const Span& span)
{
    LocalVariableSymbol* temporary = new LocalVariableSymbol(span, U"@t" + ToUtf32(std::to_string(nextTemporaryIndex++)));
    temporary->SetType(type);
    AddMember(temporary);
    AddLocalVariable(temporary);
    return temporary;
}

llvm::FunctionType* FunctionSymbol::IrType(Emitter& emitter)
{
    if (!irType)
    {
        llvm::Type* retType = llvm::Type::getVoidTy(emitter.Context());
        if (returnType && returnType->GetSymbolType() != SymbolType::voidTypeSymbol && !ReturnsClassByValue())
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
        irType = llvm::FunctionType::get(retType, paramTypes, false);
    }
    return irType;
}

StaticConstructorSymbol::StaticConstructorSymbol(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::staticConstructorSymbol, span_, name_)
{
    SetGroupName(U"@static_constructor");
}

void StaticConstructorSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    SetStatic();
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be default", GetSpan());
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be constexpr", GetSpan());
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        SetNothrow();
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        if (IsNothrow())
        {
            throw Exception("static constructor cannot be throw and nothrow at the same time", GetSpan());
        }
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("static constructor cannot be unit_test", GetSpan());
    }
}

ConstructorSymbol::ConstructorSymbol(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::constructorSymbol, span_, name_)
{
    SetGroupName(U"@constructor");
}

std::string ConstructorSymbol::TypeString() const 
{
    if (IsDefaultConstructor())
    {
        return "default_constructor";
    }
    else if (IsCopyConstructor())
    {
        return "copy_constructor";
    }
    else if (IsMoveConstructor())
    {
        return "move_constructor";
    }
    else
    {
        return "constructor";
    }
}

void ConstructorSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("ordinary constructor cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("constructor cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("constructor cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("constructor cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        SetInline();
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        SetExplicit();
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("constructor cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        SetSuppressed();
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        if (IsSuppressed())
        {
            throw Exception("constructor cannot be default and suppressed at the same time", GetSpan());
        }
        SetDefault();
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        SetConstExpr();
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception("constructor cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        SetNothrow();
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        if (IsNothrow())
        {
            throw Exception("constructor cannot be throw and nothrow at the same time", GetSpan());
        }
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("constructor cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception("constructor cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("constructor cannot be unit_test", GetSpan());
    }
}

DestructorSymbol::DestructorSymbol(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::destructorSymbol, span_, name_)
{
    SetGroupName(U"@destructor");
}

void DestructorSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    if (accessSpecifiers != Specifiers::public_)
    {
        throw Exception("destructor must be public", GetSpan());
    }
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("destructor cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        SetVirtual();
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        if (IsVirtual())
        {
            throw Exception("destructor cannot be virtual and override at the same time", GetSpan());
        }
        SetOverride();
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("destructor cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        SetInline();
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception("destructor cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("destructor cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception("destructor cannot be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        SetDefault();
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        throw Exception("destructor cannot be constexpr", GetSpan());
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception("destructor cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        throw Exception("destructor is implicitly nothrow", GetSpan());
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        throw Exception("destructor cannot be throw", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("destructor cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception("destructor cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("destructor cannot be unit_test", GetSpan());
    }
}

MemberFunctionSymbol::MemberFunctionSymbol(const Span& span_, const std::u32string& name_) : FunctionSymbol(SymbolType::memberFunctionSymbol, span_, name_)
{
}

std::string MemberFunctionSymbol::TypeString() const 
{ 
    if (IsCopyAssignment())
    {
        return "copy_assignment";
    }
    else if (IsMoveAssignment())
    {
        return "move_assignment";
    }
    else
    {
        return "member_function";
    }
}

void MemberFunctionSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
    SetStatic();
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        SetVirtual();
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        if (IsVirtual())
        {
            throw Exception("member function cannot be virtual and override at the same time", GetSpan());
        }
        SetOverride();
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        if (IsVirtual() || IsOverride())
        {
            throw Exception("member function cannot be abstract and virtual or override at the same time", GetSpan());
        }
        SetAbstract();
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        SetInline();
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception("member function cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("member function cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        if (GroupName() == U"operator=")
        {
            SetSuppressed();
        }
        else
        {
            throw Exception("only special member functions can be suppressed", GetSpan());
        }
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        if (IsSuppressed())
        {
            throw Exception("member function cannot be default and suppressed at the same time", GetSpan());
        }
        if (GroupName() == U"operator=")
        {
            SetDefault();
        }
        else
        {
            throw Exception("only special member functions can be default", GetSpan());
        }
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        SetConstExpr();
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception("member function cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        SetNothrow();
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        if (IsNothrow())
        {
            throw Exception("member function cannot be throw and nothrow at the same time", GetSpan());
        }
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        SetNew();
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        SetConst();
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("member function cannot be unit_test", GetSpan());
    }
}

FunctionGroupTypeSymbol::FunctionGroupTypeSymbol(FunctionGroupSymbol* functionGroup_) : TypeSymbol(SymbolType::functionGroupTypeSymbol, Span(), functionGroup_->Name()), functionGroup(functionGroup_)
{
}

MemberExpressionTypeSymbol::MemberExpressionTypeSymbol(const Span& span_, const std::u32string& name_, void* boundMemberExpression_) :
    TypeSymbol(SymbolType::memberExpressionTypeSymbol, span_, name_), boundMemberExpression(boundMemberExpression_)
{
}

void InitFunctionSymbol()
{
    OperatorMangleMap::Init();
}

void DoneFunctionSymbol()
{
    OperatorMangleMap::Done();
}

} } // namespace cmajor::symbols
