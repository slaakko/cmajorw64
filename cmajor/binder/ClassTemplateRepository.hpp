// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_CLASS_TEMPLATE_REPOSITORY_INCLUDED
#define CMAJOR_BINDER_CLASS_TEMPLATE_REPOSITORY_INCLUDED
#include <cmajor/symbols/ClassTemplateSpecializationSymbol.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::symbols;

class BoundCompileUnit;

class ClassTemplateRepository
{
public:
    ClassTemplateRepository(BoundCompileUnit& boundCompileUnit_);
    void ResolveDefaultTemplateArguments(std::vector<TypeSymbol*>& templateArgumentTypes, ClassTypeSymbol* classTemplate, ContainerScope* containerScope, const Span& span);
    void BindClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization, ContainerScope* containerScope, const Span& span);
    void Instantiate(FunctionSymbol* memberFunction, ContainerScope* containerScope, const Span& span);
private:
    BoundCompileUnit& boundCompileUnit;
    std::unordered_set<FunctionSymbol*> instantiatedMemberFunctions;
    void InstantiateDestructorAndVirtualFunctions(ClassTemplateSpecializationSymbol* classTemplateSpecialization, ContainerScope* containerScope, const Span& span);
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_CLASS_TEMPLATE_REPOSITORY_INCLUDED
