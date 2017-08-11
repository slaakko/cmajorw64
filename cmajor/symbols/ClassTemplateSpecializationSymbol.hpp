// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_CLASS_TEMPLATE_SPECIALIZATION_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_CLASS_TEMPLATE_SPECIALIZATION_SYMBOL_INCLUDED
#include <cmajor/symbols/ClassTypeSymbol.hpp>

namespace cmajor { namespace symbols {

std::u32string MakeClassTemplateSpecializationName(ClassTypeSymbol* classTemplate, const std::vector<TypeSymbol*>& templateArgumentTypes);

class ClassTemplateSpecializationSymbol : public ClassTypeSymbol
{
public:
    ClassTemplateSpecializationSymbol(const Span& span_, const std::u32string& name_);
    ClassTemplateSpecializationSymbol(const Span& span_, std::u32string& name_, ClassTypeSymbol* classTemplate_, const std::vector<TypeSymbol*>& templateArgumentTypes_);
    std::u32string SimpleName() const override;
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol, int index) override;
    ClassTypeSymbol* GetClassTemplate() { return classTemplate; }
    void SetGlobalNs(std::unique_ptr<Node>&& globalNs_);
    const std::vector<TypeSymbol*>& TemplateArgumentTypes() const { return templateArgumentTypes; }
    Node* GlobalNs() { return globalNs.get(); }
private:
    ClassTypeSymbol* classTemplate;
    std::vector<TypeSymbol*> templateArgumentTypes;
    std::unique_ptr<Node> globalNs;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_CLASS_TEMPLATE_SPECIALIZATION_SYMBOL_INCLUDED
