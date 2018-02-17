// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/ModuleBinder.hpp>
#include <cmajor/symbols/Scope.hpp>

namespace cmajor { namespace binder {

ModuleBinder::ModuleBinder(Module& module_, CompileUnitNode* compileUnitNode_, AttributeBinder* attributeBinder_) : 
    boundCompileUnit(module_, compileUnitNode_, attributeBinder_), containerScope(module_.GetSymbolTable().GlobalNs().GetContainerScope()), span(compileUnitNode_->GetSpan())
{
}

void ModuleBinder::SetBindingTypes()
{
    boundCompileUnit.PushBindingTypes();
}

void ModuleBinder::BindClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization)
{
    boundCompileUnit.GetClassTemplateRepository().BindClassTemplateSpecialization(classTemplateSpecialization, containerScope, span);
}

} } // namespace cmajor::binder
