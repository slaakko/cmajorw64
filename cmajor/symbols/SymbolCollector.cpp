// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/SymbolCollector.hpp>
#include <algorithm>

namespace cmajor { namespace symbols {

SymbolCollector::SymbolCollector() 
{
}

void SymbolCollector::AddBasicType(BasicTypeSymbol* basicType)
{
    basicTypes.push_back(basicType);
}

void SymbolCollector::AddClass(ClassTypeSymbol* class_)
{
    classes.push_back(class_);
}

void SymbolCollector::AddConcept(ConceptSymbol* concept)
{
    concepts.push_back(concept);
}

void SymbolCollector::AddConstant(ConstantSymbol* constant)
{
    constants.push_back(constant);
}

void SymbolCollector::AddDelegate(DelegateTypeSymbol* delegate)
{
    delegates.push_back(delegate);
}

void SymbolCollector::AddClassDelegate(ClassDelegateTypeSymbol* classDelegate)
{
    classDelegates.push_back(classDelegate);
}

void SymbolCollector::AddEnumeratedType(EnumTypeSymbol* enumType)
{
    enumeratedTypes.push_back(enumType);
}

void SymbolCollector::AddEnumerationConstant(EnumConstantSymbol* enumConstant)
{
    enumerationConstants.push_back(enumConstant);
}

void SymbolCollector::AddFunction(FunctionSymbol* function)
{
    functions.push_back(function);
}

void SymbolCollector::AddInterface(InterfaceTypeSymbol* interface_)
{
    interfaces.push_back(interface_);
}

void SymbolCollector::AddTypedef(TypedefSymbol* typedef_)
{
    typedefs.push_back(typedef_);
}

void SymbolCollector::AddMemberVariable(MemberVariableSymbol* memberVariable)
{
    memberVariables.push_back(memberVariable);
}

struct ByFullName
{
    bool operator()(Symbol* left, Symbol* right) const
    {
        return left->FullName() < right->FullName();
    }
};

void SymbolCollector::Sort()
{
    std::sort(basicTypes.begin(), basicTypes.end(), ByFullName());
    std::sort(classes.begin(), classes.end(), ByFullName());
    std::sort(concepts.begin(), concepts.end(), ByFullName());
    std::sort(constants.begin(), constants.end(), ByFullName());
    std::sort(delegates.begin(), delegates.end(), ByFullName());
    std::sort(classDelegates.begin(), classDelegates.end(), ByFullName());
    std::sort(enumeratedTypes.begin(), enumeratedTypes.end(), ByFullName());
    std::sort(functions.begin(), functions.end(), ByFullName());
    std::sort(interfaces.begin(), interfaces.end(), ByFullName());
    std::sort(typedefs.begin(), typedefs.end(), ByFullName());
    std::sort(memberVariables.begin(), memberVariables.end(), ByFullName());
}

} } // namespace cmajor::symbols
