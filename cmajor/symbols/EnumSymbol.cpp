// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/EnumSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/SymbolCollector.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

EnumTypeSymbol::EnumTypeSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::enumTypeSymbol, span_, name_), underlyingType()
{
}

void EnumTypeSymbol::Write(SymbolWriter& writer)
{
    TypeSymbol::Write(writer);
    writer.GetBinaryWriter().Write(underlyingType->TypeId());
}

void EnumTypeSymbol::Read(SymbolReader& reader)
{
    TypeSymbol::Read(reader);
    uint32_t underlyingTypeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, underlyingTypeId, 0);
}

void EnumTypeSymbol::EmplaceType(TypeSymbol* typeSymbol, int index)
{
    Assert(index == 0, "invalid emplace type index");
    underlyingType = typeSymbol;
}

std::string EnumTypeSymbol::Syntax() const
{
    std::string syntax = GetSpecifierStr();
    if (!syntax.empty())
    {
        syntax.append(1, ' ');
    }
    syntax.append("enum ");
    syntax.append(ToUtf8(DocName()));
    syntax.append(1, ';');
    return syntax;
}

void EnumTypeSymbol::Accept(SymbolCollector* collector)
{
    if (IsProject() && Access() == SymbolAccess::public_)
    {
        collector->AddEnumeratedType(this);
    }
}

void EnumTypeSymbol::CollectMembers(SymbolCollector* collector)
{
    TypeSymbol::Accept(collector);
}

void EnumTypeSymbol::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine(ToUtf8(Name()));
    formatter.WriteLine("full name: " + ToUtf8(FullNameWithSpecifiers()));
    formatter.WriteLine("underlying type: " + ToUtf8(underlyingType->FullName()));
    formatter.WriteLine("typeid: " + std::to_string(TypeId()));
    formatter.WriteLine("enumeration constants:");
    formatter.IncIndent();
    SymbolCollector collector;
    TypeSymbol::Accept(&collector);
    for (EnumConstantSymbol* enumConstant : collector.EnumerationConstants())
    {
        formatter.WriteLine();
        enumConstant->Dump(formatter);
    }
    formatter.DecIndent();
}

void EnumTypeSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be default", GetSpan());
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be constexpr", GetSpan());
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be nothrow", GetSpan());
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be throw", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("enumerated type cannot be unit_test", GetSpan());
    }
}

ValueType EnumTypeSymbol::GetValueType() const
{
    return underlyingType->GetValueType();
}

Value* EnumTypeSymbol::MakeValue() const
{
    return underlyingType->MakeValue();
}

std::u32string EnumTypeSymbol::Id() const
{
    return MangledName();
}

EnumConstantSymbol::EnumConstantSymbol(const Span& span_, const std::u32string& name_) : Symbol(SymbolType::enumConstantSymbol, span_, name_), evaluating(false)
{
}

void EnumConstantSymbol::SetValue(Value* value_)
{
    value.reset(value_);
}

void EnumConstantSymbol::Accept(SymbolCollector* collector)
{
    if (IsProject())
    {
        collector->AddEnumerationConstant(this);
    }
}

void EnumConstantSymbol::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine(ToUtf8(Name()));
    formatter.WriteLine("value: " + value->ToString());
}

void EnumConstantSymbol::Write(SymbolWriter& writer)
{
    Symbol::Write(writer);
    WriteValue(value.get(), writer.GetBinaryWriter());
}

void EnumConstantSymbol::Read(SymbolReader& reader)
{
    Symbol::Read(reader);
    value = ReadValue(reader.GetBinaryReader(), GetSpan());
}

EnumTypeDefaultConstructor::EnumTypeDefaultConstructor(const Span& span_, const std::u32string& name_) : 
    FunctionSymbol(SymbolType::enumTypeDefaultConstructor, span_, name_), underlyingTypeDefaultConstructor(nullptr)
{
}

EnumTypeDefaultConstructor::EnumTypeDefaultConstructor(EnumTypeSymbol* enumType_) : 
    FunctionSymbol(SymbolType::enumTypeDefaultConstructor, enumType_->GetSpan(), U"enumTypeDefaultConstructor"), underlyingTypeDefaultConstructor(nullptr)
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(enumType_->AddPointer(Span()));
    AddMember(thisParam);
    ComputeName();
    TypeSymbol* underlyingType = enumType_->UnderlyingType();
    Assert(underlyingType->IsBasicTypeSymbol(), "basic type expected");
    BasicTypeSymbol* basicTypeSymbol = static_cast<BasicTypeSymbol*>(underlyingType);
    underlyingTypeDefaultConstructor = basicTypeSymbol->DefaultConstructor();
}

void EnumTypeDefaultConstructor::Write(SymbolWriter& writer) 
{
    FunctionSymbol::Write(writer);
    Assert(underlyingTypeDefaultConstructor, "underlying default constructor not set");
    uint32_t defaultConstructorId = underlyingTypeDefaultConstructor->FunctionId();
    writer.GetBinaryWriter().Write(defaultConstructorId);
}

void EnumTypeDefaultConstructor::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    uint32_t defaultConstructorId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceFunctionRequest(this, defaultConstructorId, 0);
}

void EnumTypeDefaultConstructor::EmplaceFunction(FunctionSymbol* functionSymbol, int index)
{
    if (index == 0)
    {
        underlyingTypeDefaultConstructor = functionSymbol;
    }
    else
    {
        Assert(false, "invalid emplace function index");
    }
}

void EnumTypeDefaultConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(underlyingTypeDefaultConstructor, "underlying default constructor not set");
    underlyingTypeDefaultConstructor->GenerateCall(emitter, genObjects, flags);
}

EnumTypeCopyConstructor::EnumTypeCopyConstructor(const Span& span_, const std::u32string& name_) : 
    FunctionSymbol(SymbolType::enumTypeCopyConstructor, span_, name_), underlyingTypeCopyConstructor(nullptr)
{
}

EnumTypeCopyConstructor::EnumTypeCopyConstructor(EnumTypeSymbol* enumType_) : 
    FunctionSymbol(SymbolType::enumTypeCopyConstructor, enumType_->GetSpan(), U"enumTypeCopyConstructor"), underlyingTypeCopyConstructor(nullptr)
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(enumType_->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(enumType_);
    AddMember(thatParam);
    ComputeName();
    TypeSymbol* underlyingType = enumType_->UnderlyingType();
    Assert(underlyingType->IsBasicTypeSymbol(), "basic type expected");
    BasicTypeSymbol* basicTypeSymbol = static_cast<BasicTypeSymbol*>(underlyingType);
    underlyingTypeCopyConstructor = basicTypeSymbol->CopyConstructor();
}

void EnumTypeCopyConstructor::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    Assert(underlyingTypeCopyConstructor, "underlying copy constructor not set");
    uint32_t copyConstructorId = underlyingTypeCopyConstructor->FunctionId();
    writer.GetBinaryWriter().Write(copyConstructorId);
}

void EnumTypeCopyConstructor::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    uint32_t copyConstructorId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceFunctionRequest(this, copyConstructorId, 0);
}

void EnumTypeCopyConstructor::EmplaceFunction(FunctionSymbol* functionSymbol, int index)
{
    if (index == 0)
    {
        underlyingTypeCopyConstructor = functionSymbol;
    }
    else
    {
        Assert(false, "invalid emplace function index");
    }
}

void EnumTypeCopyConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(underlyingTypeCopyConstructor, "underlying copy constructor not set");
    underlyingTypeCopyConstructor->GenerateCall(emitter, genObjects, flags);
}

EnumTypeMoveConstructor::EnumTypeMoveConstructor(const Span& span_, const std::u32string& name_) : 
    FunctionSymbol(SymbolType::enumTypeMoveConstructor, span_, name_), underlyingTypeMoveConstructor(nullptr)
{
}

EnumTypeMoveConstructor::EnumTypeMoveConstructor(EnumTypeSymbol* enumType_) : 
    FunctionSymbol(SymbolType::enumTypeMoveConstructor, enumType_->GetSpan(), U"enumTypeMoveConstructor"), underlyingTypeMoveConstructor(nullptr)
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(enumType_->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(enumType_->AddRvalueReference(Span()));
    AddMember(thatParam);
    ComputeName();
    TypeSymbol* underlyingType = enumType_->UnderlyingType();
    Assert(underlyingType->IsBasicTypeSymbol(), "basic type expected");
    BasicTypeSymbol* basicTypeSymbol = static_cast<BasicTypeSymbol*>(underlyingType);
    underlyingTypeMoveConstructor = basicTypeSymbol->MoveConstructor();
}

void EnumTypeMoveConstructor::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    Assert(underlyingTypeMoveConstructor, "underlying move constructor not set");
    uint32_t moveConstructorId = underlyingTypeMoveConstructor->FunctionId();
    writer.GetBinaryWriter().Write(moveConstructorId);
}

void EnumTypeMoveConstructor::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    uint32_t moveConstructorId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceFunctionRequest(this, moveConstructorId, 0);
}

void EnumTypeMoveConstructor::EmplaceFunction(FunctionSymbol* functionSymbol, int index)
{
    if (index == 0)
    {
        underlyingTypeMoveConstructor = functionSymbol;
    }
    else
    {
        Assert(false, "invalid emplace function index");
    }
}

void EnumTypeMoveConstructor::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(underlyingTypeMoveConstructor, "underlying move constructor not set");
    underlyingTypeMoveConstructor->GenerateCall(emitter, genObjects, flags);
}

EnumTypeCopyAssignment::EnumTypeCopyAssignment(const Span& span_, const std::u32string& name_) : 
    FunctionSymbol(SymbolType::enumTypeCopyAssignment, span_, name_), underlyingTypeCopyAssignment(nullptr)
{
}

EnumTypeCopyAssignment::EnumTypeCopyAssignment(EnumTypeSymbol* enumType_, TypeSymbol* voidType_) :
    FunctionSymbol(SymbolType::enumTypeCopyAssignment, enumType_->GetSpan(), U"enumTypeCopyAssignment"), underlyingTypeCopyAssignment(nullptr)
{
    SetGroupName(U"operator=");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(enumType_->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(enumType_);
    AddMember(thatParam);
    SetReturnType(voidType_);
    ComputeName();
    TypeSymbol* underlyingType = enumType_->UnderlyingType();
    Assert(underlyingType->IsBasicTypeSymbol(), "basic type expected");
    BasicTypeSymbol* basicTypeSymbol = static_cast<BasicTypeSymbol*>(underlyingType);
    underlyingTypeCopyAssignment = basicTypeSymbol->CopyAssignment();
}

void EnumTypeCopyAssignment::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    Assert(underlyingTypeCopyAssignment, "underlying copy assignment not set");
    uint32_t copyAssignmentId = underlyingTypeCopyAssignment->FunctionId();
    writer.GetBinaryWriter().Write(copyAssignmentId);
}

void EnumTypeCopyAssignment::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    uint32_t copyAssignmentId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceFunctionRequest(this, copyAssignmentId, 0);
}

void EnumTypeCopyAssignment::EmplaceFunction(FunctionSymbol* functionSymbol, int index)
{
    if (index == 0)
    {
        underlyingTypeCopyAssignment = functionSymbol;
    }
    else
    {
        Assert(false, "invalid emplace function index");
    }
}

void EnumTypeCopyAssignment::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(underlyingTypeCopyAssignment, "underlying copy assignment not set");
    underlyingTypeCopyAssignment->GenerateCall(emitter, genObjects, flags);
}

EnumTypeMoveAssignment::EnumTypeMoveAssignment(const Span& span_, const std::u32string& name_) : 
    FunctionSymbol(SymbolType::enumTypeMoveAssignment, span_, name_), underlyingTypeMoveAssignment(nullptr)
{
}

EnumTypeMoveAssignment::EnumTypeMoveAssignment(EnumTypeSymbol* enumType_, TypeSymbol* voidType_) : 
    FunctionSymbol(SymbolType::enumTypeMoveAssignment, enumType_->GetSpan(), U"enumTypeMoveAssignment"), underlyingTypeMoveAssignment(nullptr)
{
    SetGroupName(U"operator=");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(enumType_->AddPointer(Span()));
    AddMember(thisParam);
    ParameterSymbol* thatParam = new ParameterSymbol(Span(), U"that");
    thatParam->SetType(enumType_->AddRvalueReference(Span()));
    AddMember(thatParam);
    SetReturnType(voidType_);
    ComputeName();
    TypeSymbol* underlyingType = enumType_->UnderlyingType();
    Assert(underlyingType->IsBasicTypeSymbol(), "basic type expected");
    BasicTypeSymbol* basicTypeSymbol = static_cast<BasicTypeSymbol*>(underlyingType);
    underlyingTypeMoveAssignment = basicTypeSymbol->MoveAssignment();
}

void EnumTypeMoveAssignment::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    Assert(underlyingTypeMoveAssignment, "underlying move assignment not set");
    uint32_t moveAssignmentId = underlyingTypeMoveAssignment->FunctionId();
    writer.GetBinaryWriter().Write(moveAssignmentId);
}

void EnumTypeMoveAssignment::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    uint32_t moveAssignmentId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceFunctionRequest(this, moveAssignmentId, 0);
}

void EnumTypeMoveAssignment::EmplaceFunction(FunctionSymbol* functionSymbol, int index)
{
    if (index == 0)
    {
        underlyingTypeMoveAssignment = functionSymbol;
    }
    else
    {
        Assert(false, "invalid emplace function index");
    }
}

void EnumTypeMoveAssignment::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(underlyingTypeMoveAssignment, "underlying move assignment not set");
    underlyingTypeMoveAssignment->GenerateCall(emitter, genObjects, flags);
}

EnumTypeReturn::EnumTypeReturn(const Span& span_, const std::u32string& name_) : 
    FunctionSymbol(SymbolType::enumTypeReturn, span_, name_), underlyingTypeReturn(nullptr)
{
}

EnumTypeReturn::EnumTypeReturn(EnumTypeSymbol* enumType_) : 
    FunctionSymbol(SymbolType::enumTypeReturn, enumType_->GetSpan(), U"enumTypeReturn"), underlyingTypeReturn(nullptr)
{
    SetGroupName(U"@return");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* valueParam = new ParameterSymbol(Span(), U"value");
    valueParam->SetType(enumType_);
    AddMember(valueParam);
    SetReturnType(enumType_);
    ComputeName();
    TypeSymbol* underlyingType = enumType_->UnderlyingType();
    Assert(underlyingType->IsBasicTypeSymbol(), "basic type expected");
    BasicTypeSymbol* basicTypeSymbol = static_cast<BasicTypeSymbol*>(underlyingType);
    underlyingTypeReturn = basicTypeSymbol->ReturnFun();
}

void EnumTypeReturn::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    Assert(underlyingTypeReturn, "underlying return not set");
    uint32_t returnId = underlyingTypeReturn->FunctionId();
    writer.GetBinaryWriter().Write(returnId);
}

void EnumTypeReturn::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    uint32_t returnId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceFunctionRequest(this, returnId, 0);
}

void EnumTypeReturn::EmplaceFunction(FunctionSymbol* functionSymbol, int index)
{
    if (index == 0)
    {
        underlyingTypeReturn = functionSymbol;
    }
    else
    {
        Assert(false, "invalid emplace function index");
    }
}

void EnumTypeReturn::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(underlyingTypeReturn, "underlying return not set");
    underlyingTypeReturn->GenerateCall(emitter, genObjects, flags);
}

EnumTypeEqualityOp::EnumTypeEqualityOp(const Span& span_, const std::u32string& name_) : 
    FunctionSymbol(SymbolType::enumTypeEquality, span_, name_), underlyingTypeEquality(nullptr)
{
}

EnumTypeEqualityOp::EnumTypeEqualityOp(EnumTypeSymbol* enumType_, TypeSymbol* boolType_) : 
    FunctionSymbol(SymbolType::enumTypeEquality, enumType_->GetSpan(), U"enumTypeEquality"), underlyingTypeEquality(nullptr)
{
    SetGroupName(U"operator==");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* leftParam = new ParameterSymbol(enumType_->GetSpan(), U"left");
    leftParam->SetType(enumType_);
    AddMember(leftParam);
    ParameterSymbol* rightParam = new ParameterSymbol(enumType_->GetSpan(), U"right");
    rightParam->SetType(enumType_);
    AddMember(rightParam);
    SetReturnType(boolType_);
    ComputeName();
    TypeSymbol* underlyingType = enumType_->UnderlyingType();
    Assert(underlyingType->IsBasicTypeSymbol(), "basic type expected");
    BasicTypeSymbol* basicTypeSymbol = static_cast<BasicTypeSymbol*>(underlyingType);
    underlyingTypeEquality = basicTypeSymbol->EqualityOp();
}

void EnumTypeEqualityOp::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    Assert(underlyingTypeEquality, "underlying equality not set");
    uint32_t equalityId = underlyingTypeEquality->FunctionId();
    writer.GetBinaryWriter().Write(equalityId);
}

void EnumTypeEqualityOp::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    uint32_t equalityId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceFunctionRequest(this, equalityId, 0);
}

void EnumTypeEqualityOp::EmplaceFunction(FunctionSymbol* functionSymbol, int index)
{
    if (index == 0)
    {
        underlyingTypeEquality = functionSymbol;
    }
    else
    {
        Assert(false, "invalid emplace function index");
    }
}

void EnumTypeEqualityOp::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
    Assert(underlyingTypeEquality, "underlying equality not set");
    underlyingTypeEquality->GenerateCall(emitter, genObjects, flags);
}

EnumTypeToUnderlyingTypeConversion::EnumTypeToUnderlyingTypeConversion(const Span& span_, const std::u32string& name_) :
    FunctionSymbol(SymbolType::enumTypeToUnderlyingType, span_, name_), sourceType(), targetType()
{
}

EnumTypeToUnderlyingTypeConversion::EnumTypeToUnderlyingTypeConversion(const Span& span_, const std::u32string& name_, TypeSymbol* sourceType_, TypeSymbol* targetType_) : 
    FunctionSymbol(SymbolType::enumTypeToUnderlyingType, span_, name_), sourceType(sourceType_), targetType(targetType_)
{
    SetConversion();
}

void EnumTypeToUnderlyingTypeConversion::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    writer.GetBinaryWriter().Write(sourceType->TypeId());
    writer.GetBinaryWriter().Write(targetType->TypeId());
}

void EnumTypeToUnderlyingTypeConversion::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    uint32_t sourceTypeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, sourceTypeId, 1);
    uint32_t targetTypeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, targetTypeId, 2);
}

void EnumTypeToUnderlyingTypeConversion::EmplaceType(TypeSymbol* typeSymbol, int index)
{
    if (index == 1)
    {
        sourceType = typeSymbol;
    }
    else if (index == 2)
    {
        targetType = typeSymbol;
    }
    else
    {
        FunctionSymbol::EmplaceType(typeSymbol, index);
    }
}

void EnumTypeToUnderlyingTypeConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
}

UnderlyingTypeToEnumTypeConversion::UnderlyingTypeToEnumTypeConversion(const Span& span_, const std::u32string& name_)
    : FunctionSymbol(SymbolType::underlyingToEnumType, span_, name_), sourceType(), targetType()
{
}

UnderlyingTypeToEnumTypeConversion::UnderlyingTypeToEnumTypeConversion(const Span& span_, const std::u32string& name_, TypeSymbol* sourceType_, TypeSymbol* targetType_)
    : FunctionSymbol(SymbolType::underlyingToEnumType, span_, name_), sourceType(sourceType_), targetType(targetType_)
{
    SetConversion();
}

void UnderlyingTypeToEnumTypeConversion::Write(SymbolWriter& writer)
{
    FunctionSymbol::Write(writer);
    writer.GetBinaryWriter().Write(sourceType->TypeId());
    writer.GetBinaryWriter().Write(targetType->TypeId());
}

void UnderlyingTypeToEnumTypeConversion::Read(SymbolReader& reader)
{
    FunctionSymbol::Read(reader);
    uint32_t sourceTypeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, sourceTypeId, 1);
    uint32_t targetTypeId = reader.GetBinaryReader().ReadUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, targetTypeId, 2);
}

void UnderlyingTypeToEnumTypeConversion::EmplaceType(TypeSymbol* typeSymbol, int index)
{
    if (index == 1)
    {
        sourceType = typeSymbol;
    }
    else if (index == 2)
    {
        targetType = typeSymbol;
    }
    else
    {
        FunctionSymbol::EmplaceType(typeSymbol, index);
    }
}

void UnderlyingTypeToEnumTypeConversion::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags)
{
}

} } // namespace cmajor::symbols
