// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/BasicTypeSymbol.hpp>
#include <cmajor/symbols/BasicTypeOperation.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/ClassTypeSymbol.hpp>
#include <cmajor/symbols/InterfaceTypeSymbol.hpp>
#include <cmajor/symbols/DelegateSymbol.hpp>
#include <cmajor/symbols/TypedefSymbol.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/ConstantSymbol.hpp>
#include <cmajor/symbols/EnumSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/TemplateSymbol.hpp>
#include <cmajor/symbols/ConceptSymbol.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/util/Unicode.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

void TypeIdCounter::Init()
{
    instance.reset(new TypeIdCounter());
}

void TypeIdCounter::Done()
{
    instance.reset();
}

std::unique_ptr<TypeIdCounter> TypeIdCounter::instance;

TypeIdCounter::TypeIdCounter() : nextTypeId(1)
{
}

void FunctionIdCounter::Init()
{
    instance.reset(new FunctionIdCounter());
}

void FunctionIdCounter::Done()
{
    instance.reset();
}

std::unique_ptr<FunctionIdCounter> FunctionIdCounter::instance;

FunctionIdCounter::FunctionIdCounter() : nextFunctionId(1)
{
}

bool operator==(const ClassTemplateSpecializationKey& left, const ClassTemplateSpecializationKey& right)
{
    if (!TypesEqual(left.classTemplate, right.classTemplate)) return false;
    int n = left.templateArgumentTypes.size();
    if (n != right.templateArgumentTypes.size()) return false;
    for (int i = 0; i < n; ++i)
    {
        if (!TypesEqual(left.templateArgumentTypes[i], right.templateArgumentTypes[i])) return false;
    }
    return true;
}

bool operator!=(const ClassTemplateSpecializationKey& left, const ClassTemplateSpecializationKey& right)
{
    return !(left == right);
}

bool operator==(const ArrayKey& left, const ArrayKey& right)
{
    if (!TypesEqual(left.elementType, right.elementType)) return false;
    if (left.size != right.size) return false;
    return true;
}

bool operator!=(const ArrayKey& left, const ArrayKey& right)
{
    return !(left == right);
}

SymbolTable::SymbolTable() : 
    globalNs(Span(), std::u32string()), currentCompileUnit(nullptr), container(&globalNs), currentClass(nullptr), currentInterface(nullptr), mainFunctionSymbol(nullptr), 
    parameterIndex(0), declarationBlockIndex(0)
{
    globalNs.SetSymbolTable(this);
}

void SymbolTable::Write(SymbolWriter& writer)
{
    globalNs.Write(writer);
    std::vector<ArrayTypeSymbol*> exportedArrayTypes;
    for (const std::unique_ptr<ArrayTypeSymbol>& arrayType : arrayTypes)
    {
        if (arrayType->IsProject())
        {
            exportedArrayTypes.push_back(arrayType.get());
        }
    }
    uint32_t na = exportedArrayTypes.size();
    writer.GetBinaryWriter().WriteEncodedUInt(na);
    for (ArrayTypeSymbol* exportedArrayType : exportedArrayTypes)
    {
        writer.Write(exportedArrayType);
    }
    globalNs.ComputeExportClosure();
    std::vector<TypeSymbol*> exportedDerivedTypes;
    for (const auto& derivedType : derivedTypes)
    {
        if (derivedType->MarkedExport())
        {
            exportedDerivedTypes.push_back(derivedType.get());
        }
    }
    uint32_t ned = exportedDerivedTypes.size();
    writer.GetBinaryWriter().WriteEncodedUInt(ned);
    for (TypeSymbol* exportedDerivedType : exportedDerivedTypes)
    {
        writer.Write(exportedDerivedType);
    }
    std::vector<TypeSymbol*> exportedClassTemplateSpecializations;
    for (const auto& classTemplateSpecialization : classTemplateSpecializations)
    {
        if (classTemplateSpecialization->MarkedExport())
        {
            exportedClassTemplateSpecializations.push_back(classTemplateSpecialization.get());
        }
    }
    uint32_t nec = exportedClassTemplateSpecializations.size();
    writer.GetBinaryWriter().WriteEncodedUInt(nec);
    for (TypeSymbol* classTemplateSpecialization : exportedClassTemplateSpecializations)
    {
        writer.Write(classTemplateSpecialization);
    }
    uint32_t nj = jsonClasses.size();
    writer.GetBinaryWriter().WriteEncodedUInt(nj);
    for (const std::u32string& jsonClass : jsonClasses)
    {
        writer.GetBinaryWriter().Write(jsonClass);
    }
}

void SymbolTable::Read(SymbolReader& reader)
{
    reader.SetSymbolTable(this);
    globalNs.Read(reader);
    uint32_t na = reader.GetBinaryReader().ReadEncodedUInt();
    for (uint32_t i = 0; i < na; ++i)
    {
        ArrayTypeSymbol* arrayTypeSymbol = reader.ReadArrayTypeSymbol(&globalNs);
        arrayTypes.push_back(std::unique_ptr<ArrayTypeSymbol>(arrayTypeSymbol));
    }
    uint32_t nd = reader.GetBinaryReader().ReadEncodedUInt();
    for (uint32_t i = 0; i < nd; ++i)
    {
        DerivedTypeSymbol* derivedTypeSymbol = reader.ReadDerivedTypeSymbol(&globalNs);
        derivedTypes.push_back(std::unique_ptr<DerivedTypeSymbol>(derivedTypeSymbol));
    }
    uint32_t nc = reader.GetBinaryReader().ReadEncodedUInt();
    for (uint32_t i = 0; i < nc; ++i)
    {
        ClassTemplateSpecializationSymbol* classTemplateSpecialization = reader.ReadClassTemplateSpecializationSymbol(&globalNs);
        classTemplateSpecializations.push_back(std::unique_ptr<ClassTemplateSpecializationSymbol>(classTemplateSpecialization));
        reader.AddClassTemplateSpecialization(classTemplateSpecialization);
    }
    uint32_t nj = reader.GetBinaryReader().ReadEncodedUInt();
    for (uint32_t i = 0; i < nj; ++i)
    {
        std::u32string jsonClass = reader.GetBinaryReader().ReadUtf32String();
        jsonClasses.insert(jsonClass);
    }
    ProcessTypeConceptAndFunctionRequests();
    for (FunctionSymbol* conversion : reader.Conversions())
    {
        AddConversion(conversion);
    }
}

void SymbolTable::Import(SymbolTable& symbolTable)
{
    globalNs.Import(&symbolTable.globalNs, *this);
    for (const auto& pair : symbolTable.typeIdMap)
    {
        Symbol* typeOrConcept = pair.second;
        if (typeOrConcept->IsTypeSymbol())
        {
            TypeSymbol* type = static_cast<TypeSymbol*>(typeOrConcept);
            typeIdMap[type->TypeId()] = type;
            typeNameMap[type->FullName()] = type;
        }
        else if (typeOrConcept->GetSymbolType() == SymbolType::conceptSymbol)
        {
            ConceptSymbol* concept = static_cast<ConceptSymbol*>(typeOrConcept);
            typeIdMap[concept->TypeId()] = concept;
        }
        else
        {
            Assert(false, "type or concept symbol expected");
        }
    }
    for (const auto& pair : symbolTable.functionIdMap)
    {
        FunctionSymbol* function = pair.second;
        functionIdMap[function->FunctionId()] = function;
    }
    for (auto& arrayType : symbolTable.arrayTypes)
    {
        arrayType->SetSymbolTable(this);
        arrayType->SetParent(&globalNs);
        ArrayTypeSymbol* releasedArrayType = arrayType.release();
        arrayTypes.push_back(std::unique_ptr<ArrayTypeSymbol>(releasedArrayType));
        ArrayKey key(releasedArrayType->ElementType(), releasedArrayType->Size());
        arrayTypeMap[key] = releasedArrayType;
    }
    for (auto& derivedType : symbolTable.derivedTypes)
    {
        derivedType->SetSymbolTable(this);
        derivedType->SetParent(&globalNs);
        derivedTypes.push_back(std::unique_ptr<DerivedTypeSymbol>(derivedType.release()));
    }
    int nd = derivedTypes.size();
    for (int i = 0; i < nd; ++i)
    {
        DerivedTypeSymbol* derivedTypeSymbol = derivedTypes[i].get();
        std::vector<DerivedTypeSymbol*>& derivedTypeVec = derivedTypeMap[derivedTypeSymbol->BaseType()];
        int n = derivedTypeVec.size();
        bool found = false;
        for (int i = 0; i < n; ++i)
        {
            DerivedTypeSymbol* prevDerivedTypeSymbol = derivedTypeVec[i];
            if (prevDerivedTypeSymbol->DerivationRec() == derivedTypeSymbol->DerivationRec())
            {
                found = true;
                break;
            }
        }
        if (!found)
        {
            derivedTypeVec.push_back(derivedTypeSymbol);
        }
    }
    for (auto& s : symbolTable.classTemplateSpecializations)
    {
        classTemplateSpecializations.push_back(std::move(s));
    }
    int nc = classTemplateSpecializations.size();
    for (int i = 0; i < nc; ++i)
    {
        ClassTemplateSpecializationSymbol* classTemplateSpecialization = classTemplateSpecializations[i].get();
        classTemplateSpecialization->SetSymbolTable(this);
        classTemplateSpecialization->SetParent(&globalNs);
        ClassTemplateSpecializationKey key(classTemplateSpecialization->GetClassTemplate(), classTemplateSpecialization->TemplateArgumentTypes());
        auto it = classTemplateSpecializationMap.find(key);
        if (it == classTemplateSpecializationMap.cend())
        {
            classTemplateSpecializationMap[key] = classTemplateSpecialization;
        }
    }
    conversionTable.Add(symbolTable.conversionTable);
    for (ClassTypeSymbol* polymorphicClass : symbolTable.PolymorphicClasses())
    {
        AddPolymorphicClass(polymorphicClass);
    }
    for (ClassTypeSymbol* classHavingStaticConstructor : symbolTable.ClassesHavingStaticConstructor())
    {
        AddClassHavingStaticConstructor(classHavingStaticConstructor);
    }
    for (const std::u32string& jsonClass : symbolTable.JsonClasses())
    {
        AddJsonClass(jsonClass);
    }
    symbolTable.Clear();
}

void SymbolTable::Clear()
{
    globalNs.Clear();
    typeIdMap.clear();
    functionIdMap.clear();
    typeNameMap.clear();
}

void SymbolTable::BeginContainer(ContainerSymbol* container_)
{
    containerStack.push(container);
    container = container_;
}

void SymbolTable::EndContainer()
{
    container = containerStack.top();
    containerStack.pop();
}

void SymbolTable::BeginNamespace(NamespaceNode& namespaceNode)
{
    std::u32string nsName = namespaceNode.Id()->Str();
    BeginNamespace(nsName, namespaceNode.GetSpan());
    MapNode(&namespaceNode, container);
}

void SymbolTable::BeginNamespace(const std::u32string& namespaceName, const Span& span)
{
    if (namespaceName.empty())
    {
        if (!globalNs.GetSpan().Valid())
        {
            globalNs.SetSpan(span);
        }
        BeginContainer(&globalNs);
    }
    else
    { 
        Symbol* symbol = container->GetContainerScope()->Lookup(namespaceName);
        if (symbol)
        {
            if (symbol->GetSymbolType() == SymbolType::namespaceSymbol)
            {
                NamespaceSymbol* ns = static_cast<NamespaceSymbol*>(symbol);
                BeginContainer(ns);
            }
            else
            {
                throw Exception("symbol '" + ToUtf8(symbol->Name()) + "' does not denote a namespace", symbol->GetSpan());
            }
        }
        else
        {
            NamespaceSymbol* ns = container->GetContainerScope()->CreateNamespace(namespaceName, span);
            BeginContainer(ns);
        }
    }
}

void SymbolTable::EndNamespace()
{
    EndContainer();
}

void SymbolTable::BeginFunction(FunctionNode& functionNode)
{
    FunctionSymbol* functionSymbol = new FunctionSymbol(functionNode.GetSpan(), functionNode.GroupId());
    SetFunctionIdFor(functionSymbol);
    if ((functionNode.GetSpecifiers() & Specifiers::constexpr_) != Specifiers::none)
    {
        functionSymbol->SetConstExpr();
    }
    functionSymbol->SetCompileUnit(currentCompileUnit);
    functionSymbol->SetSymbolTable(this);
    functionSymbol->SetGroupName(functionNode.GroupId());
    if (functionNode.WhereConstraint())
    {
        CloneContext cloneContext;
        functionSymbol->SetConstraint(static_cast<WhereConstraintNode*>(functionNode.WhereConstraint()->Clone(cloneContext)));
    }
    if (functionSymbol->GroupName() == U"main")
    {
        if (functionNode.IsProgramMain())
        {
            functionSymbol->SetCDecl();
            functionSymbol->SetProgramMain();
        }
        else
        {
            if (mainFunctionSymbol)
            {
                throw Exception("already has main function", functionNode.GetSpan(), mainFunctionSymbol->GetSpan());
            }
            else
            {
                mainFunctionSymbol = functionSymbol;
            }
        }
    }
    MapNode(&functionNode, functionSymbol);
    ContainerScope* functionScope = functionSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    functionScope->SetParent(containerScope);
    BeginContainer(functionSymbol);
    parameterIndex = 0;
    ResetDeclarationBlockIndex();
}

void SymbolTable::EndFunction()
{
    FunctionSymbol* functionSymbol = static_cast<FunctionSymbol*>(container);
    EndContainer();
    container->AddMember(functionSymbol);
}

void SymbolTable::AddParameter(ParameterNode& parameterNode)
{
    std::u32string& parameterName = ToUtf32("@p" + std::to_string(parameterIndex));
    if (parameterNode.Id())
    {
        parameterName = parameterNode.Id()->Str();
    }
    else
    {
        parameterNode.SetId(new IdentifierNode(parameterNode.GetSpan(), parameterName));
    }
    ParameterSymbol* parameterSymbol = new ParameterSymbol(parameterNode.GetSpan(), parameterName);
    parameterSymbol->SetCompileUnit(currentCompileUnit);
    parameterSymbol->SetSymbolTable(this);
    MapNode(&parameterNode, parameterSymbol);
    container->AddMember(parameterSymbol);
    ++parameterIndex;
}

void SymbolTable::BeginClass(ClassNode& classNode)
{
    ClassTypeSymbol* classTypeSymbol = new ClassTypeSymbol(classNode.GetSpan(), classNode.Id()->Str());
    classTypeSymbol->SetGroupName(classNode.Id()->Str());
    currentClassStack.push(currentClass);
    currentClass = classTypeSymbol;
    classTypeSymbol->SetCompileUnit(currentCompileUnit);
    classTypeSymbol->SetSymbolTable(this);
    MapNode(&classNode, classTypeSymbol);
    SetTypeIdFor(classTypeSymbol);
    ContainerScope* classScope = classTypeSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    classScope->SetParent(containerScope);
    BeginContainer(classTypeSymbol);
}

void SymbolTable::EndClass()
{
    ClassTypeSymbol* classTypeSymbol = currentClass;
    currentClass = currentClassStack.top();
    currentClassStack.pop();
    EndContainer();
    classTypeSymbol->ComputeMinArity();
    container->AddMember(classTypeSymbol);
}

void SymbolTable::BeginClassTemplateSpecialization(ClassNode& classInstanceNode, ClassTemplateSpecializationSymbol* classTemplateSpecialization)
{
    currentClassStack.push(currentClass);
    currentClass = classTemplateSpecialization;
    MapNode(&classInstanceNode, classTemplateSpecialization);
    if (classTemplateSpecialization->TypeIdNotSet())
    {
        SetTypeIdFor(classTemplateSpecialization);
    }
    ContainerScope* containerScope = container->GetContainerScope();
    ContainerScope* classScope = classTemplateSpecialization->GetContainerScope();
    classScope->SetParent(containerScope);
    BeginContainer(classTemplateSpecialization);
}

void SymbolTable::EndClassTemplateSpecialization()
{
    EndContainer();
    currentClass = currentClassStack.top();
    currentClassStack.pop();
}

void SymbolTable::AddTemplateParameter(TemplateParameterNode& templateParameterNode)
{
    TemplateParameterSymbol* templateParameterSymbol = new TemplateParameterSymbol(templateParameterNode.GetSpan(), templateParameterNode.Id()->Str());
    if (templateParameterNode.DefaultTemplateArgument())
    {
        templateParameterSymbol->SetHasDefault();
        templateParameterSymbol->SetDefaultStr(templateParameterNode.DefaultTemplateArgument()->ToString());
    }
    templateParameterSymbol->SetCompileUnit(currentCompileUnit);
    templateParameterSymbol->SetSymbolTable(this);
    SetTypeIdFor(templateParameterSymbol);
    MapNode(&templateParameterNode, templateParameterSymbol);
    container->AddMember(templateParameterSymbol);
}

void SymbolTable::AddTemplateParameter(IdentifierNode& identifierNode)
{
    TemplateParameterSymbol* templateParameterSymbol = new TemplateParameterSymbol(identifierNode.GetSpan(), identifierNode.Str());
    templateParameterSymbol->SetCompileUnit(currentCompileUnit);
    templateParameterSymbol->SetSymbolTable(this);
    SetTypeIdFor(templateParameterSymbol);
    MapNode(&identifierNode, templateParameterSymbol);
    container->AddMember(templateParameterSymbol);
}

void SymbolTable::BeginInterface(InterfaceNode& interfaceNode)
{
    InterfaceTypeSymbol* interfaceTypeSymbol = new InterfaceTypeSymbol(interfaceNode.GetSpan(), interfaceNode.Id()->Str());
    currentInterfaceStack.push(currentInterface);
    currentInterface = interfaceTypeSymbol;
    interfaceTypeSymbol->SetCompileUnit(currentCompileUnit);
    interfaceTypeSymbol->SetSymbolTable(this);
    MapNode(&interfaceNode, interfaceTypeSymbol);
    SetTypeIdFor(interfaceTypeSymbol);
    ContainerScope* interfaceScope = interfaceTypeSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    interfaceScope->SetParent(containerScope);
    container->AddMember(interfaceTypeSymbol);
    BeginContainer(interfaceTypeSymbol);
}

void SymbolTable::EndInterface()
{
    currentInterface = currentInterfaceStack.top();
    currentInterfaceStack.pop();
    EndContainer();
}

void SymbolTable::BeginStaticConstructor(StaticConstructorNode& staticConstructorNode)
{
    StaticConstructorSymbol* staticConstructorSymbol = new StaticConstructorSymbol(staticConstructorNode.GetSpan(), U"@static_constructor");
    SetFunctionIdFor(staticConstructorSymbol);
    staticConstructorSymbol->SetCompileUnit(currentCompileUnit);
    staticConstructorSymbol->SetSymbolTable(this);
    if (staticConstructorNode.WhereConstraint())
    {
        CloneContext cloneContext;
        staticConstructorSymbol->SetConstraint(static_cast<WhereConstraintNode*>(staticConstructorNode.WhereConstraint()->Clone(cloneContext)));
    }
    MapNode(&staticConstructorNode, staticConstructorSymbol);
    ContainerScope* staticConstructorScope = staticConstructorSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    staticConstructorScope->SetParent(containerScope);
    BeginContainer(staticConstructorSymbol);
    ResetDeclarationBlockIndex();
}

void SymbolTable::EndStaticConstructor()
{
    StaticConstructorSymbol* staticConstructorSymbol = static_cast<StaticConstructorSymbol*>(container);
    EndContainer();
    container->AddMember(staticConstructorSymbol);
}

void SymbolTable::BeginConstructor(ConstructorNode& constructorNode)
{
    ConstructorSymbol* constructorSymbol = new ConstructorSymbol(constructorNode.GetSpan(), U"@constructor");
    SetFunctionIdFor(constructorSymbol);
    if ((constructorNode.GetSpecifiers() & Specifiers::constexpr_) != Specifiers::none)
    {
        constructorSymbol->SetConstExpr();
    }
    constructorSymbol->SetCompileUnit(currentCompileUnit);
    constructorSymbol->SetSymbolTable(this);
    if (constructorNode.WhereConstraint())
    {
        CloneContext cloneContext;
        constructorSymbol->SetConstraint(static_cast<WhereConstraintNode*>(constructorNode.WhereConstraint()->Clone(cloneContext)));
    }
    MapNode(&constructorNode, constructorSymbol);
    ContainerScope* constructorScope = constructorSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    constructorScope->SetParent(containerScope);
    BeginContainer(constructorSymbol);
    parameterIndex = 0;
    ResetDeclarationBlockIndex();
    ParameterSymbol* thisParam = new ParameterSymbol(constructorNode.GetSpan(), U"this");
    thisParam->SetSymbolTable(this);
    TypeSymbol* thisParamType = nullptr;
    if (currentClass)
    {
        thisParamType = currentClass->AddPointer(constructorNode.GetSpan());
        thisParam->SetType(thisParamType);
        thisParam->SetBound();
        constructorSymbol->AddMember(thisParam);
    }
    else if (currentInterface)
    {
        throw Exception("interface type cannot have a constructor", constructorNode.GetSpan());
    }
}

void SymbolTable::EndConstructor()
{
    ConstructorSymbol* constructorSymbol = static_cast<ConstructorSymbol*>(container);
    EndContainer();
    container->AddMember(constructorSymbol);
}

void SymbolTable::BeginDestructor(DestructorNode& destructorNode)
{
    DestructorSymbol* destructorSymbol = new DestructorSymbol(destructorNode.GetSpan(), U"@destructor");
    SetFunctionIdFor(destructorSymbol);
    destructorSymbol->SetCompileUnit(currentCompileUnit);
    destructorSymbol->SetSymbolTable(this);
    if (destructorNode.WhereConstraint())
    {
        CloneContext cloneContext;
        destructorSymbol->SetConstraint(static_cast<WhereConstraintNode*>(destructorNode.WhereConstraint()->Clone(cloneContext)));
    }
    MapNode(&destructorNode, destructorSymbol);
    ContainerScope* destructorScope = destructorSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    destructorScope->SetParent(containerScope);
    BeginContainer(destructorSymbol);
    ResetDeclarationBlockIndex();
    ParameterSymbol* thisParam = new ParameterSymbol(destructorNode.GetSpan(), U"this");
    thisParam->SetSymbolTable(this);
    TypeSymbol* thisParamType = nullptr;
    if (currentClass)
    {
        thisParamType = currentClass->AddPointer(destructorNode.GetSpan());
        thisParam->SetType(thisParamType);
        thisParam->SetBound();
        destructorSymbol->AddMember(thisParam);
    }
    else if (currentInterface)
    {
        throw Exception("interface type cannot have a destructor", destructorNode.GetSpan());
    }
}

void SymbolTable::EndDestructor()
{
    DestructorSymbol* destructorSymbol = static_cast<DestructorSymbol*>(container);
    EndContainer();
    container->AddMember(destructorSymbol);
}

void SymbolTable::BeginMemberFunction(MemberFunctionNode& memberFunctionNode)
{
    MemberFunctionSymbol* memberFunctionSymbol = new MemberFunctionSymbol(memberFunctionNode.GetSpan(), memberFunctionNode.GroupId());
    SetFunctionIdFor(memberFunctionSymbol);
    if ((memberFunctionNode.GetSpecifiers() & Specifiers::constexpr_) != Specifiers::none)
    {
        memberFunctionSymbol->SetConstExpr();
    }
    memberFunctionSymbol->SetCompileUnit(currentCompileUnit);
    memberFunctionSymbol->SetSymbolTable(this);
    memberFunctionSymbol->SetGroupName(memberFunctionNode.GroupId());
    if (memberFunctionNode.WhereConstraint())
    {
        CloneContext cloneContext;
        memberFunctionSymbol->SetConstraint(static_cast<WhereConstraintNode*>(memberFunctionNode.WhereConstraint()->Clone(cloneContext)));
    }
    MapNode(&memberFunctionNode, memberFunctionSymbol);
    ContainerScope* memberFunctionScope = memberFunctionSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    memberFunctionScope->SetParent(containerScope);
    BeginContainer(memberFunctionSymbol);
    parameterIndex = 0;
    ResetDeclarationBlockIndex();
    if ((memberFunctionNode.GetSpecifiers() & Specifiers::static_) == Specifiers::none)
    {
        ParameterSymbol* thisParam = new ParameterSymbol(memberFunctionNode.GetSpan(), U"this");
        thisParam->SetSymbolTable(this);
        TypeSymbol* thisParamType = nullptr;
        if (currentClass)
        {
            if (memberFunctionNode.IsConst())
            {
                thisParamType = currentClass->AddConst(memberFunctionNode.GetSpan())->AddPointer(memberFunctionNode.GetSpan());
            }
            else
            {
                thisParamType = currentClass->AddPointer(memberFunctionNode.GetSpan());
            }
        }
        else if (currentInterface)
        {
            thisParamType = currentInterface->AddPointer(memberFunctionNode.GetSpan());
        }
        else
        {
            Assert(false, "class or interface expected");
        }
        thisParam->SetType(thisParamType);
        thisParam->SetBound();
        memberFunctionSymbol->AddMember(thisParam);
    }
}

void SymbolTable::EndMemberFunction()
{
    MemberFunctionSymbol* memberFunctionSymbol = static_cast<MemberFunctionSymbol*>(container);
    EndContainer();
    container->AddMember(memberFunctionSymbol);
}

void SymbolTable::BeginConversionFunction(ConversionFunctionNode& conversionFunctionNode)
{
    ConversionFunctionSymbol* conversionFunctionSymbol = new ConversionFunctionSymbol(conversionFunctionNode.GetSpan(), U"@conversion");
    SetFunctionIdFor(conversionFunctionSymbol);
    if ((conversionFunctionNode.GetSpecifiers() & Specifiers::constexpr_) != Specifiers::none)
    {
        conversionFunctionSymbol->SetConstExpr();
    }
    conversionFunctionSymbol->SetCompileUnit(currentCompileUnit);
    conversionFunctionSymbol->SetSymbolTable(this);
    conversionFunctionSymbol->SetGroupName(U"@operator_conv");
    if (conversionFunctionNode.WhereConstraint())
    {
        CloneContext cloneContext;
        conversionFunctionSymbol->SetConstraint(static_cast<WhereConstraintNode*>(conversionFunctionNode.WhereConstraint()->Clone(cloneContext)));
    }
    MapNode(&conversionFunctionNode, conversionFunctionSymbol);
    ContainerScope* conversionFunctionScope = conversionFunctionSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    conversionFunctionScope->SetParent(containerScope);
    BeginContainer(conversionFunctionSymbol);
    ResetDeclarationBlockIndex();
    ParameterSymbol* thisParam = new ParameterSymbol(conversionFunctionNode.GetSpan(), U"this");
    thisParam->SetSymbolTable(this);
    TypeSymbol* thisParamType = nullptr;
    if (conversionFunctionNode.IsConst())
    {
        thisParamType = currentClass->AddConst(conversionFunctionNode.GetSpan())->AddPointer(conversionFunctionNode.GetSpan());
    }
    else
    {
        thisParamType = currentClass->AddPointer(conversionFunctionNode.GetSpan());
    }
    thisParam->SetType(thisParamType);
    thisParam->SetBound();
    conversionFunctionSymbol->AddMember(thisParam);
}

void SymbolTable::EndConversionFunction()
{
    ConversionFunctionSymbol* conversionFunctionSymbol = static_cast<ConversionFunctionSymbol*>(container);
    EndContainer();
    container->AddMember(conversionFunctionSymbol);
}

void SymbolTable::AddMemberVariable(MemberVariableNode& memberVariableNode)
{
    MemberVariableSymbol* memberVariableSymbol = new MemberVariableSymbol(memberVariableNode.GetSpan(), memberVariableNode.Id()->Str());
    if ((memberVariableNode.GetSpecifiers() & Specifiers::static_) != Specifiers::none)
    {
        memberVariableSymbol->SetStatic();
    }
    memberVariableSymbol->SetCompileUnit(currentCompileUnit);
    memberVariableSymbol->SetSymbolTable(this);
    MapNode(&memberVariableNode, memberVariableSymbol);
    container->AddMember(memberVariableSymbol);
}

void SymbolTable::BeginDelegate(DelegateNode& delegateNode)
{
    DelegateTypeSymbol* delegateTypeSymbol = new DelegateTypeSymbol(delegateNode.GetSpan(), delegateNode.Id()->Str());
    delegateTypeSymbol->SetCompileUnit(currentCompileUnit);
    delegateTypeSymbol->SetSymbolTable(this);
    MapNode(&delegateNode, delegateTypeSymbol);
    SetTypeIdFor(delegateTypeSymbol);
    ContainerScope* delegateScope = delegateTypeSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    delegateScope->SetParent(containerScope);
    container->AddMember(delegateTypeSymbol);
    BeginContainer(delegateTypeSymbol);
    parameterIndex = 0;
}

void SymbolTable::EndDelegate()
{
    EndContainer();
}

void SymbolTable::BeginClassDelegate(ClassDelegateNode& classDelegateNode)
{
    ClassDelegateTypeSymbol* classDelegateTypeSymbol = new ClassDelegateTypeSymbol(classDelegateNode.GetSpan(), classDelegateNode.Id()->Str());
    classDelegateTypeSymbol->SetCompileUnit(currentCompileUnit);
    classDelegateTypeSymbol->SetSymbolTable(this);
    MapNode(&classDelegateNode, classDelegateTypeSymbol);
    SetTypeIdFor(classDelegateTypeSymbol);
    ContainerScope* classDelegateScope = classDelegateTypeSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    classDelegateScope->SetParent(containerScope);
    container->AddMember(classDelegateTypeSymbol);
    BeginContainer(classDelegateTypeSymbol);
    parameterIndex = 0;
}

void SymbolTable::EndClassDelegate()
{
    EndContainer();
}

void SymbolTable::BeginConcept(ConceptNode& conceptNode)
{
    ConceptSymbol* conceptSymbol = new ConceptSymbol(conceptNode.GetSpan(), conceptNode.Id()->Str());
    conceptSymbol->SetGroupName(conceptNode.Id()->Str());
    conceptSymbol->SetCompileUnit(currentCompileUnit);
    conceptSymbol->SetSymbolTable(this);
    MapNode(&conceptNode, conceptSymbol);
    SetTypeIdFor(conceptSymbol);
    ContainerScope* conceptScope = conceptSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    conceptScope->SetParent(containerScope);
    BeginContainer(conceptSymbol);
}

void SymbolTable::EndConcept()
{
    ConceptSymbol* conceptSymbol = static_cast<ConceptSymbol*>(container);
    EndContainer();
    container->AddMember(conceptSymbol);
}

void SymbolTable::BeginDeclarationBlock(Node& node)
{
    DeclarationBlock* declarationBlock = new DeclarationBlock(node.GetSpan(), U"@locals" + ToUtf32(std::to_string(GetNextDeclarationBlockIndex())));
    declarationBlock->SetCompileUnit(currentCompileUnit);
    declarationBlock->SetSymbolTable(this);
    MapNode(&node, declarationBlock);
    ContainerScope* declarationBlockScope = declarationBlock->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    declarationBlockScope->SetParent(containerScope);
    container->AddMember(declarationBlock);
    BeginContainer(declarationBlock);
}

void SymbolTable::EndDeclarationBlock()
{
    EndContainer();
}

void SymbolTable::AddLocalVariable(ConstructionStatementNode& constructionStatementNode)
{
    LocalVariableSymbol* localVariableSymbol = new LocalVariableSymbol(constructionStatementNode.GetSpan(), constructionStatementNode.Id()->Str());
    localVariableSymbol->SetCompileUnit(currentCompileUnit);
    localVariableSymbol->SetSymbolTable(this);
    MapNode(&constructionStatementNode, localVariableSymbol);
    container->AddMember(localVariableSymbol);
}

void SymbolTable::AddLocalVariable(IdentifierNode& identifierNode)
{
    LocalVariableSymbol* localVariableSymbol = new LocalVariableSymbol(identifierNode.GetSpan(), identifierNode.Str());
    localVariableSymbol->SetCompileUnit(currentCompileUnit);
    localVariableSymbol->SetSymbolTable(this);
    MapNode(&identifierNode, localVariableSymbol);
    container->AddMember(localVariableSymbol);
}

void SymbolTable::AddTypedef(TypedefNode& typedefNode)
{
    TypedefSymbol* typedefSymbol = new TypedefSymbol(typedefNode.GetSpan(), typedefNode.Id()->Str());
    typedefSymbol->SetCompileUnit(currentCompileUnit);
    typedefSymbol->SetSymbolTable(this);
    MapNode(&typedefNode, typedefSymbol);
    container->AddMember(typedefSymbol);
}

void SymbolTable::AddConstant(ConstantNode& constantNode)
{
    ConstantSymbol* constantSymbol = new ConstantSymbol(constantNode.GetSpan(), constantNode.Id()->Str());
    constantSymbol->SetCompileUnit(currentCompileUnit);
    constantSymbol->SetSymbolTable(this);
    MapNode(&constantNode, constantSymbol);
    container->AddMember(constantSymbol);
}

void SymbolTable::BeginEnumType(EnumTypeNode& enumTypeNode)
{
    EnumTypeSymbol* enumTypeSymbol = new EnumTypeSymbol(enumTypeNode.GetSpan(), enumTypeNode.Id()->Str());
    enumTypeSymbol->SetCompileUnit(currentCompileUnit);
    enumTypeSymbol->SetSymbolTable(this);
    MapNode(&enumTypeNode, enumTypeSymbol);
    SetTypeIdFor(enumTypeSymbol);
    ContainerScope* enumTypeScope = enumTypeSymbol->GetContainerScope();
    ContainerScope* containerScope = container->GetContainerScope();
    enumTypeScope->SetParent(containerScope);
    container->AddMember(enumTypeSymbol);
    BeginContainer(enumTypeSymbol);
}

void SymbolTable::EndEnumType()
{
    EndContainer();
}

void SymbolTable::AddEnumConstant(EnumConstantNode& enumConstantNode)
{
    EnumConstantSymbol* enumConstantSymbol = new EnumConstantSymbol(enumConstantNode.GetSpan(), enumConstantNode.Id()->Str());
    enumConstantSymbol->SetCompileUnit(currentCompileUnit);
    enumConstantSymbol->SetSymbolTable(this);
    MapNode(&enumConstantNode, enumConstantSymbol);
    container->AddMember(enumConstantSymbol);
}

void SymbolTable::AddTypeSymbolToGlobalScope(TypeSymbol* typeSymbol)
{
    typeSymbol->SetSymbolTable(this);
    globalNs.AddMember(typeSymbol);
    SetTypeIdFor(typeSymbol);
    typeNameMap[typeSymbol->FullName()] = typeSymbol;
}

void SymbolTable::AddFunctionSymbolToGlobalScope(FunctionSymbol* functionSymbol)
{
    SetFunctionIdFor(functionSymbol);
    functionSymbol->SetSymbolTable(this);
    globalNs.AddMember(functionSymbol);
    if (functionSymbol->IsConversion())
    {
        conversionTable.AddConversion(functionSymbol);
    }
}

void SymbolTable::MapNode(Node* node, Symbol* symbol)
{
    nodeSymbolMap[node] = symbol;
    symbolNodeMap[symbol] = node;
}

Symbol* SymbolTable::GetSymbolNoThrow(Node* node) const
{
    auto it = nodeSymbolMap.find(node);
    if (it != nodeSymbolMap.cend())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

Symbol* SymbolTable::GetSymbol(Node* node) const
{
    Symbol* symbol = GetSymbolNoThrow(node);
    if (symbol)
    {
        return symbol;
    }
    else
    {
        throw std::runtime_error("symbol for node not found");
    }
}

Node* SymbolTable::GetNodeNoThrow(Symbol* symbol) const
{
    auto it = symbolNodeMap.find(symbol);
    if (it != symbolNodeMap.cend())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

Node* SymbolTable::GetNode(Symbol* symbol) const
{
    Node* node = GetNodeNoThrow(symbol);
    if (node)
    {
        return node;
    }
    else
    {
        throw std::runtime_error("node for symbol not found");
    }
}

void SymbolTable::AddTypeOrConceptSymbolToTypeIdMap(Symbol* typeOrConceptSymbol)
{
    if (typeOrConceptSymbol->IsTypeSymbol())
    {
        TypeSymbol* typeSymbol = static_cast<TypeSymbol*>(typeOrConceptSymbol);
        typeIdMap[typeSymbol->TypeId()] = typeSymbol;
    }
    else if (typeOrConceptSymbol->GetSymbolType() == SymbolType::conceptSymbol)
    {
        ConceptSymbol* conceptSymbol = static_cast<ConceptSymbol*>(typeOrConceptSymbol);
        typeIdMap[conceptSymbol->TypeId()] = conceptSymbol;
    }
    else
    {
        Assert(false, "type or concept symbol expected");
    }
}

void SymbolTable::AddFunctionSymbolToFunctionIdMap(FunctionSymbol* functionSymbol)
{
    functionIdMap[functionSymbol->FunctionId()] = functionSymbol;
}

void SymbolTable::SetTypeIdFor(TypeSymbol* typeSymbol)
{
    typeSymbol->SetTypeId(TypeIdCounter::Instance().GetNextTypeId());
}

void SymbolTable::SetTypeIdFor(ConceptSymbol* conceptSymbol)
{
    conceptSymbol->SetTypeId(TypeIdCounter::Instance().GetNextTypeId());
}

void SymbolTable::SetFunctionIdFor(FunctionSymbol* functionSymbol)
{
    functionSymbol->SetFunctionId(FunctionIdCounter::Instance().GetNextFunctionId());
}

void SymbolTable::EmplaceTypeRequest(Symbol* forSymbol, uint32_t typeId, int index)
{
    EmplaceTypeOrConceptRequest(forSymbol, typeId, index);
}

const int conceptRequestIndex = std::numeric_limits<int>::max();

void SymbolTable::EmplaceConceptRequest(Symbol* forSymbol, uint32_t typeId)
{
    EmplaceTypeOrConceptRequest(forSymbol, typeId, conceptRequestIndex);
}

void SymbolTable::EmplaceTypeOrConceptRequest(Symbol* forSymbol, uint32_t typeId, int index)
{
    auto it = typeIdMap.find(typeId);
    if (it != typeIdMap.cend())
    {
        Symbol* typeOrConceptSymbol = it->second;
        if (typeOrConceptSymbol->IsTypeSymbol())
        {
            if (index == conceptRequestIndex)
            {
                throw Exception("internal error: invalid concept request (id denotes a type)", forSymbol->GetSpan());
            }
            TypeSymbol* typeSymbol = static_cast<TypeSymbol*>(typeOrConceptSymbol);
            forSymbol->EmplaceType(typeSymbol, index);
        }
        else if (typeOrConceptSymbol->GetSymbolType() == SymbolType::conceptSymbol)
        {
            if (index != conceptRequestIndex)
            {
                throw Exception("internal error: invalid type request (id denotes a concept)", forSymbol->GetSpan());
            }
            ConceptSymbol* conceptSymbol = static_cast<ConceptSymbol*>(typeOrConceptSymbol);
            forSymbol->EmplaceConcept(conceptSymbol);
        }
        else
        {
            Assert(false, "internal error: type or concept symbol expected");
        }
    }
    else
    {
        typeAndConceptRequests.push_back(TypeOrConceptRequest(forSymbol, typeId, index));
    }
}

void SymbolTable::EmplaceFunctionRequest(Symbol* forSymbol, uint32_t functionId, int index)
{
    auto it = functionIdMap.find(functionId);
    if (it != functionIdMap.cend())
    {
        FunctionSymbol* functionSymbol = it->second;
        forSymbol->EmplaceFunction(functionSymbol, index);
    }
    else
    {
        functionRequests.push_back(FunctionRequest(forSymbol, functionId, index));
    }
}

void SymbolTable::ProcessTypeConceptAndFunctionRequests()
{
    for (const TypeOrConceptRequest& typeOrConceptRequest : typeAndConceptRequests)
    {
        Symbol* symbol = typeOrConceptRequest.symbol;
        auto it = typeIdMap.find(typeOrConceptRequest.typeId);
        if (it != typeIdMap.cend())
        {
            Symbol* typeOrConceptSymbol = it->second;
            int index = typeOrConceptRequest.index;
            if (typeOrConceptSymbol->IsTypeSymbol())
            {
                if (index == conceptRequestIndex)
                {
                    throw Exception("internal error: invalid concept request (id denotes a type)", symbol->GetSpan());
                }
                TypeSymbol* typeSymbol = static_cast<TypeSymbol*>(typeOrConceptSymbol);
                symbol->EmplaceType(typeSymbol, index);
            }
            else if (typeOrConceptSymbol->GetSymbolType() == SymbolType::conceptSymbol)
            {
                if (index != conceptRequestIndex)
                {
                    throw Exception("internal error: invalid type request (id denotes a concept)", symbol->GetSpan());
                }
                ConceptSymbol* conceptSymbol = static_cast<ConceptSymbol*>(typeOrConceptSymbol);
                symbol->EmplaceConcept(conceptSymbol);
            }
            else
            {
                Assert(false, "internal error: type or concept symbol expected");
            }
        }
        else
        {
            throw std::runtime_error("internal error: cannot satisfy type or concept request for symbol '" + ToUtf8(symbol->Name()) + "': type or concept not found from symbol table");
        }
    }
    typeAndConceptRequests.clear();
    for (const FunctionRequest& functionRequest : functionRequests)
    {
        Symbol* symbol = functionRequest.symbol;
        auto it = functionIdMap.find(functionRequest.functionId);
        if (it != functionIdMap.cend())
        {
            FunctionSymbol* functionSymbol = it->second;
            int index = functionRequest.index;
            symbol->EmplaceFunction(functionSymbol, index);
        }
        else
        {
            throw std::runtime_error("internal error: cannot satisfy function request for symbol '" + ToUtf8(symbol->Name()) + "': function not found from symbol table");
        }
    }
    functionRequests.clear();
}

TypeSymbol* SymbolTable::GetTypeByNameNoThrow(const std::u32string& typeName) const
{
    auto it = typeNameMap.find(typeName);
    if (it != typeNameMap.cend())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

TypeSymbol* SymbolTable::GetTypeByName(const std::u32string& typeName) const
{
    TypeSymbol* typeSymbol = GetTypeByNameNoThrow(typeName);
    if (typeSymbol)
    {
        return typeSymbol;
    }
    else
    {
        throw std::runtime_error("type '" + ToUtf8(typeName) + "' not found");
    }
}

TypeSymbol* SymbolTable::MakeDerivedType(TypeSymbol* baseType, const TypeDerivationRec& derivationRec, const Span& span)
{
    if (derivationRec.IsEmpty())
    {
        return baseType;
    }
    if (baseType->IsVoidType() && HasReferenceDerivation(derivationRec.derivations) && !HasPointerDerivation(derivationRec.derivations))
    {
        throw Exception("cannot have reference to void type", span);
    }
    std::vector<DerivedTypeSymbol*>& mappedDerivedTypes = derivedTypeMap[baseType];
    int n = mappedDerivedTypes.size();
    for (int i = 0; i < n; ++i)
    {
        DerivedTypeSymbol* derivedType = mappedDerivedTypes[i];
        if (derivedType->DerivationRec() == derivationRec)
        {
            return derivedType;
        }
    }
    DerivedTypeSymbol* derivedType = new DerivedTypeSymbol(baseType->GetSpan(), MakeDerivedTypeName(baseType, derivationRec), baseType, derivationRec);
    derivedType->SetParent(&globalNs);
    derivedType->SetSymbolTable(this);
    SetTypeIdFor(derivedType);
    mappedDerivedTypes.push_back(derivedType);
    derivedTypes.push_back(std::unique_ptr<DerivedTypeSymbol>(derivedType));
    if (derivedType->IsPointerType() && !derivedType->BaseType()->IsVoidType() && !derivedType->IsReferenceType())
    {
        TypedefSymbol* valueType = new TypedefSymbol(span, U"ValueType");
        valueType->SetAccess(SymbolAccess::public_);
        valueType->SetType(derivedType->RemovePointer(span));
        if (valueType->GetType()->RemoveConst(span)->IsBasicTypeSymbol())
        {
            valueType->SetType(valueType->GetType()->RemoveConst(span));
        }
        valueType->SetBound();
        valueType->GetType()->MarkExport();
        derivedType->AddMember(valueType);
        TypedefSymbol* referenceType = new TypedefSymbol(span, U"ReferenceType");
        referenceType->SetAccess(SymbolAccess::public_);
        referenceType->SetType(valueType->GetType()->AddLvalueReference(span));
        referenceType->SetBound();
        referenceType->GetType()->MarkExport();
        derivedType->AddMember(referenceType);
        TypedefSymbol* pointerType = new TypedefSymbol(span, U"PointerType");
        pointerType->SetAccess(SymbolAccess::public_);
        pointerType->SetType(derivedType);
        pointerType->SetBound();
        pointerType->GetType()->MarkExport();
        derivedType->AddMember(pointerType);
    }
    return derivedType;
}

ClassTemplateSpecializationSymbol* SymbolTable::MakeClassTemplateSpecialization(ClassTypeSymbol* classTemplate, const std::vector<TypeSymbol*>& templateArgumentTypes, const Span& span)
{
    ClassTemplateSpecializationKey key(classTemplate, templateArgumentTypes);
    auto it = classTemplateSpecializationMap.find(key);
    if (it != classTemplateSpecializationMap.cend())
    {
        ClassTemplateSpecializationSymbol* classTemplateSpecialization = it->second;
        return classTemplateSpecialization;
    }
    else
    {
        ClassTemplateSpecializationSymbol* classTemplateSpecialization = new ClassTemplateSpecializationSymbol(span,
            MakeClassTemplateSpecializationName(classTemplate, templateArgumentTypes), classTemplate, templateArgumentTypes);
        classTemplateSpecialization->SetGroupName(classTemplate->GroupName());
        classTemplateSpecializationMap[key] = classTemplateSpecialization;
        classTemplateSpecialization->SetParent(&globalNs);
        classTemplateSpecialization->SetSymbolTable(this);
        classTemplateSpecializations.push_back(std::unique_ptr<ClassTemplateSpecializationSymbol>(classTemplateSpecialization));
        return classTemplateSpecialization;
    }
}

ArrayTypeSymbol* SymbolTable::MakeArrayType(TypeSymbol* elementType, int64_t size, const Span& span)
{
    ArrayKey key(elementType, size);
    auto it = arrayTypeMap.find(key);
    if (it != arrayTypeMap.cend())
    {
        ArrayTypeSymbol* arrayType = it->second;
        return arrayType;
    }
    else
    {
        ArrayTypeSymbol* arrayType = new ArrayTypeSymbol(span, elementType->FullName() + U"[" + ToUtf32(std::to_string(size)) + U"]", elementType, size);
        SetTypeIdFor(arrayType);
        arrayTypeMap[key] = arrayType;
        arrayType->SetParent(&globalNs); 
        arrayType->SetSymbolTable(this);
        ArrayLengthFunction* arrayLengthFunction = new ArrayLengthFunction(arrayType);
        SetFunctionIdFor(arrayLengthFunction);
        arrayType->AddMember(arrayLengthFunction);
        ArrayBeginFunction* arrayBeginFunction = new ArrayBeginFunction(arrayType);
        SetFunctionIdFor(arrayBeginFunction);
        arrayType->AddMember(arrayBeginFunction);
        ArrayEndFunction* arrayEndFunction = new ArrayEndFunction(arrayType);
        SetFunctionIdFor(arrayEndFunction);
        arrayType->AddMember(arrayEndFunction);
        ArrayCBeginFunction* arrayCBeginFunction = new ArrayCBeginFunction(arrayType);
        SetFunctionIdFor(arrayCBeginFunction);
        arrayType->AddMember(arrayCBeginFunction);
        ArrayCEndFunction* arrayCEndFunction = new ArrayCEndFunction(arrayType);
        SetFunctionIdFor(arrayCEndFunction);
        arrayType->AddMember(arrayCEndFunction);
        TypedefSymbol* iterator = new TypedefSymbol(span, U"Iterator");
        iterator->SetAccess(SymbolAccess::public_);
        iterator->SetType(arrayType->ElementType()->AddPointer(span));
        iterator->SetBound();
        arrayType->AddMember(iterator);
        TypedefSymbol* constIterator = new TypedefSymbol(span, U"ConstIterator");
        constIterator->SetAccess(SymbolAccess::public_);
        constIterator->SetType(arrayType->ElementType()->AddConst(span)->AddPointer(span));
        constIterator->SetBound();
        arrayType->AddMember(constIterator);
        arrayTypes.push_back(std::unique_ptr<ArrayTypeSymbol>(arrayType));
        return arrayType;
    }
}

void SymbolTable::AddClassTemplateSpecializationsToClassTemplateSpecializationMap(const std::vector<ClassTemplateSpecializationSymbol*>& classTemplateSpecializations)
{
    for (ClassTemplateSpecializationSymbol* classTemplateSpecialization : classTemplateSpecializations)
    {
        classTemplateSpecialization->SetSymbolTable(this);
        ClassTemplateSpecializationKey key(classTemplateSpecialization->GetClassTemplate(), classTemplateSpecialization->TemplateArgumentTypes());
        auto it = classTemplateSpecializationMap.find(key);
        if (it == classTemplateSpecializationMap.cend())
        {
            classTemplateSpecializationMap[key] = classTemplateSpecialization;
        }
    }
}

void SymbolTable::AddConversion(FunctionSymbol* conversion)
{
    conversionTable.AddConversion(conversion);
}

FunctionSymbol* SymbolTable::GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType, const Span& span) const
{
    return conversionTable.GetConversion(sourceType, targetType, span);
}

void SymbolTable::AddPolymorphicClass(ClassTypeSymbol* polymorphicClass)
{
    if (!polymorphicClass->IsPolymorphic())
    {
        throw Exception("not a polymorphic class", polymorphicClass->GetSpan());
    }
    polymorphicClasses.insert(polymorphicClass);
}

void SymbolTable::AddClassHavingStaticConstructor(ClassTypeSymbol* classHavingStaticConstructor)
{
    if (!classHavingStaticConstructor->StaticConstructor())
    {
        throw Exception("not having static constructor", classHavingStaticConstructor->GetSpan());
    }
    classesHavingStaticConstructor.insert(classHavingStaticConstructor);
}

void SymbolTable::AddJsonClass(const std::u32string& jsonClass)
{
    jsonClasses.insert(jsonClass);
}

std::vector<TypeSymbol*> SymbolTable::Types() const
{
    std::vector<TypeSymbol*> types;
    for (const auto& p : typeNameMap)
    {
        TypeSymbol* type = p.second;
        types.push_back(type);
    }
    for (const std::unique_ptr<DerivedTypeSymbol>& dt : derivedTypes)
    {
        types.push_back(dt.get());
    }
    for (const std::unique_ptr<ClassTemplateSpecializationSymbol>& ts : classTemplateSpecializations)
    {
        types.push_back(ts.get());
    }
    return types;
}

void SymbolTable::Copy(const SymbolTable& that)
{
    for (const auto& p : that.typeIdMap)
    {
        typeIdMap[p.first] = p.second;
    }
    for (const auto& p : that.functionIdMap)
    {
        functionIdMap[p.first] = p.second;
    }
}

class IntrinsicConcepts
{
public:
    static void Init();
    static void Done();
    static IntrinsicConcepts& Instance() { return *instance; }
    void AddIntrinsicConcept(ConceptNode* intrinsicConcept);
    const std::vector<std::unique_ptr<ConceptNode>>& GetIntrinsicConcepts() const { return intrinsicConcepts; }
private:
    static std::unique_ptr<IntrinsicConcepts> instance;
    std::vector<std::unique_ptr<ConceptNode>> intrinsicConcepts;
};

std::unique_ptr<IntrinsicConcepts> IntrinsicConcepts::instance;

void IntrinsicConcepts::Init()
{
    instance.reset(new IntrinsicConcepts());
}

void IntrinsicConcepts::Done()
{
    instance.reset();
}

void IntrinsicConcepts::AddIntrinsicConcept(ConceptNode* intrinsicConcept)
{
    intrinsicConcepts.push_back(std::unique_ptr<ConceptNode>(intrinsicConcept));
}

void InitCoreSymbolTable(SymbolTable& symbolTable)
{
    BoolTypeSymbol* boolType = new BoolTypeSymbol(Span(), U"bool");
    SByteTypeSymbol* sbyteType = new SByteTypeSymbol(Span(), U"sbyte");
    ByteTypeSymbol* byteType = new ByteTypeSymbol(Span(), U"byte");
    ShortTypeSymbol* shortType = new ShortTypeSymbol(Span(), U"short");
    UShortTypeSymbol* ushortType = new UShortTypeSymbol(Span(), U"ushort");
    IntTypeSymbol* intType = new IntTypeSymbol(Span(), U"int");
    UIntTypeSymbol* uintType = new UIntTypeSymbol(Span(), U"uint");
    LongTypeSymbol* longType = new LongTypeSymbol(Span(), U"long");
    ULongTypeSymbol* ulongType = new ULongTypeSymbol(Span(), U"ulong");
    FloatTypeSymbol* floatType = new FloatTypeSymbol(Span(), U"float");
    DoubleTypeSymbol* doubleType = new DoubleTypeSymbol(Span(), U"double");
    CharTypeSymbol* charType = new CharTypeSymbol(Span(), U"char");
    WCharTypeSymbol* wcharType = new WCharTypeSymbol(Span(), U"wchar");
    UCharTypeSymbol* ucharType = new UCharTypeSymbol(Span(), U"uchar");
    VoidTypeSymbol* voidType = new VoidTypeSymbol(Span(), U"void");
    symbolTable.AddTypeSymbolToGlobalScope(boolType);
    symbolTable.AddTypeSymbolToGlobalScope(sbyteType);
    symbolTable.AddTypeSymbolToGlobalScope(byteType);
    symbolTable.AddTypeSymbolToGlobalScope(shortType);
    symbolTable.AddTypeSymbolToGlobalScope(ushortType);
    symbolTable.AddTypeSymbolToGlobalScope(intType);
    symbolTable.AddTypeSymbolToGlobalScope(uintType);
    symbolTable.AddTypeSymbolToGlobalScope(longType);
    symbolTable.AddTypeSymbolToGlobalScope(ulongType);
    symbolTable.AddTypeSymbolToGlobalScope(floatType);
    symbolTable.AddTypeSymbolToGlobalScope(doubleType);
    symbolTable.AddTypeSymbolToGlobalScope(charType);
    symbolTable.AddTypeSymbolToGlobalScope(wcharType);
    symbolTable.AddTypeSymbolToGlobalScope(ucharType);
    symbolTable.AddTypeSymbolToGlobalScope(voidType);
    symbolTable.AddTypeSymbolToGlobalScope(new NullPtrType(Span(), U"@nullptr_type"));
    MakeBasicTypeOperations(symbolTable, boolType, sbyteType, byteType, shortType, ushortType, intType, uintType, longType, ulongType, floatType, doubleType, charType, wcharType, ucharType, voidType);
    IntrinsicConcepts::Instance().AddIntrinsicConcept(new SameConceptNode());
    IntrinsicConcepts::Instance().AddIntrinsicConcept(new DerivedConceptNode());
    IntrinsicConcepts::Instance().AddIntrinsicConcept(new ConvertibleConceptNode());
    IntrinsicConcepts::Instance().AddIntrinsicConcept(new ExplicitlyConvertibleConceptNode());
    IntrinsicConcepts::Instance().AddIntrinsicConcept(new CommonConceptNode());
    IntrinsicConcepts::Instance().AddIntrinsicConcept(new NonreferenceTypeConceptNode());
    for (const std::unique_ptr<ConceptNode>& conceptNode : IntrinsicConcepts::Instance().GetIntrinsicConcepts())
    {
        symbolTable.BeginConcept(*conceptNode);
        ConceptSymbol* conceptSymbol = static_cast<ConceptSymbol*>(symbolTable.Container());
        conceptSymbol->SetAccess(SymbolAccess::public_);
        int n = conceptNode->TypeParameters().Count();
        for (int i = 0; i < n; ++i)
        {
            IdentifierNode* typeParamId = conceptNode->TypeParameters()[i];
            symbolTable.AddTemplateParameter(*typeParamId);
        }
        symbolTable.EndConcept();
        conceptSymbol->ComputeName();
    }
}

void CreateClassFile(const std::string& executableFilePath, const SymbolTable& symbolTable)
{
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Generating class file..." << std::endl;
    }
    std::string classFilePath = boost::filesystem::path(executableFilePath).replace_extension(".cls").generic_string();
    const std::unordered_set<ClassTypeSymbol*>& polymorphicClasses = symbolTable.PolymorphicClasses();
    uint32_t n = 0;
    for (ClassTypeSymbol* polymorphicClass : polymorphicClasses)
    {
        if (!polymorphicClass->IsVmtObjectCreated()) continue;
        ++n;
    }
    BinaryWriter writer(classFilePath);
    writer.WriteEncodedUInt(n);
    for (ClassTypeSymbol* polymorphicClass : polymorphicClasses)
    {
        if (!polymorphicClass->IsVmtObjectCreated()) continue;
        uint32_t typeId = polymorphicClass->TypeId();
        const std::string& vmtObjectName = polymorphicClass->VmtObjectName();
        uint32_t baseClassTypeId = 0;
        if (polymorphicClass->BaseClass())
        {
            baseClassTypeId = polymorphicClass->BaseClass()->TypeId();
        }
        writer.Write(typeId);
        writer.Write(vmtObjectName);
        writer.Write(baseClassTypeId);
    }
    const std::unordered_set<ClassTypeSymbol*>& classesHavingStaticConstructor = symbolTable.ClassesHavingStaticConstructor();
    uint32_t ns = classesHavingStaticConstructor.size();
    writer.WriteEncodedUInt(ns);
    for (ClassTypeSymbol* classHavingStaticConstructor : classesHavingStaticConstructor)
    {
        uint32_t typeId = classHavingStaticConstructor->TypeId();
        writer.Write(typeId);
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "==> " << classFilePath << std::endl;
    }
}

void InitSymbolTable()
{
    IntrinsicConcepts::Init();
    TypeIdCounter::Init();
    FunctionIdCounter::Init();
}

void DoneSymbolTable()
{
    FunctionIdCounter::Done();
    TypeIdCounter::Done();
    IntrinsicConcepts::Done();
}

} } // namespace cmajor::symbols
