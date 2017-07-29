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
    vmtInitialized = 1 << 2,
    imtsInitialized = 1 << 3,
    classObjectLayoutComputed = 1 << 4,
    dontCreateDefaultConstructor = 1 << 5
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
    bool HasNontrivialDestructor() const override;
    void CreateDestructorSymbol();
    const ClassTypeSymbol* BaseClass() const { return baseClass; }
    ClassTypeSymbol* BaseClass() { return baseClass; }
    void SetBaseClass(ClassTypeSymbol* baseClass_) { baseClass = baseClass_; }
    bool HasBaseClass(ClassTypeSymbol* cls) const;
    bool HasBaseClass(ClassTypeSymbol* cls, uint8_t& distance) const;
    const std::vector<InterfaceTypeSymbol*>& ImplementedInterfaces() const { return implementedInterfaces; }
    void AddImplementedInterface(InterfaceTypeSymbol* interfaceTypeSymbol);
    bool IsClassTemplate() const { return !templateParameters.empty(); }
    void CloneUsingNodes(const std::vector<Node*>& usingNodes_);
    void SetSpecifiers(Specifiers specifiers);
    StaticConstructorSymbol* StaticConstructor() { return staticConstructor; }
    ConstructorSymbol* DefaultConstructor() { return defaultConstructor; }
    void SetDefaultConstructor(ConstructorSymbol* defaultConstructor_) { defaultConstructor = defaultConstructor_; }
    DestructorSymbol* Destructor() { return destructor; }
    const std::vector<MemberVariableSymbol*>& MemberVariables() const { return memberVariables; }
    bool IsAbstract() const { return GetFlag(ClassTypeSymbolFlags::abstract_); }
    void SetAbstract() { SetFlag(ClassTypeSymbolFlags::abstract_); }
    bool IsPolymorphic() const { return GetFlag(ClassTypeSymbolFlags::polymorphic); }
    void SetPolymorphic() { SetFlag(ClassTypeSymbolFlags::polymorphic); }
    bool IsVmtInitialized() const { return GetFlag(ClassTypeSymbolFlags::vmtInitialized);  }
    void SetVmtInitialized() { SetFlag(ClassTypeSymbolFlags::vmtInitialized); }
    bool IsImtsInitialized() const { return GetFlag(ClassTypeSymbolFlags::imtsInitialized); }
    void SetImtsInitialized() { SetFlag(ClassTypeSymbolFlags::imtsInitialized); }
    bool IsClassObjectLayoutComputed() const { return GetFlag(ClassTypeSymbolFlags::classObjectLayoutComputed); }
    void SetClassObjectLayoutComputed() { SetFlag(ClassTypeSymbolFlags::classObjectLayoutComputed); }
    bool DontCreateDefaultConstructor() const { return GetFlag(ClassTypeSymbolFlags::dontCreateDefaultConstructor); }
    void SetDontCreateDefaultConstructor() { SetFlag(ClassTypeSymbolFlags::dontCreateDefaultConstructor); }
    ClassTypeSymbolFlags GetClassTypeSymbolFlags() const { return flags; }
    bool GetFlag(ClassTypeSymbolFlags flag) const { return (flags & flag) != ClassTypeSymbolFlags::none; }
    void SetFlag(ClassTypeSymbolFlags flag) { flags = flags | flag; }
    void InitVmt();
    void InitImts();
    void CreateObjectLayout();
    llvm::Type* IrType(Emitter& emitter) override;
private:
    ClassTypeSymbol* baseClass;
    ClassTypeSymbolFlags flags;
    std::vector<InterfaceTypeSymbol*> implementedInterfaces;
    std::vector<TemplateParameterSymbol*> templateParameters;
    std::vector<MemberVariableSymbol*> memberVariables;
    std::vector<MemberVariableSymbol*> staticMemberVariables;
    StaticConstructorSymbol* staticConstructor;
    std::vector<ConstructorSymbol*> constructors;
    ConstructorSymbol* defaultConstructor;
    DestructorSymbol* destructor;
    std::vector<MemberFunctionSymbol*> memberFunctions;
    std::vector<FunctionSymbol*> vmt;
    std::vector<std::vector<FunctionSymbol*>> imts;
    std::vector<TypeSymbol*> objectLayout;
    llvm::Type* irType;
    int32_t vptrIndex;
    NodeList<Node> usingNodes;
    void InitVmt(std::vector<FunctionSymbol*>& vmtToInit);
};

class TemplateParameterSymbol : public TypeSymbol
{
public:
    TemplateParameterSymbol(const Span& span_, const std::u32string& name_);
    llvm::Type* IrType(Emitter& emitter) override { return nullptr; } 
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
