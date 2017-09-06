// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/ModuleBinder.hpp>
#include <cmajor/symbols/Scope.hpp>

namespace cmajor { namespace binder {

ModuleBinder::ModuleBinder(Module& module_, CompileUnitNode* compileUnitNode_) : 
    boundCompileUnit(module_, compileUnitNode_), containerScope(module_.GetSymbolTable().GlobalNs().GetContainerScope()), span(compileUnitNode_->GetSpan())
{
}

void ModuleBinder::BindClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization)
{
    boundCompileUnit.GetClassTemplateRepository().BindClassTemplateSpecialization(classTemplateSpecialization, containerScope, span);
}

} } // namespace cmajor::binder
