// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/SymbolCollector.hpp>
//#include <cmajor/parser/FileRegistry.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

VariableSymbol::VariableSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) : Symbol(symbolType_, span_, name_), type()
{
}

void VariableSymbol::Write(SymbolWriter& writer)
{
    Symbol::Write(writer);
    writer.GetBinaryWriter().Write(type->TypeId());
}

void VariableSymbol::Read(SymbolReader& reader)
{
    Symbol::Read(reader);
    //uint32_t typeId = reader.GetBinaryReader().ReadUInt();
    boost::uuids::uuid typeId;
    reader.GetBinaryReader().ReadUuid(typeId);
    GetSymbolTable()->EmplaceTypeRequest(this, typeId, 0);
}

void VariableSymbol::EmplaceType(TypeSymbol* typeSymbol, int index)
{
    Assert(index == 0, "invalid emplace type index");
    type = typeSymbol;
}

ParameterSymbol::ParameterSymbol(const Span& span_, const std::u32string& name_) : VariableSymbol(SymbolType::parameterSymbol, span_, name_)
{
}

void ParameterSymbol::ComputeExportClosure()
{
    if (IsProject())
    {
        if (!GetType()->ExportComputed())
        {
            GetType()->SetExportComputed();
            GetType()->ComputeExportClosure();
        }
    }
}

LocalVariableSymbol::LocalVariableSymbol(const Span& span_, const std::u32string& name_) : VariableSymbol(SymbolType::localVariableSymbol, span_, name_)
{
}

MemberVariableSymbol::MemberVariableSymbol(const Span& span_, const std::u32string& name_) : VariableSymbol(SymbolType::memberVariableSymbol, span_, name_), layoutIndex(-1), compileUnitIndex(-1), diMemberType(nullptr)
{
}

void MemberVariableSymbol::Write(SymbolWriter& writer)
{
    VariableSymbol::Write(writer);
    writer.GetBinaryWriter().Write(layoutIndex);
}

void MemberVariableSymbol::Read(SymbolReader& reader)
{
    VariableSymbol::Read(reader);
    layoutIndex = reader.GetBinaryReader().ReadInt();
}

bool MemberVariableSymbol::IsExportSymbol() const
{
    if (Parent()->GetSymbolType() == SymbolType::classTemplateSpecializationSymbol) return false;
    return VariableSymbol::IsExportSymbol();
}

void MemberVariableSymbol::ComputeExportClosure()
{
    if (IsProject())
    {
        if (!GetType()->ExportComputed())
        {
            GetType()->SetExportComputed();
            GetType()->ComputeExportClosure();
        }
    }
}

void MemberVariableSymbol::Accept(SymbolCollector* collector)
{
    if (IsProject() && Access() == SymbolAccess::public_)
    {
        collector->AddMemberVariable(this);
    }
}

void MemberVariableSymbol::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine(ToUtf8(Name()));
    formatter.WriteLine("full name: " + ToUtf8(FullNameWithSpecifiers()));
    formatter.WriteLine("mangled name: " + ToUtf8(MangledName()));
    formatter.WriteLine("type: " + ToUtf8(GetType()->FullName()));
    formatter.WriteLine("layout index: " + std::to_string(layoutIndex));
}

std::string MemberVariableSymbol::Syntax() const
{
    std::string syntax = GetSpecifierStr();
    if (!syntax.empty())
    {
        syntax.append(1, ' ');
    }
    syntax.append(ToUtf8(GetType()->DocName()));
    syntax.append(1, ' ');
    syntax.append(ToUtf8(DocName()));
    syntax.append(1, ';');
    return syntax;
}

void MemberVariableSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        SetStatic();
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be default", GetSpan());
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be constexpr", GetSpan());
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be nothrow", GetSpan());
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be throw", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception(GetModule(), "member variable cannot be unit_test", GetSpan());
    }
}

llvm::DIDerivedType* MemberVariableSymbol::GetDIMemberType(Emitter& emitter, uint64_t offsetInBits)
{
    if (!diMemberType || compileUnitIndex != emitter.CompileUnitIndex())
    {
        uint64_t sizeInBits = GetType()->SizeInBits(emitter);
        uint32_t alignInBits = GetType()->AlignmentInBits(emitter);
        llvm::DINode::DIFlags flags = llvm::DINode::DIFlags::FlagZero;
        llvm::DIType* scope = nullptr;
        Symbol* parent = Parent();
        if (parent->IsClassTypeSymbol())
        {
            ClassTypeSymbol* cls = static_cast<ClassTypeSymbol*>(parent);
            scope = cls->GetDIType(emitter);
        }
        diMemberType = emitter.DIBuilder()->createMemberType(scope, ToUtf8(Name()), emitter.GetFile(GetSpan().FileIndex()), GetSpan().LineNumber(), sizeInBits, alignInBits, offsetInBits, flags, 
            GetType()->GetDIType(emitter));
        compileUnitIndex = emitter.CompileUnitIndex();
    }
    return diMemberType;
}

} } // namespace cmajor::symbols
