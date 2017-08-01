// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/ClassTypeSymbol.hpp>
#include <cmajor/symbols/InterfaceTypeSymbol.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/Sha1.hpp>
#include <llvm/IR/Module.h>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

ClassTypeSymbol::ClassTypeSymbol(const Span& span_, const std::u32string& name_) : 
    TypeSymbol(SymbolType::classTypeSymbol, span_, name_), 
    baseClass(), flags(ClassTypeSymbolFlags::none), implementedInterfaces(), templateParameters(), memberVariables(), staticMemberVariables(), 
    staticConstructor(nullptr), defaultConstructor(nullptr), copyConstructor(nullptr), moveConstructor(nullptr), copyAssignment(nullptr), moveAssignment(nullptr), 
    constructors(), destructor(nullptr), memberFunctions(), vmtPtrIndex(-1), irType(nullptr), vmtObjectType(nullptr)
{
}

ClassTypeSymbol::ClassTypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) :
    TypeSymbol(symbolType_, span_, name_),
    baseClass(), flags(ClassTypeSymbolFlags::none), implementedInterfaces(), templateParameters(), memberVariables(), staticMemberVariables(), 
    staticConstructor(nullptr), defaultConstructor(nullptr), copyConstructor(nullptr), moveConstructor(nullptr), copyAssignment(nullptr), moveAssignment(nullptr), 
    constructors(), destructor(nullptr), memberFunctions(), vmtPtrIndex(-1), irType(nullptr), vmtObjectType(nullptr)
{
}

void ClassTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    writer.GetBinaryWriter().Write(static_cast<uint8_t>(flags));
    uint32_t baseClassId = 0;
    if (baseClass)
    {
        baseClassId = baseClass->TypeId();
    }
    writer.GetBinaryWriter().WriteEncodedUInt(baseClassId);
    uint32_t n = uint32_t(implementedInterfaces.size());
    writer.GetBinaryWriter().WriteEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        InterfaceTypeSymbol* intf = implementedInterfaces[i];
        uint32_t intfTypeId = intf->TypeId();
        writer.GetBinaryWriter().WriteEncodedUInt(intfTypeId);
    }
    uint32_t vmtSize = vmt.size();
    writer.GetBinaryWriter().WriteEncodedUInt(vmtSize);
    writer.GetBinaryWriter().Write(vmtPtrIndex);
}

void ClassTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    flags = static_cast<ClassTypeSymbolFlags>(reader.GetBinaryReader().ReadByte());
    uint32_t baseClassId = reader.GetBinaryReader().ReadEncodedUInt();
    if (baseClassId != 0)
    {
        GetSymbolTable()->EmplaceTypeRequest(this, baseClassId, 0);
    }
    uint32_t n = reader.GetBinaryReader().ReadEncodedUInt();
    implementedInterfaces.resize(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        uint32_t intfTypeId = reader.GetBinaryReader().ReadEncodedUInt();
        GetSymbolTable()->EmplaceTypeRequest(this, intfTypeId, 1 + i);
    }
    uint32_t vmtSize = reader.GetBinaryReader().ReadEncodedUInt();
    vmt.resize(vmtSize);
    vmtPtrIndex = reader.GetBinaryReader().ReadInt();
    if (destructor)
    {
        if (destructor->VmtIndex() != -1)
        {
            Assert(destructor->VmtIndex() < vmt.size(), "invalid destructor vmt index");
            vmt[destructor->VmtIndex()] = destructor;
        }
    }
    for (FunctionSymbol* memberFunction : memberFunctions)
    {
        if (memberFunction->VmtIndex() != -1)
        {
            Assert(memberFunction->VmtIndex() < vmt.size(), "invalid member function vmt index");
            vmt[memberFunction->VmtIndex()] = memberFunction;
        }
    }
    if (IsPolymorphic())
    {
        GetSymbolTable()->AddPolymorphicClass(this);
    }
    reader.AddClassType(this);
}

void ClassTypeSymbol::EmplaceType(TypeSymbol* typeSymbol_, int index)
{
    if (index == 0)
    {
        Assert(typeSymbol_->GetSymbolType() == SymbolType::classTypeSymbol, "class type symbol expected");
        baseClass = static_cast<ClassTypeSymbol*>(typeSymbol_);
    }
    else 
    {
        Assert(index >= 1 && index < implementedInterfaces.size(), "invalid interface type index");
        Assert(typeSymbol_->GetSymbolType() == SymbolType::interfaceTypeSymbol, "interface type symbol expected");
        InterfaceTypeSymbol* interfaceTypeSymbol = static_cast<InterfaceTypeSymbol*>(typeSymbol_);
        implementedInterfaces[index - 1] = interfaceTypeSymbol;
    }
}

void ClassTypeSymbol::AddMember(Symbol* member)
{
    TypeSymbol::AddMember(member);
    switch (member->GetSymbolType())
    {
        case SymbolType::templateParameterSymbol:
        {
            templateParameters.push_back(static_cast<TemplateParameterSymbol*>(member));
            break;
        }
        case SymbolType::memberVariableSymbol:
        {
            if (member->IsStatic())
            {
                staticMemberVariables.push_back(static_cast<MemberVariableSymbol*>(member));
            }
            else
            {
                memberVariables.push_back(static_cast<MemberVariableSymbol*>(member));
            }
            break;
        }
        case SymbolType::staticConstructorSymbol:
        {
            if (staticConstructor)
            {
                throw Exception("already has a static constructor", member->GetSpan(), staticConstructor->GetSpan());
            }
            else
            {
                staticConstructor = static_cast<StaticConstructorSymbol*>(member);
            }
            break;
        }
        case SymbolType::constructorSymbol:
        {
            ConstructorSymbol* constructor = static_cast<ConstructorSymbol*>(member);
            constructors.push_back(constructor);
            break;
        }
        case SymbolType::destructorSymbol:
        {
            if (destructor)
            {
                throw Exception("already has a destructor", member->GetSpan(), destructor->GetSpan());
            }
            else
            {
                destructor = static_cast<DestructorSymbol*>(member);
            }
            break;
        }
        case SymbolType::memberFunctionSymbol:
        {
            memberFunctions.push_back(static_cast<MemberFunctionSymbol*>(member));
            break;
        }
    }
}

bool ClassTypeSymbol::HasNontrivialDestructor() const
{
    if (destructor || IsPolymorphic()) return true;
    int n = memberVariables.size();
    for (int i = 0; i < n; ++i)
    {
        MemberVariableSymbol* memberVariable = memberVariables[i];
        if (memberVariable->GetType()->HasNontrivialDestructor())
        {
            return true;
        }
    }
    return false;
}

void ClassTypeSymbol::CreateDestructorSymbol()
{
    if (!destructor)
    {
        AddMember(new DestructorSymbol(GetSpan(), U"@destructor"));
        Assert(destructor, "destructor expected");
        ParameterSymbol* thisParam = new ParameterSymbol(GetSpan(), U"this");
        thisParam->SetType(AddPointer(GetSpan()));
        destructor->AddMember(thisParam);
        destructor->ComputeName();
    }
}

bool ClassTypeSymbol::HasBaseClass(ClassTypeSymbol* cls) const
{
    if (!baseClass) return false;
    if (baseClass == cls || baseClass->HasBaseClass(cls)) return true;
    return false;
}

bool ClassTypeSymbol::HasBaseClass(ClassTypeSymbol* cls, uint8_t& distance) const
{
    if (!baseClass) return false;
    ++distance;
    if (baseClass == cls) return true;
    return baseClass->HasBaseClass(cls, distance);
}

void ClassTypeSymbol::AddImplementedInterface(InterfaceTypeSymbol* interfaceTypeSymbol)
{
    int n = implementedInterfaces.size();
    for (int i = 0; i < n; ++i)
    {
        if (implementedInterfaces[i] == interfaceTypeSymbol)
        {
            throw Exception("class cannot implement an interface more than once", GetSpan(), interfaceTypeSymbol->GetSpan());
        }
    }
    implementedInterfaces.push_back(interfaceTypeSymbol);
}

void ClassTypeSymbol::CloneUsingNodes(const std::vector<Node*>& usingNodes_)
{
    CloneContext cloneContext;
    for (Node* usingNode : usingNodes_)
    {
        usingNodes.Add(usingNode->Clone(cloneContext));
    }
}

void ClassTypeSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        SetStatic();
        SetDontCreateDefaultConstructor();
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        SetAbstract();
        SetPolymorphic();
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be default", GetSpan());
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be constexpr", GetSpan());
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be nothrow", GetSpan());
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be throw", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("class type symbol cannot be unit_test", GetSpan());
    }
}

void ClassTypeSymbol::SetSpecialMemberFunctions()
{
    int nc = constructors.size();
    for (int i = 0; i < nc; ++i)
    {
        ConstructorSymbol* constructor = constructors[i];
        if (constructor->IsDefaultConstructor())
        {
            defaultConstructor = constructor;
        }
        else if (constructor->IsCopyConstructor())
        {
            copyConstructor = constructor;
        }
        else if (constructor->IsMoveConstructor())
        {
            moveConstructor = constructor;
        }
    }
    int nm = memberFunctions.size();
    for (int i = 0; i < nm; ++i)
    {
        MemberFunctionSymbol* memberFunction = memberFunctions[i];
        if (memberFunction->IsCopyAssignment())
        {
            copyAssignment = memberFunction;
        }
        else if (memberFunction->IsMoveAssignment())
        {
            moveAssignment = memberFunction;
        }
    }
}

void ClassTypeSymbol::InitVmt()
{
    if (IsVmtInitialized()) return;
    SetVmtInitialized();
    if (baseClass)
    {
        baseClass->InitVmt();
        if (baseClass->IsPolymorphic())
        {
            SetPolymorphic();
        }
    }
    if (destructor && (destructor->IsVirtual() || destructor->IsOverride()))
    {
        SetPolymorphic();
    }
    for (MemberFunctionSymbol* memberFunction : memberFunctions)
    {
        if (memberFunction->IsVirtualAbstractOrOverride())
        {
            SetPolymorphic();
        }
    }
    if (!implementedInterfaces.empty())
    {
        SetPolymorphic();
    }
    if (IsPolymorphic())
    {
        CreateDestructorSymbol();
        if (baseClass && baseClass->IsPolymorphic())
        {
            destructor->SetOverride();
        }
        else
        {
            destructor->SetVirtual();
        }
    }
    if (IsPolymorphic())
    {
        if (!baseClass || !baseClass->IsPolymorphic())
        {
            if (baseClass)
            {
                vmtPtrIndex = 1;
            }
            else
            {
                vmtPtrIndex = 0;
            }
        }
        InitVmt(vmt);
        for (FunctionSymbol* virtualFunction : vmt)
        {
            if (virtualFunction->IsAbstract())
            {
                if (!IsAbstract())
                {
                    throw Exception("class containing abstract member functions must be declared abstract", GetSpan(), virtualFunction->GetSpan());
                }
            }
        }
    }
}

bool Overrides(FunctionSymbol* f, FunctionSymbol* g)
{
    if (f->GroupName() == g->GroupName())
    {
        int n = f->Parameters().size();
        if (n == g->Parameters().size())
        {
            for (int i = 1; i < n; ++i)
            {
                ParameterSymbol* p = f->Parameters()[i];
                ParameterSymbol* q = g->Parameters()[i];
                if (!TypesEqual(p->GetType(), q->GetType())) return false;
            }
            return true;
        }
    }
    return false;
}

void ClassTypeSymbol::InitVmt(std::vector<FunctionSymbol*>& vmtToInit)
{
    if (!IsPolymorphic()) return;
    if (baseClass)
    {
        baseClass->InitVmt(vmtToInit);
    }
    std::vector<FunctionSymbol*> virtualFunctions;
    if (destructor)
    {
        if (destructor->IsVirtual() || destructor->IsOverride())
        {
            virtualFunctions.push_back(destructor);
        }
    }
    for (FunctionSymbol* memberFunction : memberFunctions)
    {
        if (memberFunction->IsVirtualAbstractOrOverride())
        {
            virtualFunctions.push_back(memberFunction);
        }
    }
    int n = virtualFunctions.size();
    for (int i = 0; i < n; ++i)
    {
        FunctionSymbol* f = virtualFunctions[i];
        bool found = false;
        int m = vmtToInit.size();
        for (int j = 0; j < m; ++j)
        {
            FunctionSymbol* v = vmtToInit[j];
            if (Overrides(f, v))
            {
                if (!f->IsOverride())
                {
                    throw Exception("overriding function should be declared with override specifier", f->GetSpan());
                }
                if (f->DontThrow() != v->DontThrow())
                {
                    throw Exception("overriding function has conflicting nothrow specification compared to the base class virtual function", f->GetSpan(), v->GetSpan());
                }
                f->SetVmtIndex(j);
                vmtToInit[j] = f;
                found = true;
                break;
            }
        }
        if (!found)
        {
            if (f->IsOverride())
            {
                throw Exception("no suitable function to override ('" + ToUtf8(f->FullName()) + "')", f->GetSpan());
            }
            f->SetVmtIndex(m);
            vmtToInit.push_back(f);
        }
    }
}

bool Implements(MemberFunctionSymbol* classMemFun, MemberFunctionSymbol* intfMemFun)
{
    if (classMemFun->GroupName() != intfMemFun->GroupName()) return false;
    if (!classMemFun->ReturnType() || !intfMemFun->ReturnType()) return false;
    if (classMemFun->ReturnType() != intfMemFun->ReturnType()) return false;
    int n = classMemFun->Parameters().size();
    if (n != intfMemFun->Parameters().size()) return false;
    for (int i = 1; i < n; ++i)
    {
        TypeSymbol* classMemFunParamType = classMemFun->Parameters()[i]->GetType();
        TypeSymbol* intfMemFunParamType = intfMemFun->Parameters()[i]->GetType();
        if (!TypesEqual(classMemFunParamType, intfMemFunParamType)) return false;
    }
    return true;
}

void ClassTypeSymbol::InitImts()
{
    if (IsImtsInitialized()) return;
    SetImtsInitialized();
    int n = implementedInterfaces.size();
    if (n == 0) return;
    imts.resize(n);
    for (int32_t i = 0; i < n; ++i)
    {
        std::vector<FunctionSymbol*>& imt = imts[i];
        InterfaceTypeSymbol* intf = implementedInterfaces[i];
        int q = intf->MemberFunctions().size();
        imt.resize(q);
    }
    int m = memberFunctions.size();
    for (int j = 0; j < m; ++j)
    {
        MemberFunctionSymbol* classMemFun = memberFunctions[j];
        for (int32_t i = 0; i < n; ++i)
        {
            std::vector<FunctionSymbol*>& imt = imts[i];
            InterfaceTypeSymbol* intf = implementedInterfaces[i];
            int q = intf->MemberFunctions().size();
            for (int k = 0; k < q; ++k)
            {
                MemberFunctionSymbol* intfMemFun = intf->MemberFunctions()[k];
                if (Implements(classMemFun, intfMemFun))
                {
                    imt[intfMemFun->ImtIndex()] = classMemFun;
                    break;
                }
            }
        }
    }
    for (int i = 0; i < n; ++i)
    {
        InterfaceTypeSymbol* intf = implementedInterfaces[i];
        std::vector<FunctionSymbol*>& imt = imts[i];
        int m = imt.size();
        for (int j = 0; j < m; ++j)
        {
            if (!imt[j])
            {
                MemberFunctionSymbol* intfMemFun = intf->MemberFunctions()[j];
                throw Exception("class '" + ToUtf8(FullName()) + "' does not implement interface '" + ToUtf8(intf->FullName()) + "' because implementation of interface function '" +
                    ToUtf8(intfMemFun->FullName()) + "' is missing", GetSpan(), intfMemFun->GetSpan());
            }
        }
    }
}

void ClassTypeSymbol::CreateObjectLayout()
{
    if (IsClassObjectLayoutComputed()) return;
    SetClassObjectLayoutComputed();
    if (baseClass)
    {
        objectLayout.push_back(baseClass);
    }
    else
    {
        if (IsPolymorphic())
        {
            vmtPtrIndex = objectLayout.size();
            objectLayout.push_back(GetSymbolTable()->GetTypeByName(U"void")->AddPointer(GetSpan()));
        }
        else if (memberVariables.empty())
        {
            objectLayout.push_back(GetSymbolTable()->GetTypeByName(U"byte"));
        }
    }
    int n = memberVariables.size();
    for (int i = 0; i < n; ++i)
    {
        MemberVariableSymbol* memberVariable = memberVariables[i];
        memberVariable->SetLayoutIndex(objectLayout.size());
        objectLayout.push_back(memberVariable->GetType());
    }
}

llvm::Type* ClassTypeSymbol::IrType(Emitter& emitter)
{
    if (!irType)
    {
        std::vector<llvm::Type*> elementTypes;
        int n = objectLayout.size();
        for (int i = 0; i < n; ++i)
        {
            TypeSymbol* elementType = objectLayout[i];
            elementTypes.push_back(elementType->IrType(emitter));
        }
        irType = llvm::StructType::get(emitter.Context(), elementTypes);
    }
    return irType;
}

const std::string& ClassTypeSymbol::VmtObjectName()
{
    if (vmtObjectName.empty())
    {
        vmtObjectName = "vmt_" + ToUtf8(SimpleName()) + "_" + GetSha1MessageDigest(ToUtf8(FullNameWithSpecifiers()));
    }
    return vmtObjectName;
}

ClassTypeSymbol* ClassTypeSymbol::VmtPtrHolderClass() 
{
    if (!IsPolymorphic())
    {
        throw Exception("nonpolymorphic class does not contain a vmt ptr", GetSpan());
    }
    if (vmtPtrIndex != -1)
    {
        return this;
    }
    else
    {
        if (baseClass)
        {
            return baseClass->VmtPtrHolderClass();
        }
        else
        {
            throw Exception("vmt ptr holder class not found", GetSpan());
        }
    }
}

llvm::Type* ClassTypeSymbol::VmtPtrType(Emitter& emitter)
{
    return llvm::PointerType::get(llvm::ArrayType::get(emitter.Builder().getInt8PtrTy(), vmt.size() + functionVmtIndexOffset), 0);
}

llvm::Value* ClassTypeSymbol::VmtObject(Emitter& emitter, bool create)
{
    if (!IsPolymorphic()) return nullptr;
    if (!vmtObjectType)
    {
        vmtObjectType = llvm::ArrayType::get(emitter.Builder().getInt8PtrTy(), vmt.size() + functionVmtIndexOffset);
    }
    llvm::Constant* vmtObject = emitter.Module()->getOrInsertGlobal(VmtObjectName(), vmtObjectType);
    if (!IsVmtObjectCreated() && create)
    {
        SetVmtObjectCreated();
        llvm::GlobalVariable* vmtObjectGlobal = llvm::cast<llvm::GlobalVariable>(vmtObject);
        vmtObjectGlobal->setDLLStorageClass(llvm::GlobalValue::DLLExportStorageClass);
        std::vector<llvm::Constant*> vmtArray;
        vmtArray.push_back(llvm::Constant::getNullValue(emitter.Builder().getInt8PtrTy()));
        llvm::Value* className = emitter.Builder().CreateGlobalStringPtr(ToUtf8(FullName()));
        vmtArray.push_back(llvm::cast<llvm::Constant>(emitter.Builder().CreateBitCast(className, emitter.Builder().getInt8PtrTy())));
        vmtArray.push_back(llvm::Constant::getNullValue(emitter.Builder().getInt8PtrTy()));
        int n = vmt.size();
        for (int i = 0; i < n; ++i)
        {
            FunctionSymbol* virtualFunction = vmt[i];
            if (!virtualFunction)
            {
                vmtArray.push_back(llvm::Constant::getNullValue(emitter.Builder().getInt8PtrTy()));
            }
            else
            {
                llvm::Function* functionObject = llvm::cast<llvm::Function>(emitter.Module()->getOrInsertFunction(ToUtf8(virtualFunction->MangledName()), virtualFunction->IrType(emitter)));
                vmtArray.push_back(llvm::cast<llvm::Constant>(emitter.Builder().CreateBitCast(functionObject, emitter.Builder().getInt8PtrTy())));
            }
        }
        vmtObjectGlobal->setInitializer(llvm::ConstantArray::get(vmtObjectType, vmtArray));
    }
    return vmtObject;
}

TemplateParameterSymbol::TemplateParameterSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::templateParameterSymbol, span_, name_)
{
}

BoundTemplateParameterSymbol::BoundTemplateParameterSymbol(const Span& span_, const std::u32string& name_) : Symbol(SymbolType::boundTemplateParameterSymbol, span_, name_)
{
}

} } // namespace cmajor::symbols