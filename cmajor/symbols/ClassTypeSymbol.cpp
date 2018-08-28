// =================================
// Copyright (c) 2018 Seppo Laakko
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
#include <cmajor/symbols/TemplateSymbol.hpp>
#include <cmajor/symbols/Module.hpp>
#include <cmajor/symbols/SymbolCollector.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/Sha1.hpp>
#include <llvm/IR/Module.h>
#include <boost/uuid/uuid_generators.hpp>
#include <boost/uuid/uuid_io.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

ClassGroupTypeSymbol::ClassGroupTypeSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::classGroupTypeSymbol, span_, name_)
{
}

llvm::Type* ClassGroupTypeSymbol::IrType(Emitter& emitter)
{
    Assert(false, "tried to get ir type of class group");
    return nullptr;
}

llvm::Constant* ClassGroupTypeSymbol::CreateDefaultIrValue(Emitter& emitter)
{
    Assert(false, "tried to create default ir value of class group");
    return nullptr;
}

void ClassGroupTypeSymbol::AddClass(ClassTypeSymbol* classTypeSymbol)
{
    for (int arity = classTypeSymbol->MinArity(); arity <= classTypeSymbol->MaxArity(); ++arity)
    {
        if (arityClassMap.find(arity) != arityClassMap.cend())
        {
            throw Exception(classTypeSymbol->GetModule(), "already has class with arity " + std::to_string(arity) + " in class group '" + ToUtf8(Name()) + "'", GetSpan(), classTypeSymbol->GetSpan());
        }
        arityClassMap[arity] = classTypeSymbol;
    }
}

ClassTypeSymbol* ClassGroupTypeSymbol::GetClass(int arity) const
{
    auto it = arityClassMap.find(arity);
    if (it != arityClassMap.cend())
    {
        ClassTypeSymbol* classTypeSymbol = it->second;
        return classTypeSymbol;
    }
    return nullptr;
}

bool ClassGroupTypeSymbol::HasProjectMembers() const
{
    for (const auto& p : arityClassMap)
    {
        ClassTypeSymbol* cls = p.second;
        if (cls->IsProject())
        {
            return true;
        }
    }
    return false;
}

void ClassGroupTypeSymbol::AppendChildElements(dom::Element* element, TypeMap& typeMap) const
{
    for (const auto& p : arityClassMap)
    {
        ClassTypeSymbol* cls = p.second;
        std::unique_ptr<dom::Element> classElement = cls->ToDomElement(typeMap);
        if (cls->IsProject())
        {
            element->AppendChild(std::unique_ptr<dom::Node>(classElement.release()));
        }
    }
}

ClassTypeSymbol::ClassTypeSymbol(const Span& span_, const std::u32string& name_) : 
    TypeSymbol(SymbolType::classTypeSymbol, span_, name_), 
    minArity(0), baseClass(), flags(ClassTypeSymbolFlags::none), implementedInterfaces(), templateParameters(), memberVariables(), staticMemberVariables(),
    staticConstructor(nullptr), defaultConstructor(nullptr), copyConstructor(nullptr), moveConstructor(nullptr), copyAssignment(nullptr), moveAssignment(nullptr), 
    constructors(), destructor(nullptr), memberFunctions(), vmtPtrIndex(-1), irType(nullptr), vmtObjectType(nullptr), staticObjectType(nullptr), prototype(nullptr)
{
}

ClassTypeSymbol::ClassTypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) :
    TypeSymbol(symbolType_, span_, name_),
    minArity(0), baseClass(), flags(ClassTypeSymbolFlags::none), implementedInterfaces(), templateParameters(), memberVariables(), staticMemberVariables(),
    staticConstructor(nullptr), defaultConstructor(nullptr), copyConstructor(nullptr), moveConstructor(nullptr), copyAssignment(nullptr), moveAssignment(nullptr), 
    constructors(), destructor(nullptr), memberFunctions(), vmtPtrIndex(-1), irType(nullptr), vmtObjectType(nullptr), staticObjectType(nullptr), prototype(nullptr)
{
}

void ClassTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    writer.GetBinaryWriter().Write(groupName);
    writer.GetBinaryWriter().Write(static_cast<uint8_t>(flags & ~ClassTypeSymbolFlags::layoutsComputed));
    writer.GetBinaryWriter().Write(static_cast<int32_t>(minArity));
    if (IsClassTemplate())
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
        bool hasConstraint = constraint != nullptr;
        writer.GetBinaryWriter().Write(hasConstraint);
        if (hasConstraint)
        {
            writer.GetAstWriter().Write(constraint.get());
        }
        bool hasPrototype = prototype != nullptr;
        writer.GetBinaryWriter().Write(hasPrototype);
        if (hasPrototype)
        {
            writer.GetBinaryWriter().Write(prototype->TypeId());
        }
    }
    else if (GetSymbolType() == SymbolType::classTypeSymbol)
    {
        boost::uuids::uuid baseClassId = boost::uuids::nil_generator()();
        if (baseClass)
        {
            baseClassId = baseClass->TypeId();
        }
        writer.GetBinaryWriter().Write(baseClassId);
        uint32_t n = uint32_t(implementedInterfaces.size());
        writer.GetBinaryWriter().WriteULEB128UInt(n);
        for (uint32_t i = 0; i < n; ++i)
        {
            InterfaceTypeSymbol* intf = implementedInterfaces[i];
            const boost::uuids::uuid& intfTypeId = intf->TypeId();
            writer.GetBinaryWriter().Write(intfTypeId);
        }
        uint32_t vmtSize = vmt.size();
        writer.GetBinaryWriter().WriteULEB128UInt(vmtSize);
        writer.GetBinaryWriter().Write(vmtPtrIndex);
        bool hasConstraint = constraint != nullptr;
        writer.GetBinaryWriter().Write(hasConstraint);
        if (hasConstraint)
        {
            writer.GetAstWriter().Write(constraint.get());
        }
    }
    else if (GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        bool hasConstraint = constraint != nullptr;
        writer.GetBinaryWriter().Write(hasConstraint);
        if (hasConstraint)
        {
            writer.GetAstWriter().Write(constraint.get());
        }
    }
}

void ClassTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    groupName = reader.GetBinaryReader().ReadUtf32String();
    flags = static_cast<ClassTypeSymbolFlags>(reader.GetBinaryReader().ReadByte());
    minArity = reader.GetBinaryReader().ReadInt();
    if (IsClassTemplate())
    {
        sizeOfAstNodes = reader.GetBinaryReader().ReadUInt();
        astNodesPos = reader.GetBinaryReader().Pos();
        reader.GetBinaryReader().Skip(sizeOfAstNodes);
        filePathReadFrom = reader.GetBinaryReader().FileName();
        bool hasConstraint = reader.GetBinaryReader().ReadBool();
        if (hasConstraint)
        {
            constraint.reset(reader.GetAstReader().ReadConstraintNode());
        }
        bool hasPrototype = reader.GetBinaryReader().ReadBool();
        if (hasPrototype)
        {
            boost::uuids::uuid prototypeId;
            reader.GetBinaryReader().ReadUuid(prototypeId);
            GetSymbolTable()->EmplaceTypeRequest(this, prototypeId, 1000000);
        }
    }
    else if (GetSymbolType() == SymbolType::classTypeSymbol)
    {
        boost::uuids::uuid baseClassId;
        reader.GetBinaryReader().ReadUuid(baseClassId);
        if (!baseClassId.is_nil())
        {
            GetSymbolTable()->EmplaceTypeRequest(this, baseClassId, 0);
        }
        uint32_t n = reader.GetBinaryReader().ReadULEB128UInt();
        implementedInterfaces.resize(n);
        for (uint32_t i = 0; i < n; ++i)
        {
            boost::uuids::uuid intfTypeId;
            reader.GetBinaryReader().ReadUuid(intfTypeId);
            GetSymbolTable()->EmplaceTypeRequest(this, intfTypeId, 1 + i);
        }
        uint32_t vmtSize = reader.GetBinaryReader().ReadULEB128UInt();
        vmt.resize(vmtSize);
        vmtPtrIndex = reader.GetBinaryReader().ReadInt();
        bool hasConstraint = reader.GetBinaryReader().ReadBool();
        if (hasConstraint)
        {
            constraint.reset(reader.GetAstReader().ReadConstraintNode());
        }
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
        if (IsPolymorphic() && !IsPrototypeTemplateSpecialization())
        {
            GetSymbolTable()->AddPolymorphicClass(this);
        }
        if (StaticConstructor())
        {
            GetSymbolTable()->AddClassHavingStaticConstructor(this);
        }
        reader.AddClassType(this);
    }
    else if (GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        bool hasConstraint = reader.GetBinaryReader().ReadBool();
        if (hasConstraint)
        {
            constraint.reset(reader.GetAstReader().ReadConstraintNode());
        }
    }
}

void ClassTypeSymbol::ComputeExportClosure()
{
    if (IsProject())
    {
        ContainerSymbol::ComputeExportClosure();
        if (baseClass)
        {
            if (!baseClass->ExportComputed())
            {
                baseClass->SetExportComputed();
                baseClass->ComputeExportClosure();
            }
        }
    }
}

void ClassTypeSymbol::ReadAstNodes()
{
    AstReader reader(filePathReadFrom);
    reader.GetBinaryReader().Skip(astNodesPos);
    usingNodes.Read(reader);
    Node* node = reader.ReadNode();
    Assert(node->GetNodeType() == NodeType::classNode, "class node expected");
    ClassNode* clsNode = static_cast<ClassNode*>(node);
    classNode.reset(clsNode);
    GetSymbolTable()->MapNode(clsNode, this);
}

void ClassTypeSymbol::EmplaceType(TypeSymbol* typeSymbol, int index)
{
    if (index == 0)
    {
        Assert(typeSymbol->GetSymbolType() == SymbolType::classTypeSymbol || typeSymbol->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol, "class type symbol expected");
        baseClass = static_cast<ClassTypeSymbol*>(typeSymbol);
        GetContainerScope()->SetBase(baseClass->GetContainerScope());
    }
    else if (index >= 1)
    {
        if (index == 1000000)
        {
            Assert(typeSymbol->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol, "class template specialization expected");
            SetPrototype(static_cast<ClassTemplateSpecializationSymbol*>(typeSymbol));
        }
        else
        {
            Assert(typeSymbol->GetSymbolType() == SymbolType::interfaceTypeSymbol, "interface type symbol expected");
            InterfaceTypeSymbol* interfaceTypeSymbol = static_cast<InterfaceTypeSymbol*>(typeSymbol);
            implementedInterfaces[index - 1] = interfaceTypeSymbol;
        }
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
                throw Exception(GetModule(), "already has a static constructor", member->GetSpan(), staticConstructor->GetSpan());
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
                throw Exception(GetModule(), "already has a destructor", member->GetSpan(), destructor->GetSpan());
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

std::string ClassTypeSymbol::GetSpecifierStr() const
{
    std::string specifierStr;
    if (IsAbstract())
    {
        specifierStr.append("abstract");
    }
    std::string baseSpecifierStr = TypeSymbol::GetSpecifierStr();
    if (!baseSpecifierStr.empty())
    {
        specifierStr.append(1, ' ');
    }
    specifierStr.append(baseSpecifierStr);
    return specifierStr;
}

bool ClassTypeSymbol::HasNontrivialDestructor() const
{
    if (destructor || IsPolymorphic()) return true;
    if (baseClass && baseClass->Destructor()) return true;
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

void ClassTypeSymbol::Accept(SymbolCollector* collector)
{
    if (IsProject())
    {
        if (prototype)
        {
            prototype->SetParent(Ns());
            if (Constraint())
            {
                CloneContext cloneContext;
                prototype->SetConstraint(static_cast<ConstraintNode*>(Constraint()->Clone(cloneContext)));
            }
            collector->AddClass(prototype);
        }
        else
        {
            if (Access() == SymbolAccess::public_)
            {
                collector->AddClass(this);
            }
        }
    }
}

void ClassTypeSymbol::CollectMembers(SymbolCollector* collector)
{
    TypeSymbol::Accept(collector);
}

void ClassTypeSymbol::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine(ToUtf8(Name()));
    formatter.WriteLine("group name: " + ToUtf8(groupName));
    formatter.WriteLine("full name: " + ToUtf8(FullNameWithSpecifiers()));
    if (baseClass)
    {
        formatter.WriteLine("base class: " + ToUtf8(baseClass->FullName()));
    }
    if (!implementedInterfaces.empty())
    {
        formatter.WriteLine("implemented interfaces:");
        formatter.IncIndent();
        for (InterfaceTypeSymbol* interface : implementedInterfaces)
        {
            formatter.WriteLine(ToUtf8(interface->FullName()));
        }
        formatter.DecIndent();
    }
    if (constraint)
    {
        formatter.WriteLine("constraint: " + constraint->ToString());
    }
    formatter.WriteLine("typeid: " + boost::uuids::to_string(TypeId()));
    if (IsPolymorphic())
    {
        formatter.WriteLine("vmt object name: " + VmtObjectName());
    }
    if (!staticLayout.empty())
    {
        formatter.WriteLine("static object name: " + StaticObjectName());
    }
    formatter.IncIndent();
    SymbolCollector collector;
    TypeSymbol::Accept(&collector);
    if (!collector.Functions().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("MEMBER FUNCTIONS");
        for (FunctionSymbol* function : collector.Functions())
        {
            formatter.WriteLine();
            function->Dump(formatter);
        }
    }
    if (!collector.Classes().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("CLASSES");
        for (ClassTypeSymbol* class_ : collector.Classes())
        {
            formatter.WriteLine();
            class_->Dump(formatter);
        }
    }
    if (!collector.Interfaces().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("INTERFACES");
        for (InterfaceTypeSymbol* interface : collector.Interfaces())
        {
            formatter.WriteLine();
            interface->Dump(formatter);
        }
    }
    if (!collector.Typedefs().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("TYPEDEFS");
        for (TypedefSymbol* typedef_ : collector.Typedefs())
        {
            formatter.WriteLine();
            typedef_->Dump(formatter);
        }
    }
    if (!collector.Concepts().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("CONCEPTS");
        for (ConceptSymbol* concept_ : collector.Concepts())
        {
            formatter.WriteLine();
            concept_->Dump(formatter);
        }
    }
    if (!collector.Constants().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("CONSTANTS");
        for (ConstantSymbol* constant : collector.Constants())
        {
            formatter.WriteLine();
            constant->Dump(formatter);
        }
    }
    if (!collector.Delegates().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("DELEGATES");
        for (DelegateTypeSymbol* delegate_ : collector.Delegates())
        {
            formatter.WriteLine();
            delegate_->Dump(formatter);
        }
    }
    if (!collector.ClassDelegates().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("CLASS DELEGATES");
        for (ClassDelegateTypeSymbol* classDelegate : collector.ClassDelegates())
        {
            formatter.WriteLine();
            classDelegate->Dump(formatter);
        }
    }
    if (!collector.EnumeratedTypes().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("ENUMERATED TYPES");
        for (EnumTypeSymbol* enumeratedType : collector.EnumeratedTypes())
        {
            formatter.WriteLine();
            enumeratedType->Dump(formatter);
        }
    }
    if (!collector.MemberVariables().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("MEMBER VARIABLES");
        for (MemberVariableSymbol* memberVariable : collector.MemberVariables())
        {
            formatter.WriteLine();
            memberVariable->Dump(formatter);
        }
    }
    formatter.DecIndent();
}

bool ClassTypeSymbol::IsRecursive(TypeSymbol* type, std::unordered_set<TypeSymbol*>& tested) 
{
    if (tested.find(type) != tested.cend()) return type == this;
    tested.insert(this);
    if (TypeSymbol::IsRecursive(type, tested)) return true;
    if (baseClass && baseClass->IsRecursive(type, tested)) return true;
    for (MemberVariableSymbol* memberVariable : memberVariables)
    {
        if (memberVariable->GetType()->IsRecursive(type, tested)) return true;
    }
    return false;
}

void ClassTypeSymbol::CreateDestructorSymbol()
{
    if (!destructor)
    {
        DestructorSymbol* destructorSymbol = new DestructorSymbol(GetSpan(), U"@destructor");
        destructorSymbol->SetModule(GetModule());
        destructorSymbol->SetOriginalModule(GetOriginalModule());
        destructorSymbol->SetSymbolTable(GetSymbolTable());
        GetSymbolTable()->SetFunctionIdFor(destructorSymbol);
        destructorSymbol->SetGenerated();
        ParameterSymbol* thisParam = new ParameterSymbol(GetSpan(), U"this");
        thisParam->SetType(AddPointer(GetSpan()));
        destructorSymbol->SetAccess(SymbolAccess::public_);
        destructorSymbol->AddMember(thisParam);
        AddMember(destructorSymbol);
        Assert(destructor, "destructor expected");
        if (GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
        {
            destructor->SetLinkOnceOdrLinkage();
        }
        destructor->ComputeName();
    }
}

void ClassTypeSymbol::SetGroupName(const std::u32string& groupName_)
{
    groupName = groupName_;
}

void ClassTypeSymbol::ComputeMinArity()
{
    bool defaultHit = false;
    int n = templateParameters.size();
    for (int i = 0; i < n; ++i)
    {
        TemplateParameterSymbol* templateParameter = templateParameters[i];
        if (templateParameter->HasDefault())
        {
            defaultHit = true;
            break;
        }
        minArity = i;
    }
    if (!defaultHit)
    {
        minArity = n;
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
            throw Exception(GetModule(), "class cannot implement an interface more than once", GetSpan(), interfaceTypeSymbol->GetSpan());
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
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        SetAbstract();
        SetPolymorphic();
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be default", GetSpan());
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be constexpr", GetSpan());
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be nothrow", GetSpan());
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be throw", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception(GetModule(), "class type symbol cannot be unit_test", GetSpan());
    }
}

void ClassTypeSymbol::ComputeName()
{
    std::u32string name = Name();
    if (IsClassTemplate())
    {
        name.append(1, '<');
        bool first = true;
        for (TemplateParameterSymbol* templateParameter : templateParameters)
        {
            if (first)
            {
                first = false;
            }
            else
            {
                name.append(U", ");
            }
            name.append(templateParameter->Name());
        }
        name.append(1, '>');
    }
    SetName(name);
    ComputeMangledName();
}

void ClassTypeSymbol::ComputeMangledName()
{
    std::u32string mangledName = ToUtf32(TypeString());
    mangledName.append(1, U'_').append(SimpleName());
    std::string constraintStr;
    if (constraint)
    {
        constraintStr = " " + constraint->ToString();
    }
    mangledName.append(1, U'_').append(ToUtf32(GetSha1MessageDigest(ToUtf8(FullNameWithSpecifiers()) + constraintStr)));
    SetMangledName(mangledName);
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

bool ClassTypeSymbol::IsLiteralClassType() const
{
    if (IsPolymorphic()) return false;
    if (HasNontrivialDestructor()) return false;
    if (!StaticMemberVariables().empty()) return false;
    if (BaseClass()) return false;
    return true;
}

void ClassTypeSymbol::SetInitializedVar(MemberVariableSymbol* initializedVar_) 
{ 
    initializedVar.reset(initializedVar_); 
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
            break;
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
                    throw Exception(GetModule(), "class containing abstract member functions must be declared abstract", GetSpan(), virtualFunction->GetSpan());
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
                    throw Exception(GetModule(), "overriding function should be declared with override specifier", f->GetSpan());
                }
                if (f->DontThrow() != v->DontThrow())
                {
                    throw Exception(GetModule(), "overriding function has conflicting nothrow specification compared to the base class virtual function", f->GetSpan(), v->GetSpan());
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
                throw Exception(GetModule(), "no suitable function to override ('" + ToUtf8(f->FullName()) + "')", f->GetSpan());
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
                throw Exception(GetModule(), "class '" + ToUtf8(FullName()) + "' does not implement interface '" + ToUtf8(intf->FullName()) + "' because implementation of interface function '" +
                    ToUtf8(intfMemFun->FullName()) + "' is missing", GetSpan(), intfMemFun->GetSpan());
            }
        }
    }
}

void ClassTypeSymbol::CreateLayouts()
{
    if (IsLayoutsComputed()) return;
    SetLayoutsComputed();
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
    if (!staticMemberVariables.empty() || StaticConstructor())
    {
        MemberVariableSymbol* initVar = new MemberVariableSymbol(GetSpan(), U"@initialized");
        initVar->SetParent(this);
        initVar->SetStatic();
        initVar->SetType(GetSymbolTable()->GetTypeByName(U"bool"));
        initVar->SetLayoutIndex(0);
        SetInitializedVar(initVar);
        staticLayout.push_back(GetSymbolTable()->GetTypeByName(U"bool"));
        int ns = staticMemberVariables.size();
        for (int i = 0; i < ns; ++i)
        {
            MemberVariableSymbol* staticMemberVariable = staticMemberVariables[i];
            staticMemberVariable->SetLayoutIndex(staticLayout.size());
            staticLayout.push_back(staticMemberVariable->GetType());
        }
    }
}

llvm::Type* ClassTypeSymbol::IrType(Emitter& emitter)
{
    bool recursive = false;
    if (!irType)
    {
        std::vector<llvm::Type*> elementTypes;
        int n = objectLayout.size();
        for (int i = 0; i < n; ++i)
        {
            TypeSymbol* elementType = objectLayout[i];
            std::unordered_set<TypeSymbol*> tested;
            if (elementType->IsRecursive(this, tested))
            {
                recursive = true;
                break;
            }
        }
        if (!recursive)
        {
            for (int i = 0; i < n; ++i)
            {
                TypeSymbol* elementType = objectLayout[i];
                elementTypes.push_back(elementType->IrType(emitter));
            }
            irType = llvm::StructType::get(emitter.Context(), elementTypes);
        }
        else
        {
            llvm::StructType* forwardDeclaredType = llvm::StructType::create(emitter.Context());
            irType = forwardDeclaredType;
            for (int i = 0; i < n; ++i)
            {
                TypeSymbol* elementType = objectLayout[i];
                elementTypes.push_back(elementType->IrType(emitter));
            }
            forwardDeclaredType->setBody(elementTypes);
        }
    }
    return irType;
}

llvm::Constant* ClassTypeSymbol::CreateDefaultIrValue(Emitter& emitter)
{
    llvm::Type* irType = IrType(emitter);
    std::vector<llvm::Constant*> arrayOfDefaults;
    for (TypeSymbol* type : objectLayout)
    {
        arrayOfDefaults.push_back(type->CreateDefaultIrValue(emitter));
    }
    return llvm::ConstantStruct::get(llvm::cast<llvm::StructType>(irType), arrayOfDefaults);
}

llvm::DIType* ClassTypeSymbol::CreateDIType(Emitter& emitter)
{
    llvm::DIType* baseClassDIType = nullptr;
    if (baseClass)
    {
        baseClassDIType = baseClass->GetDIType(emitter);
    }
    llvm::DIType* vtableHolderClass = nullptr;
    if (IsPolymorphic() && VmtPtrHolderClass())
    {
        vtableHolderClass = VmtPtrHolderClass()->CreateDIForwardDeclaration(emitter);
        emitter.MapFwdDeclaration(vtableHolderClass, VmtPtrHolderClass());
    }
    std::vector<llvm::Metadata*> elements;
    const llvm::StructLayout* structLayout = emitter.DataLayout()->getStructLayout(llvm::cast<llvm::StructType>(IrType(emitter)));
    for (MemberVariableSymbol* memberVariable : memberVariables)
    {
        int memberVariableLayoutIndex = memberVariable->LayoutIndex();
        uint64_t offsetInBits = structLayout->getElementOffsetInBits(memberVariableLayoutIndex);
        elements.push_back(memberVariable->GetDIMemberType(emitter, offsetInBits));
    }
    llvm::MDNode* templateParams = nullptr;
    uint64_t sizeInBits = structLayout->getSizeInBits();
    uint32_t alignInBits = 8 * structLayout->getAlignment();
    uint64_t offsetInBits = 0; // todo?
    llvm::DINode::DIFlags flags = llvm::DINode::DIFlags::FlagZero;
    Span classSpan = GetSpan();
    if (GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        ClassTemplateSpecializationSymbol* specialization = static_cast<ClassTemplateSpecializationSymbol*>(this);
        classSpan = specialization->GetClassTemplate()->GetSpan();
    }
    return emitter.DIBuilder()->createClassType(nullptr, ToUtf8(Name()), emitter.GetFile(classSpan.FileIndex()), classSpan.LineNumber(), sizeInBits, alignInBits, offsetInBits,
        flags, baseClassDIType, emitter.DIBuilder()->getOrCreateArray(elements), vtableHolderClass, templateParams, ToUtf8(MangledName()));
}

llvm::DIType* ClassTypeSymbol::CreateDIForwardDeclaration(Emitter& emitter)
{
    Span classSpan = GetSpan();
    if (GetSymbolType() == SymbolType::classTemplateSpecializationSymbol)
    {
        ClassTemplateSpecializationSymbol* specialization = static_cast<ClassTemplateSpecializationSymbol*>(this);
        classSpan = specialization->GetClassTemplate()->GetSpan();
    }
    uint64_t sizeInBits = emitter.DataLayout()->getStructLayout(llvm::cast<llvm::StructType>(IrType(emitter)))->getSizeInBits();
    uint32_t alignInBits = 8 * emitter.DataLayout()->getStructLayout(llvm::cast<llvm::StructType>(IrType(emitter)))->getAlignment();
    uint64_t offsetInBits = 0; // todo?
    return emitter.DIBuilder()->createReplaceableCompositeType(llvm::dwarf::DW_TAG_class_type, ToUtf8(Name()), nullptr, emitter.GetFile(classSpan.FileIndex()), classSpan.LineNumber(),
        0, sizeInBits, alignInBits, llvm::DINode::DIFlags::FlagZero, ToUtf8(MangledName()));
}

const std::string& ClassTypeSymbol::VmtObjectName()
{
    if (vmtObjectName.empty())
    {
        vmtObjectName = "vmt_" + ToUtf8(SimpleName()) + "_" + GetSha1MessageDigest(ToUtf8(FullNameWithSpecifiers()));
    }
    return vmtObjectName;
}

const std::string& ClassTypeSymbol::ImtArrayObjectName()
{
    if (itabsArrayObjectName.empty())
    {
        itabsArrayObjectName = "imts_" + ToUtf8(SimpleName()) + "_" + GetSha1MessageDigest(ToUtf8(FullNameWithSpecifiers()));
    }
    return itabsArrayObjectName;
}

std::string ClassTypeSymbol::ImtObjectName(int index)
{ 
    return "imt_" + std::to_string(index) + "_" + ToUtf8(SimpleName()) + "_" + GetSha1MessageDigest(ToUtf8(FullNameWithSpecifiers()));
}

ClassTypeSymbol* ClassTypeSymbol::VmtPtrHolderClass() 
{
    if (!IsPolymorphic())
    {
        throw Exception(GetModule(), "nonpolymorphic class does not contain a vmt ptr", GetSpan());
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
            throw Exception(GetModule(), "vmt ptr holder class not found", GetSpan());
        }
    }
}

llvm::Type* ClassTypeSymbol::VmtPtrType(Emitter& emitter)
{
    return llvm::PointerType::get(llvm::ArrayType::get(emitter.Builder().getInt8PtrTy(), vmt.size() + functionVmtIndexOffset), 0);
}

llvm::Value* ClassTypeSymbol::CreateImt(Emitter& emitter, int index)
{
    std::vector<FunctionSymbol*>& imt = imts[index];
    std::string imtObjectName = ImtObjectName(index);
    llvm::ArrayType* imtType = llvm::ArrayType::get(emitter.Builder().getInt8PtrTy(), imt.size());
    llvm::Constant* imtObject = emitter.Module()->getOrInsertGlobal(imtObjectName, imtType);
    llvm::Comdat* comdat = emitter.Module()->getOrInsertComdat(imtObjectName);
    llvm::GlobalVariable* imtObjectGlobal = llvm::cast<llvm::GlobalVariable>(imtObject);
    imtObjectGlobal->setComdat(comdat);
    std::vector<llvm::Constant*> irImt;
    int n = imt.size();
    for (int i = 0; i < n; ++i)
    {
        FunctionSymbol* memFun = imt[i];
        llvm::Function* interfaceFun = llvm::cast<llvm::Function>(emitter.Module()->getOrInsertFunction(ToUtf8(memFun->MangledName()), memFun->IrType(emitter)));
        irImt.push_back(llvm::cast<llvm::Constant>(emitter.Builder().CreateBitCast(interfaceFun, emitter.Builder().getInt8PtrTy())));
    }
    imtObjectGlobal->setInitializer(llvm::ConstantArray::get(imtType, irImt));
    return imtObject;
}

llvm::Value* ClassTypeSymbol::CreateImts(Emitter& emitter)
{
    std::string imtArrayObjectName = ImtArrayObjectName();
    llvm::ArrayType* imtsArrayType = llvm::ArrayType::get(emitter.Builder().getInt8PtrTy(), implementedInterfaces.size());
    llvm::Constant* imtsArrayObject = emitter.Module()->getOrInsertGlobal(itabsArrayObjectName, imtsArrayType);
    llvm::Comdat* comdat = emitter.Module()->getOrInsertComdat(itabsArrayObjectName);
    llvm::GlobalVariable* imtsArrayObjectGlobal = llvm::cast<llvm::GlobalVariable>(imtsArrayObject);
    imtsArrayObjectGlobal->setComdat(comdat);
    std::vector<llvm::Constant*> imtsArray;
    int n = imts.size();
    for (int i = 0; i < n; ++i)
    {
        llvm::Value* irImt = CreateImt(emitter, i);
        imtsArray.push_back(llvm::cast<llvm::Constant>(emitter.Builder().CreateBitCast(irImt, emitter.Builder().getInt8PtrTy())));
    }
    imtsArrayObjectGlobal->setInitializer(llvm::ConstantArray::get(imtsArrayType, imtsArray));
    return imtsArrayObject;
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
        std::string vmtObjectName = VmtObjectName();
        llvm::Comdat* comdat = emitter.Module()->getOrInsertComdat(vmtObjectName);
        GetModule()->AddExportedData(vmtObjectName);
        comdat->setSelectionKind(llvm::Comdat::SelectionKind::Any);
        llvm::GlobalVariable* vmtObjectGlobal = llvm::cast<llvm::GlobalVariable>(vmtObject);
        vmtObjectGlobal->setComdat(comdat);
        std::vector<llvm::Constant*> vmtArray;
        vmtArray.push_back(llvm::Constant::getNullValue(emitter.Builder().getInt8PtrTy()));
        llvm::Value* className = emitter.Builder().CreateGlobalStringPtr(ToUtf8(FullName()));
        vmtArray.push_back(llvm::cast<llvm::Constant>(emitter.Builder().CreateBitCast(className, emitter.Builder().getInt8PtrTy())));
        if (!implementedInterfaces.empty())
        {
            llvm::Value* itabsArrayObject = CreateImts(emitter);
            vmtArray.push_back(llvm::cast<llvm::Constant>(emitter.Builder().CreateBitCast(itabsArrayObject, emitter.Builder().getInt8PtrTy())));
        }
        else
        {
            vmtArray.push_back(llvm::Constant::getNullValue(emitter.Builder().getInt8PtrTy()));
        }
        int n = vmt.size();
        for (int i = 0; i < n; ++i)
        {
            FunctionSymbol* virtualFunction = vmt[i];
            if (!virtualFunction || virtualFunction->IsAbstract())
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

llvm::Value* ClassTypeSymbol::StaticObject(Emitter& emitter, bool create)
{
    if (staticLayout.empty()) return nullptr;
    llvm::Constant* staticObject = emitter.Module()->getOrInsertGlobal(StaticObjectName(), StaticObjectType(emitter));
    if (!IsStaticObjectCreated() && create)
    {
        SetStaticObjectCreated();
        llvm::GlobalVariable* staticObjectGlobal = llvm::cast<llvm::GlobalVariable>(staticObject);
        std::vector<llvm::Constant*> arrayOfStatics;
        for (TypeSymbol* type : staticLayout)
        {
            arrayOfStatics.push_back(type->CreateDefaultIrValue(emitter));
        }
        staticObjectGlobal->setInitializer(llvm::ConstantStruct::get(StaticObjectType(emitter), arrayOfStatics));
    }
    return staticObject;
}

llvm::StructType* ClassTypeSymbol::StaticObjectType(Emitter& emitter)
{
    if (!staticObjectType)
    {
        std::vector<llvm::Type*> elementTypes;
        int n = staticLayout.size();
        for (int i = 0; i < n; ++i)
        {
            llvm::Type* elementType = staticLayout[i]->IrType(emitter);
            elementTypes.push_back(elementType);
        }
        staticObjectType = llvm::StructType::get(emitter.Context(), elementTypes);
    }
    return staticObjectType;
}

const std::string& ClassTypeSymbol::StaticObjectName()
{
    if (staticObjectName.empty())
    {
        staticObjectName = "statics_" + ToUtf8(SimpleName()) + "_" + GetSha1MessageDigest(ToUtf8(FullNameWithSpecifiers()));
    }
    return staticObjectName;
}

ValueType ClassTypeSymbol::GetValueType() const
{
    return ValueType::structuredValue;
}

Value* ClassTypeSymbol::MakeValue() const
{
    std::vector<std::unique_ptr<Value>> memberValues;
    return new StructuredValue(GetSpan(), const_cast<TypeSymbol*>(static_cast<const TypeSymbol*>(this)), std::move(memberValues));
}

std::u32string ClassTypeSymbol::Id() const
{
    if (prototype)
    {
        return prototype->Id();
    }
    else
    {
        return MangledName();
    }
}

} } // namespace cmajor::symbols
