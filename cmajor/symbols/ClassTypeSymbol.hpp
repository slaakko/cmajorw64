// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_CLASS_TYPE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_CLASS_TYPE_SYMBOL_INCLUDED
#include <cmajor/symbols/TypeSymbol.hpp>

namespace cmajor { namespace symbols {

class InterfaceTypeSymbol;
class TemplateParameterSymbol;
class MemberVariableSymbol;
class StaticConstructorSymbol;
class ConstructorSymbol;
class DestructorSymbol;
class MemberFunctionSymbol;

enum class ClassTypeSymbolFlags : uint8_t
{
    none = 0,
    abstract_ = 1 << 0,
    polymorphic = 1 << 1,
    dontCreateDefaultConstructor = 1 << 2
};

inline ClassTypeSymbolFlags operator|(ClassTypeSymbolFlags left, ClassTypeSymbolFlags right)
{
    return ClassTypeSymbolFlags(uint8_t(left) | uint8_t(right));
}

inline ClassTypeSymbolFlags operator&(ClassTypeSymbolFlags left, ClassTypeSymbolFlags right)
{
    return ClassTypeSymbolFlags(uint8_t(left) & uint8_t(right));
}

class ClassTypeSymbol : public TypeSymbol
{
public:
    ClassTypeSymbol(const Span& span_, const std::u32string& name_);
    ClassTypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol_, int index) override;
    void AddMember(Symbol* member) override;
    bool IsClassTypeSymbol() const override { return true; }
    std::string TypeString() const override { return "class"; }
    const ClassTypeSymbol* BaseClass() const { return baseClass; }
    ClassTypeSymbol* BaseClass() { return baseClass; }
    void SetBaseClass(ClassTypeSymbol* baseClass_) { baseClass = baseClass_; }
    bool HasBaseClass(ClassTypeSymbol* cls) const;
    const std::vector<InterfaceTypeSymbol*>& ImplementedInterfaces() const { return implementedInterfaces; }
    void AddImplementedInterface(InterfaceTypeSymbol* interfaceTypeSymbol);
    bool IsClassTemplate() const { return !templateParameters.empty(); }
    void CloneUsingNodes(const std::vector<Node*>& usingNodes_);
    void SetSpecifiers(Specifiers specifiers);
    bool IsAbstract() const { return GetFlag(ClassTypeSymbolFlags::abstract_); }
    void SetAbstract() { SetFlag(ClassTypeSymbolFlags::abstract_); }
    bool IsPolymorphic() const { return GetFlag(ClassTypeSymbolFlags::polymorphic); }
    void SetPolymorphic() { SetFlag(ClassTypeSymbolFlags::polymorphic); }
    bool DontCreateDefaultConstructor() const { return GetFlag(ClassTypeSymbolFlags::dontCreateDefaultConstructor); }
    void SetDontCreateDefaultConstructor() { SetFlag(ClassTypeSymbolFlags::dontCreateDefaultConstructor); }
    ClassTypeSymbolFlags GetClassTypeSymbolFlags() const { return flags; }
    bool GetFlag(ClassTypeSymbolFlags flag) const { return (flags & flag) != ClassTypeSymbolFlags::none; }
    void SetFlag(ClassTypeSymbolFlags flag) { flags = flags | flag; }
    llvm::Type* IrType(Emitter& emitter) const override { return nullptr; } // todo
private:
    ClassTypeSymbol* baseClass;
    ClassTypeSymbolFlags flags;
    std::vector<InterfaceTypeSymbol*> implementedInterfaces;
    std::vector<TemplateParameterSymbol*> templateParameters;
    std::vector<MemberVariableSymbol*> memberVariables;
    std::vector<MemberVariableSymbol*> staticMemberVariables;
    StaticConstructorSymbol* staticConstructor;
    std::vector<ConstructorSymbol*> constructors;
    DestructorSymbol* destructor;
    std::vector<MemberFunctionSymbol*> memberFunctions;
    NodeList<Node> usingNodes;
};

class TemplateParameterSymbol : public TypeSymbol
{
public:
    TemplateParameterSymbol(const Span& span_, const std::u32string& name_);
    llvm::Type* IrType(Emitter& emitter) const override { return nullptr; } 
};

class BoundTemplateParameterSymbol : public Symbol
{
public:
    BoundTemplateParameterSymbol(const Span& span_, const std::u32string& name_);
    bool IsBoundTemplateParameterSymbol() const override { return true; }
    TypeSymbol* GetType() const { return type; }
    void SetType(TypeSymbol* type_) { type = type_; }
private:
    TypeSymbol* type;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_CLASS_TYPE_SYMBOL_INCLUDED
