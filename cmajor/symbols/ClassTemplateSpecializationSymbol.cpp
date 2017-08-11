// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/ClassTemplateSpecializationSymbol.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/SymbolTable.hpp>

namespace cmajor { namespace symbols {

std::u32string MakeClassTemplateSpecializationName(ClassTypeSymbol* classTemplate, const std::vector<TypeSymbol*>& templateArgumentTypes)
{
    std::u32string name = classTemplate->Name();
    name.append(1, '<');
    int n = templateArgumentTypes.size();
    for (int i = 0; i < n; ++i)
    {
        if (i > 0)
        {
            name.append(U", ");
        }
        name.append(templateArgumentTypes[i]->FullName());
    }
    name.append(1, '>');
    return name;
}

ClassTemplateSpecializationSymbol::ClassTemplateSpecializationSymbol(const Span& span_, const std::u32string& name_) : 
    ClassTypeSymbol(SymbolType::classTemplateSpecializationSymbol, span_, name_), classTemplate(nullptr), templateArgumentTypes()
{
}

ClassTemplateSpecializationSymbol::ClassTemplateSpecializationSymbol(const Span& span_, std::u32string& name_, ClassTypeSymbol* classTemplate_, const std::vector<TypeSymbol*>& templateArgumentTypes_) : 
    ClassTypeSymbol(SymbolType::classTemplateSpecializationSymbol, span_, name_), classTemplate(classTemplate_), templateArgumentTypes(templateArgumentTypes_)
{
}

std::u32string ClassTemplateSpecializationSymbol::SimpleName() const
{
    std::u32string simpleName = classTemplate->Name();
    int n = templateArgumentTypes.size();
    for (int i = 0; i < n; ++i)
    {
        simpleName.append(U"_").append(templateArgumentTypes[i]->SimpleName());
    }
    return simpleName;
}

void ClassTemplateSpecializationSymbol::Write(SymbolWriter& writer)
{
    ClassTypeSymbol::Write(writer);
    uint32_t classTemplateId = classTemplate->TypeId();
    writer.GetBinaryWriter().WriteEncodedUInt(classTemplateId);
    uint32_t n = templateArgumentTypes.size();
    writer.GetBinaryWriter().WriteEncodedUInt(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        TypeSymbol* templateArgumentType = templateArgumentTypes[i];
        uint32_t templateArgumentTypeId = templateArgumentType->TypeId();
        writer.GetBinaryWriter().WriteEncodedUInt(templateArgumentTypeId);
    }
}

void ClassTemplateSpecializationSymbol::Read(SymbolReader& reader)
{
    ClassTypeSymbol::Read(reader);
    uint32_t classTemplateId = reader.GetBinaryReader().ReadEncodedUInt();
    GetSymbolTable()->EmplaceTypeRequest(this, classTemplateId, -1);
    uint32_t n = reader.GetBinaryReader().ReadEncodedUInt();
    templateArgumentTypes.resize(n);
    for (uint32_t i = 0; i < n; ++i)
    {
        uint32_t typeArgumentId = reader.GetBinaryReader().ReadEncodedUInt();
        GetSymbolTable()->EmplaceTypeRequest(this, typeArgumentId, -2 - i);
    }
}

void ClassTemplateSpecializationSymbol::EmplaceType(TypeSymbol* typeSymbol, int index)
{
    if (index < 0)
    {
        if (index == -1)
        {
            Assert(typeSymbol->GetSymbolType() == SymbolType::classTypeSymbol, "class type symbol expected");
            classTemplate = static_cast<ClassTypeSymbol*>(typeSymbol);
        }
        else
        {
            int typeArgumentIndex = -(index + 2);
            if (typeArgumentIndex < 0 || typeArgumentIndex >= templateArgumentTypes.size())
            {
                Assert(false, "invalid emplace type index in class template specialization");
            }
            templateArgumentTypes[typeArgumentIndex] = typeSymbol;
        }
    }
    else
    {
        ClassTypeSymbol::EmplaceType(typeSymbol, index);
    }
}

void ClassTemplateSpecializationSymbol::SetGlobalNs(std::unique_ptr<Node>&& globalNs_)
{
    globalNs = std::move(globalNs_);
}

} } // namespace cmajor::symbols
