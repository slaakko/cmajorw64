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
    void ComputeExportClosure() override;
    bool IsPrototypeTemplateSpecialization() const override;
    ClassTypeSymbol* GetClassTemplate() { return classTemplate; }
    const std::vector<TypeSymbol*>& TemplateArgumentTypes() const { return templateArgumentTypes; }
    void SetGlobalNs(std::unique_ptr<Node>&& globalNs_);
    Node* GlobalNs() { return globalNs.get(); }
    void SetFileScope(FileScope* fileScope_);
    FileScope* ReleaseFileScope();
    void SetPrototype() { prototype = true; }
private:
    ClassTypeSymbol* classTemplate;
    std::vector<TypeSymbol*> templateArgumentTypes;
    std::unique_ptr<Node> globalNs;
    std::unique_ptr<FileScope> fileScope;
    bool prototype;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_CLASS_TEMPLATE_SPECIALIZATION_SYMBOL_INCLUDED
