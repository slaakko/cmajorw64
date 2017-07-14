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

namespace cmajor { namespace symbols {

ClassTypeSymbol::ClassTypeSymbol(const Span& span_, const std::u32string& name_) : 
    TypeSymbol(SymbolType::classTypeSymbol, span_, name_), 
    baseClass(), implementedInterfaces(), templateParameters(), memberVariables(), staticMemberVariables(), staticConstructor(), constructors(), destructor(), memberFunctions()
{
}

ClassTypeSymbol::ClassTypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) :
    TypeSymbol(symbolType_, span_, name_),
    baseClass(), implementedInterfaces(), templateParameters(), memberVariables(), staticMemberVariables(), staticConstructor(), constructors(), destructor(), memberFunctions()
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
            constructors.push_back(static_cast<ConstructorSymbol*>(member));
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

bool ClassTypeSymbol::HasBaseClass(ClassTypeSymbol* cls) const
{
    if (!baseClass) return false;
    if (baseClass == cls || baseClass->HasBaseClass(cls)) return true;
    return false;
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

TemplateParameterSymbol::TemplateParameterSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::templateParameterSymbol, span_, name_)
{
}

BoundTemplateParameterSymbol::BoundTemplateParameterSymbol(const Span& span_, const std::u32string& name_) : Symbol(SymbolType::boundTemplateParameterSymbol, span_, name_)
{

}

} } // namespace cmajor::symbols