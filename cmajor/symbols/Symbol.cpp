// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/Symbol.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/BasicTypeSymbol.hpp>
#include <cmajor/symbols/BasicTypeOperation.hpp>
#include <cmajor/symbols/DerivedTypeSymbol.hpp>
#include <cmajor/symbols/NamespaceSymbol.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/ClassTypeSymbol.hpp>
#include <cmajor/symbols/InterfaceTypeSymbol.hpp>
#include <cmajor/symbols/DelegateSymbol.hpp>
#include <cmajor/symbols/TypedefSymbol.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/ConstantSymbol.hpp>
#include <cmajor/symbols/EnumSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/Sha1.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

const char* symbolTypeStr[uint8_t(SymbolType::maxSymbol)] =
{
    "boolTypeSymbol", "sbyteTypeSymbol", "byteTypeSymbol", "shortTypeSymbol", "ushortTypeSymbol", "intTypeSymbol", "uintTypeSymbol", "longTypeSymbol", "ulongTypeSymbol", "floatTypeSymbol", "doubleTypeSymbol",
    "charTypeSymbol", "wcharTypeSymbol", "ucharTypeSymbol", "voidTypeSymbol", "nullPtrTypeSymbol",
    "derivedTypeSymbol"
    "namespaceSymbol", "functionSymbol", "staticConstructorSymbol", "constructorSymbol", "destructorSymbol", "memberFunctionSymbol", "functionGroupSymbol", "classTypeSymbol", "interfaceTypeSymbol",
    "delegateTypeSymbol", "classDelegateTypeSymbol", "declarationBlock", "typedefSymbol", "constantSymbol", "enumTypeSymbol", "enumConstantSymbol",
    "templateParameterSymbol", "boundTemplateParameterSymbol", "parameterSymbol", "localVariableSymbol", "memberVariableSymbol",
    "basicTypeUnaryPlus", "basicTypeIntUnaryMinus", "basicTypeFloatUnaryMinus", "basicTypeComplement", "basicTypeAdd", "basicTypeFAdd", "basicTypeSub", "basicTypeFSub", "basicTypeMul", "basicTypeFMul",
    "basicTypeSDiv", "basicTypeUDiv", "basicTypeFDiv", "basicTypeSRem", "basicTypeURem", "basicTypeAnd", "basicTypeOr", "basicTypeXor", "basicTypeShl", "basicTypeAShr", "basicTypeLShr",
    "basicTypeNot", "basicTypeIntegerEquality", "basicTypeUnsignedIntegerLessThan", "basicTypeSignedIntegerLessThan", "basicTypeFloatingEquality"", basicTypeFloatingLessThan",
    "defaultInt1", "defaultInt8", "defaultInt16", "defaultInt32", "defaultInt64", "defaultFloat", "defaultDouble", "basicTypeCopyCtor", "basicTypeCopyAssignment", "basicTypeReturn",
    "basicTypeImplicitSignExtension", "basicTypeImplicitZeroExtension", "basicTypeExplicitSignExtension", "basicTypeExplicitZeroExtension", "basicTypeTruncation", "basicTypeBitCast",
    "basicTypeImplicitUnsignedIntToFloating", "basicTypeImplicitSignedIntToFloating", "basicTypeExplicitUnsignedIntToFloating", "basicTypeExplicitSignedIntToFloating",
    "basicTypeFloatingToUnsignedInt", "basicTypeFloatingToSignedInt", "basicTypeFloatingExtension", "basicTypeFloatingTruncation",
    "namespaceTypeSymbol", "functionGroupTypeSymbol"
};

std::string SymbolTypeStr(SymbolType symbolType)
{
    return symbolTypeStr[static_cast<uint8_t>(symbolType)];
}

std::string SymbolFlagStr(SymbolFlags symbolFlags)
{
    std::string s;
    SymbolAccess access = SymbolAccess(symbolFlags & SymbolFlags::access);
    switch (access)
    { 
        case SymbolAccess::private_: s.append("private"); break;
        case SymbolAccess::protected_: s.append("protected"); break;
        case SymbolAccess::internal_: s.append("internal"); break;
        case SymbolAccess::public_: s.append("public"); break;
    }
    if ((symbolFlags & SymbolFlags::static_) != SymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("static");
    }
    if ((symbolFlags & SymbolFlags::nothrow_) != SymbolFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("nothrow");
    }
    return s;
}


Symbol::Symbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) : 
    symbolType(symbolType_), span(span_), name(name_), flags(SymbolFlags::project), parent(nullptr), symbolTable(nullptr), module(nullptr), irObject(nullptr)
{
}

Symbol::~Symbol()
{
}

void Symbol::Write(SymbolWriter& writer)
{
    SymbolFlags f = flags & ~(SymbolFlags::project | SymbolFlags::bound);
    writer.GetBinaryWriter().Write(static_cast<uint8_t>(f));
    writer.GetBinaryWriter().Write(mangledName);
}

void Symbol::Read(SymbolReader& reader)
{
    flags = static_cast<SymbolFlags>(reader.GetBinaryReader().ReadByte());
    mangledName = reader.GetBinaryReader().ReadUtf32String();
}

std::u32string Symbol::FullName() const
{
    std::u32string fullName;
    if (parent)
    {
        fullName.append(parent->FullName());
    }
    if (!fullName.empty())
    {
        fullName.append(1, '.');
    }
    fullName.append(Name());
    return fullName;
}

std::u32string Symbol::FullNameWithSpecifiers() const
{
    std::u32string fullNameWithSpecifiers = ToUtf32(SymbolFlagStr(flags));
    if (!fullNameWithSpecifiers.empty())
    {
        fullNameWithSpecifiers.append(1, U' ');
    }
    fullNameWithSpecifiers.append(FullName());
    return fullNameWithSpecifiers;
}

void Symbol::ComputeMangledName()
{
    mangledName = ToUtf32(TypeString());
    mangledName.append(1, U'_').append(SimpleName());
    mangledName.append(1, U'_').append(ToUtf32(GetSha1MessageDigest(ToUtf8(FullNameWithSpecifiers()))));
}

void Symbol::SetMangledName(const std::u32string& mangledName_)
{
    mangledName = mangledName_;
}

void Symbol::SetAccess(Specifiers accessSpecifiers)
{
    ContainerSymbol* cls = ContainingClassNoThrow();
    SymbolAccess access = SymbolAccess::private_;
    bool classMember = true;
    if (!cls)
    {
        access = SymbolAccess::internal_;
        classMember = false;
        ContainerSymbol* intf = ContainingInterfaceNoThrow();
        if (intf)
        {
            access = SymbolAccess::public_;
        }
    }
    if (accessSpecifiers == Specifiers::public_)
    {
        access = SymbolAccess::public_;
    }
    else if (accessSpecifiers == Specifiers::protected_)
    {
        if (classMember)
        {
            access = SymbolAccess::protected_;
        }
        else
        {
            throw Exception("only class members can have protected access", GetSpan());
        }
    }
    else if (accessSpecifiers == Specifiers::internal_)
    {
        access = SymbolAccess::internal_;
    }
    else if (accessSpecifiers == Specifiers::private_)
    {
        if (classMember)
        {
            access = SymbolAccess::private_;
        }
        else
        {
            throw Exception("only class members can have private access", GetSpan());
        }
    }
    else if (accessSpecifiers != Specifiers::none)
    {
        throw Exception("invalid combination of access specifiers: " + SpecifierStr(accessSpecifiers), GetSpan());
    }
    SetAccess(access);
}

bool Symbol::IsSameParentOrAncestorOf(const Symbol* that) const
{
    if (!that)
    {
        return false;
    }
    else if (this == that)
    {
        return true;
    }
    else if (that->parent)
    {
        return IsSameParentOrAncestorOf(that->parent);
    }
    else
    {
        return false;
    }
}

const NamespaceSymbol* Symbol::Ns() const
{
    if (symbolType == SymbolType::namespaceSymbol)
    {
        return static_cast<const NamespaceSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->Ns();
        }
        else
        {
            throw std::runtime_error("namespace symbol not found");
        }
    }
}

NamespaceSymbol* Symbol::Ns()
{
    if (symbolType == SymbolType::namespaceSymbol)
    {
        return static_cast<NamespaceSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->Ns();
        }
        else
        {
            throw std::runtime_error("namespace symbol not found");
        }
    }
}

const ClassTypeSymbol* Symbol::ClassNoThrow() const
{
    if (IsClassTypeSymbol())
    {
        return static_cast<const ClassTypeSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->ClassNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

ClassTypeSymbol* Symbol::ClassNoThrow()
{
    if (IsClassTypeSymbol())
    {
        return static_cast<ClassTypeSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->ClassNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

const ContainerSymbol* Symbol::ClassOrNsNoThrow() const
{
    if (symbolType == SymbolType::namespaceSymbol)
    {
        return static_cast<const NamespaceSymbol*>(this);
    }
    else if (IsClassTypeSymbol())
    {
        return static_cast<const ClassTypeSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->ClassOrNsNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

ContainerSymbol* Symbol::ClassOrNsNoThrow()
{
    if (symbolType == SymbolType::namespaceSymbol)
    {
        return static_cast<NamespaceSymbol*>(this);
    }
    else if (IsClassTypeSymbol())
    {
        return static_cast<ClassTypeSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->ClassOrNsNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

const ContainerSymbol* Symbol::ClassInterfaceOrNsNoThrow() const
{
    if (symbolType == SymbolType::namespaceSymbol)
    {
        return static_cast<const NamespaceSymbol*>(this);
    }
    else if (symbolType == SymbolType::interfaceTypeSymbol)
    {
        return static_cast<const InterfaceTypeSymbol*>(this);
    }
    else if (IsClassTypeSymbol())
    {
        return static_cast<const ClassTypeSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->ClassInterfaceOrNsNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

ContainerSymbol* Symbol::ClassInterfaceOrNsNoThrow()
{
    if (symbolType == SymbolType::namespaceSymbol)
    {
        return static_cast<NamespaceSymbol*>(this);
    }
    else if (symbolType == SymbolType::interfaceTypeSymbol)
    {
        return static_cast<InterfaceTypeSymbol*>(this);
    }
    else if (IsClassTypeSymbol())
    {
        return static_cast<ClassTypeSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->ClassInterfaceOrNsNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

const ClassTypeSymbol* Symbol::Class() const
{
    const ClassTypeSymbol* cls = ClassNoThrow();
    if (cls)
    {
        return cls;
    }
    else
    {
        throw std::runtime_error("class type symbol not found");
    }
}

ClassTypeSymbol* Symbol::Class()
{
    ClassTypeSymbol* cls = ClassNoThrow();
    if (cls)
    {
        return cls;
    }
    else
    {
        throw std::runtime_error("class type symbol not found");
    }
}

const ClassTypeSymbol* Symbol::ContainingClassNoThrow() const
{
    if (parent)
    {
        return parent->ClassNoThrow();
    }
    else
    {
        return nullptr;
    }
}

ClassTypeSymbol* Symbol::ContainingClassNoThrow()
{
    if (parent)
    {
        return parent->ClassNoThrow();
    }
    else
    {
        return nullptr;
    }
}

const InterfaceTypeSymbol* Symbol::InterfaceNoThrow() const
{
    if (symbolType == SymbolType::interfaceTypeSymbol)
    {
        return static_cast<const InterfaceTypeSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->InterfaceNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

InterfaceTypeSymbol* Symbol::InterfaceNoThrow()
{
    if (symbolType == SymbolType::interfaceTypeSymbol)
    {
        return static_cast<InterfaceTypeSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->InterfaceNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

const InterfaceTypeSymbol* Symbol::ContainingInterfaceNoThrow() const
{
    if (parent)
    {
        return parent->InterfaceNoThrow();
    }
    else
    {
        return nullptr;
    }
}

InterfaceTypeSymbol* Symbol::ContainingInterfaceNoThrow()
{
    if (parent)
    {
        return parent->InterfaceNoThrow();
    }
    else
    {
        return nullptr;
    }
}

const FunctionSymbol* Symbol::FunctionNoThrow() const
{
    if (IsFunctionSymbol())
    {
        return static_cast<const FunctionSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->FunctionNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

FunctionSymbol* Symbol::FunctionNoThrow()
{
    if (IsFunctionSymbol())
    {
        return static_cast<FunctionSymbol*>(this);
    }
    else
    {
        if (parent)
        {
            return parent->FunctionNoThrow();
        }
        else
        {
            return nullptr;
        }
    }
}

const FunctionSymbol* Symbol::Function() const
{
    const FunctionSymbol* function = FunctionNoThrow();
    if (function)
    {
        return function;
    }
    else
    {
        throw std::runtime_error("function symbol not found");
    }
}

FunctionSymbol* Symbol::Function()
{
    FunctionSymbol* function = FunctionNoThrow();
    if (function)
    {
        return function;
    }
    else
    {
        throw std::runtime_error("function symbol not found");
    }
}

const FunctionSymbol* Symbol::ContainingFunctionNoThrow() const
{
    if (parent)
    {
        return parent->FunctionNoThrow();
    }
    else
    {
        return nullptr;
    }
}

FunctionSymbol* Symbol::ContainingFunctionNoThrow()
{
    if (parent)
    {
        return parent->FunctionNoThrow();
    }
    else
    {
        return nullptr;
    }
}

const ContainerScope* Symbol::ClassOrNsScope() const
{
    const ContainerSymbol* classOrNs = ClassOrNsNoThrow();
    if (classOrNs)
    {
        return classOrNs->GetContainerScope();
    }
    else
    {
        throw std::runtime_error("class or namespace scope not found");
    }
}

ContainerScope* Symbol::ClassOrNsScope()
{
    ContainerSymbol* classOrNs = ClassOrNsNoThrow();
    if (classOrNs)
    {
        return classOrNs->GetContainerScope();
    }
    else
    {
        throw std::runtime_error("class or namespace scope not found");
    }
}

const ContainerScope* Symbol::ClassInterfaceOrNsScope() const
{
    const ContainerSymbol* classInterfaceOrNs = ClassInterfaceOrNsNoThrow();
    if (classInterfaceOrNs)
    {
        return classInterfaceOrNs->GetContainerScope();
    }
    else
    {
        throw std::runtime_error("class, interface or namespace scope not found");
    }
}

ContainerScope* Symbol::ClassInterfaceOrNsScope()
{
    ContainerSymbol* classInterfaceOrNs = ClassInterfaceOrNsNoThrow();
    if (classInterfaceOrNs)
    {
        return classInterfaceOrNs->GetContainerScope();
    }
    else
    {
        throw std::runtime_error("class, interface or namespace scope not found");
    }
}


SymbolCreator::~SymbolCreator()
{
}

template<typename SymbolT>
class ConcreteSymbolCreator : public SymbolCreator
{
public:
    Symbol* CreateSymbol(const Span& span, const std::u32string& name)
    {
        return new SymbolT(span, name);
    }
};

void SymbolFactory::Init()
{
    instance.reset(new SymbolFactory());
}

void SymbolFactory::Done()
{
    instance.reset();
}

std::unique_ptr<SymbolFactory> SymbolFactory::instance;

SymbolFactory::SymbolFactory()
{
    symbolCreators.resize(static_cast<uint8_t>(SymbolType::maxSymbol));
    Register(SymbolType::boolTypeSymbol, new ConcreteSymbolCreator<BoolTypeSymbol>());
    Register(SymbolType::sbyteTypeSymbol, new ConcreteSymbolCreator<SByteTypeSymbol>());
    Register(SymbolType::byteTypeSymbol, new ConcreteSymbolCreator<ByteTypeSymbol>());
    Register(SymbolType::shortTypeSymbol, new ConcreteSymbolCreator<ShortTypeSymbol>());
    Register(SymbolType::ushortTypeSymbol, new ConcreteSymbolCreator<UShortTypeSymbol>());
    Register(SymbolType::intTypeSymbol, new ConcreteSymbolCreator<IntTypeSymbol>());
    Register(SymbolType::uintTypeSymbol, new ConcreteSymbolCreator<UIntTypeSymbol>());
    Register(SymbolType::longTypeSymbol, new ConcreteSymbolCreator<LongTypeSymbol>());
    Register(SymbolType::ulongTypeSymbol, new ConcreteSymbolCreator<ULongTypeSymbol>());
    Register(SymbolType::floatTypeSymbol, new ConcreteSymbolCreator<FloatTypeSymbol>());
    Register(SymbolType::doubleTypeSymbol, new ConcreteSymbolCreator<DoubleTypeSymbol>());
    Register(SymbolType::charTypeSymbol, new ConcreteSymbolCreator<CharTypeSymbol>());
    Register(SymbolType::wcharTypeSymbol, new ConcreteSymbolCreator<WCharTypeSymbol>());
    Register(SymbolType::ucharTypeSymbol, new ConcreteSymbolCreator<UCharTypeSymbol>());
    Register(SymbolType::voidTypeSymbol, new ConcreteSymbolCreator<VoidTypeSymbol>());
    Register(SymbolType::nullPtrTypeSymbol, new ConcreteSymbolCreator<NullPtrType>());
    Register(SymbolType::derivedTypeSymbol, new ConcreteSymbolCreator<DerivedTypeSymbol>());
    Register(SymbolType::namespaceSymbol, new ConcreteSymbolCreator<NamespaceSymbol>());
    Register(SymbolType::functionSymbol, new ConcreteSymbolCreator<FunctionSymbol>());
    Register(SymbolType::staticConstructorSymbol, new ConcreteSymbolCreator<StaticConstructorSymbol>());
    Register(SymbolType::constructorSymbol, new ConcreteSymbolCreator<ConstructorSymbol>());
    Register(SymbolType::destructorSymbol, new ConcreteSymbolCreator<DestructorSymbol>());
    Register(SymbolType::memberFunctionSymbol, new ConcreteSymbolCreator<MemberFunctionSymbol>());
    Register(SymbolType::functionGroupSymbol, new ConcreteSymbolCreator<FunctionGroupSymbol>());
    Register(SymbolType::classTypeSymbol, new ConcreteSymbolCreator<ClassTypeSymbol>());
    Register(SymbolType::interfaceTypeSymbol, new ConcreteSymbolCreator<InterfaceTypeSymbol>());
    Register(SymbolType::delegateTypeSymbol, new ConcreteSymbolCreator<DelegateTypeSymbol>());
    Register(SymbolType::classDelegateTypeSymbol, new ConcreteSymbolCreator<ClassDelegateTypeSymbol>());
    Register(SymbolType::declarationBlock, new ConcreteSymbolCreator<DeclarationBlock>());
    Register(SymbolType::typedefSymbol, new ConcreteSymbolCreator<TypedefSymbol>());
    Register(SymbolType::constantSymbol, new ConcreteSymbolCreator<ConstantSymbol>());
    Register(SymbolType::enumTypeSymbol, new ConcreteSymbolCreator<EnumTypeSymbol>());
    Register(SymbolType::enumConstantSymbol, new ConcreteSymbolCreator<EnumConstantSymbol>());
    Register(SymbolType::templateParameterSymbol, new ConcreteSymbolCreator<TemplateParameterSymbol>());
    Register(SymbolType::boundTemplateParameterSymbol, new ConcreteSymbolCreator<BoundTemplateParameterSymbol>());
    Register(SymbolType::parameterSymbol, new ConcreteSymbolCreator<ParameterSymbol>());
    Register(SymbolType::localVariableSymbol, new ConcreteSymbolCreator<LocalVariableSymbol>());
    Register(SymbolType::memberVariableSymbol, new ConcreteSymbolCreator<MemberVariableSymbol>());
    Register(SymbolType::basicTypeUnaryPlus, new ConcreteSymbolCreator<BasicTypeUnaryPlusOperation>());
    Register(SymbolType::basicTypeIntUnaryMinus, new ConcreteSymbolCreator<BasicTypeIntUnaryMinusOperation>());
    Register(SymbolType::basicTypeFloatUnaryMinus, new ConcreteSymbolCreator<BasicTypeFloatUnaryMinusOperation>());
    Register(SymbolType::basicTypeComplement, new ConcreteSymbolCreator<BasicTypeComplementOperation>());
    Register(SymbolType::basicTypeAdd, new ConcreteSymbolCreator<BasicTypeAddOperation>());
    Register(SymbolType::basicTypeFAdd, new ConcreteSymbolCreator<BasicTypeFAddOperation>());
    Register(SymbolType::basicTypeSub, new ConcreteSymbolCreator<BasicTypeSubOperation>());
    Register(SymbolType::basicTypeFSub, new ConcreteSymbolCreator<BasicTypeFSubOperation>());
    Register(SymbolType::basicTypeMul, new ConcreteSymbolCreator<BasicTypeMulOperation>());
    Register(SymbolType::basicTypeFMul, new ConcreteSymbolCreator<BasicTypeFMulOperation>());
    Register(SymbolType::basicTypeSDiv, new ConcreteSymbolCreator<BasicTypeSDivOperation>());
    Register(SymbolType::basicTypeUDiv, new ConcreteSymbolCreator<BasicTypeUDivOperation>());
    Register(SymbolType::basicTypeFDiv, new ConcreteSymbolCreator<BasicTypeFDivOperation>());
    Register(SymbolType::basicTypeSRem, new ConcreteSymbolCreator<BasicTypeSRemOperation>());
    Register(SymbolType::basicTypeURem, new ConcreteSymbolCreator<BasicTypeURemOperation>());
    Register(SymbolType::basicTypeAnd, new ConcreteSymbolCreator<BasicTypeAndOperation>());
    Register(SymbolType::basicTypeOr, new ConcreteSymbolCreator<BasicTypeOrOperation>());
    Register(SymbolType::basicTypeXor, new ConcreteSymbolCreator<BasicTypeXorOperation>());
    Register(SymbolType::basicTypeShl, new ConcreteSymbolCreator<BasicTypeShlOperation>());
    Register(SymbolType::basicTypeAShr, new ConcreteSymbolCreator<BasicTypeAShrOperation>());
    Register(SymbolType::basicTypeLShr, new ConcreteSymbolCreator<BasicTypeLShrOperation>());
    Register(SymbolType::basicTypeNot, new ConcreteSymbolCreator<BasicTypeNotOperation>());
    Register(SymbolType::basicTypeIntegerEquality, new ConcreteSymbolCreator<BasicTypeIntegerEqualityOperation>());
    Register(SymbolType::basicTypeUnsignedIntegerLessThan, new ConcreteSymbolCreator<BasicTypeUnsignedIntegerLessThanOperation>());
    Register(SymbolType::basicTypeSignedIntegerLessThan, new ConcreteSymbolCreator<BasicTypeSignedIntegerLessThanOperation>());
    Register(SymbolType::basicTypeFloatingEquality, new ConcreteSymbolCreator<BasicTypeFloatingEqualityOperation>());
    Register(SymbolType::basicTypeFloatingLessThan, new ConcreteSymbolCreator<BasicTypeFloatingLessThanOperation>());
    Register(SymbolType::defaultInt1, new ConcreteSymbolCreator<BasicTypeDefaultInt1Operation>());
    Register(SymbolType::defaultInt8, new ConcreteSymbolCreator<BasicTypeDefaultInt8Operation>());
    Register(SymbolType::defaultInt16, new ConcreteSymbolCreator<BasicTypeDefaultInt16Operation>());
    Register(SymbolType::defaultInt32, new ConcreteSymbolCreator<BasicTypeDefaultInt32Operation>());
    Register(SymbolType::defaultInt64, new ConcreteSymbolCreator<BasicTypeDefaultInt64Operation>());
    Register(SymbolType::defaultFloat, new ConcreteSymbolCreator<BasicTypeDefaultFloatOperation>());
    Register(SymbolType::defaultDouble, new ConcreteSymbolCreator<BasicTypeDefaultDoubleOperation>());
    Register(SymbolType::basicTypeCopyCtor, new ConcreteSymbolCreator<BasicTypeCopyCtor>());
    Register(SymbolType::basicTypeCopyAssignment, new ConcreteSymbolCreator<BasicTypeCopyAssignment>());
    Register(SymbolType::basicTypeReturn, new ConcreteSymbolCreator<BasicTypeReturn>());
    Register(SymbolType::basicTypeImplicitSignExtension, new ConcreteSymbolCreator<BasicTypeImplicitSignExtensionOperation>());
    Register(SymbolType::basicTypeImplicitZeroExtension, new ConcreteSymbolCreator<BasicTypeImplicitZeroExtensionOperation>());
    Register(SymbolType::basicTypeExplicitSignExtension, new ConcreteSymbolCreator<BasicTypeExplicitSignExtensionOperation>());
    Register(SymbolType::basicTypeExplicitZeroExtension, new ConcreteSymbolCreator<BasicTypeExplicitZeroExtensionOperation>());
    Register(SymbolType::basicTypeTruncation, new ConcreteSymbolCreator<BasicTypeTruncationOperation>());
    Register(SymbolType::basicTypeBitCast, new ConcreteSymbolCreator<BasicTypeBitCastOperation>());
    Register(SymbolType::basicTypeImplicitUnsignedIntToFloating, new ConcreteSymbolCreator<BasicTypeImplicitUnsignedIntToFloatingOperation>());
    Register(SymbolType::basicTypeImplicitSignedIntToFloating, new ConcreteSymbolCreator<BasicTypeImplicitSignedIntToFloatingOperation>());
    Register(SymbolType::basicTypeExplicitUnsignedIntToFloating, new ConcreteSymbolCreator<BasicTypeExplicitUnsignedIntToFloatingOperation>());
    Register(SymbolType::basicTypeExplicitSignedIntToFloating, new ConcreteSymbolCreator<BasicTypeExplicitSignedIntToFloatingOperation>());
    Register(SymbolType::basicTypeFloatingToUnsignedInt, new ConcreteSymbolCreator<BasicTypeFloatingToUnsignedIntOperation>());
    Register(SymbolType::basicTypeFloatingToSignedInt, new ConcreteSymbolCreator<BasicTypeFloatingToSignedIntOperation>());
    Register(SymbolType::basicTypeFloatingExtension, new ConcreteSymbolCreator<BasicTypeFloatingExtensionOperation>());
    Register(SymbolType::basicTypeFloatingTruncation, new ConcreteSymbolCreator<BasicTypeFloatingTruncationOperation>());
}

Symbol* SymbolFactory::CreateSymbol(SymbolType symbolType, const Span& span, const std::u32string& name)
{
    const std::unique_ptr<SymbolCreator>& symbolCreator = symbolCreators[static_cast<uint8_t>(symbolType)];
    if (symbolCreator)
    {
        Symbol* symbol = symbolCreator->CreateSymbol(span, name);
        if (symbol)
        {
            return symbol;
        }
        else
        {
            throw std::runtime_error("could not create symbol");
        }
    }
    else
    {
        throw std::runtime_error("no creator for symbol type '" + SymbolTypeStr(symbolType) + "'");
    }
}

void SymbolFactory::Register(SymbolType symbolType, SymbolCreator* creator)
{
    symbolCreators[static_cast<uint8_t>(symbolType)] = std::unique_ptr<SymbolCreator>(creator);
}

void InitSymbol() 
{
    SymbolFactory::Init();
}

void DoneSymbol()
{
    SymbolFactory::Done();
}

} } // namespace cmajor::symbols
