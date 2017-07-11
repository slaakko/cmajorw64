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
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

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
    ContainerSymbol(SymbolType::functionSymbol, span_, name_), groupName(), parameters(), localVariables(), returnType(), flags(FunctionSymbolFlags::none)
{
}

FunctionSymbol::FunctionSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) : 
    ContainerSymbol(symbolType_, span_, name_), groupName(), parameters(), localVariables(), returnType(), flags(FunctionSymbolFlags::none)
{
}

void FunctionSymbol::Write(SymbolWriter& writer)
{
    ContainerSymbol::Write(writer);
    writer.GetBinaryWriter().Write(groupName);
    uint32_t returnTypeId = 0;
    if (returnType)
    {
        returnTypeId = returnType->TypeId();
    }
    writer.GetBinaryWriter().WriteEncodedUInt(returnTypeId);
    writer.GetBinaryWriter().Write(static_cast<uint16_t>(flags));
}

void FunctionSymbol::Read(SymbolReader& reader)
{
    ContainerSymbol::Read(reader);
    groupName = reader.GetBinaryReader().ReadUtf32String();
    uint32_t returnTypeId = reader.GetBinaryReader().ReadEncodedUInt();
    if (returnTypeId != 0)
    {
        GetSymbolTable()->EmplaceTypeRequest(this, returnTypeId, 0);
    }
    flags = static_cast<FunctionSymbolFlags>(reader.GetBinaryReader().ReadUShort());
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

void FunctionSymbol::ComputeName()
{
    std::u32string name;
    name.append(GroupName());
    name.append(1, U'(');
    int n = parameters.size();
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            name.append(U", ");
        }
        ParameterSymbol* parameter = parameters[i];
        name.append(parameter->GetType()->FullName());
        name.append(1, U' ');
        name.append(std::u32string(parameter->Name()));
    }
    name.append(1, U')');
    SetName(name);
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
    fullName.append(GroupName());
    fullName.append(1, U'(');
    int n = parameters.size();
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            fullName.append(U", ");
        }
        ParameterSymbol* parameter = parameters[i];
        fullName.append(parameter->GetType()->FullName());
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

void FunctionSymbol::AddLocalVariable(LocalVariableSymbol* localVariable)
{
    localVariables.push_back(localVariable);
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

} } // namespace cmajor::symbols
