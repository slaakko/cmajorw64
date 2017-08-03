// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_CLASS_TYPE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_CLASS_TYPE_SYMBOL_INCLUDED
#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/ast/Class.hpp>

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
    vmtObjectCreated = 1 << 5,
    dontCreateDefaultConstructor = 1 << 6
};

inline ClassTypeSymbolFlags operator|(ClassTypeSymbolFlags left, ClassTypeSymbolFlags right)
{
    return ClassTypeSymbolFlags(uint8_t(left) | uint8_t(right));
}

inline ClassTypeSymbolFlags operator&(ClassTypeSymbolFlags left, ClassTypeSymbolFlags right)
{
    return ClassTypeSymbolFlags(uint8_t(left) & uint8_t(right));
}

const int32_t classIdVmtIndexOffset = 0;
const int32_t classNameVmtIndexOffset = 1;
const int32_t imtsVmtIndexOffset = 2;
const int32_t functionVmtIndexOffset = 3;

class ClassTypeSymbol : public TypeSymbol
{
public:
    ClassTypeSymbol(const Span& span_, const std::u32string& name_);
    ClassTypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void ReadAstNodes();
    ClassNode* GetFunctionNode() { return classNode.get(); }
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
    const std::vector<TemplateParameterSymbol*>& TemplateParameters() const { return templateParameters; }
    bool IsClassTemplate() const { return !templateParameters.empty(); }
    void CloneUsingNodes(const std::vector<Node*>& usingNodes_);
    void SetSpecifiers(Specifiers specifiers);
    StaticConstructorSymbol* StaticConstructor() { return staticConstructor; }
    ConstructorSymbol* DefaultConstructor() { return defaultConstructor; }
    void SetDefaultConstructor(ConstructorSymbol* defaultConstructor_) { defaultConstructor = defaultConstructor_; }
    ConstructorSymbol* CopyConstructor() { return copyConstructor; }
    void SetCopyConstructor(ConstructorSymbol* copyConstructor_) { copyConstructor = copyConstructor_; }
    ConstructorSymbol* MoveConstructor() { return moveConstructor; }
    void SetMoveConstructor(ConstructorSymbol* moveConstructor_) { moveConstructor = moveConstructor_; }
    DestructorSymbol* Destructor() { return destructor; }
    MemberFunctionSymbol* CopyAssignment() { return copyAssignment; }
    void SetCopyAssignment(MemberFunctionSymbol* copyAssignment_) { copyAssignment = copyAssignment_; }
    MemberFunctionSymbol* MoveAssignment() { return moveAssignment; }
    void SetMoveAssignment(MemberFunctionSymbol* moveAssignment_) { moveAssignment = moveAssignment_; }
    void SetSpecialMemberFunctions();
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
    bool IsVmtObjectCreated() const { return GetFlag(ClassTypeSymbolFlags::vmtObjectCreated); }
    void SetVmtObjectCreated() { SetFlag(ClassTypeSymbolFlags::vmtObjectCreated); }
    bool DontCreateDefaultConstructor() const { return GetFlag(ClassTypeSymbolFlags::dontCreateDefaultConstructor); }
    void SetDontCreateDefaultConstructor() { SetFlag(ClassTypeSymbolFlags::dontCreateDefaultConstructor); }
    ClassTypeSymbolFlags GetClassTypeSymbolFlags() const { return flags; }
    bool GetFlag(ClassTypeSymbolFlags flag) const { return (flags & flag) != ClassTypeSymbolFlags::none; }
    void SetFlag(ClassTypeSymbolFlags flag) { flags = flags | flag; }
    void InitVmt();
    void InitImts();
    void CreateObjectLayout();
    llvm::Type* IrType(Emitter& emitter) override;
    llvm::Value* VmtObject(Emitter& emitter, bool create);
    llvm::Type* VmtPtrType(Emitter& emitter);
    const std::string& VmtObjectName();
    int32_t VmtPtrIndex() const { return vmtPtrIndex; }
    ClassTypeSymbol* VmtPtrHolderClass();
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
    ConstructorSymbol* copyConstructor;
    ConstructorSymbol* moveConstructor;
    DestructorSymbol* destructor;
    MemberFunctionSymbol* copyAssignment;
    MemberFunctionSymbol* moveAssignment;
    std::vector<MemberFunctionSymbol*> memberFunctions;
    std::vector<FunctionSymbol*> vmt;
    std::vector<std::vector<FunctionSymbol*>> imts;
    std::vector<TypeSymbol*> objectLayout;
    llvm::Type* irType;
    llvm::ArrayType* vmtObjectType;
    int32_t vmtPtrIndex;
    NodeList<Node> usingNodes;
    std::unique_ptr<ClassNode> classNode;
    uint32_t sizeOfAstNodes;
    uint32_t astNodesPos;
    std::string filePathReadFrom;
    std::string vmtObjectName;
    void InitVmt(std::vector<FunctionSymbol*>& vmtToInit);
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_CLASS_TYPE_SYMBOL_INCLUDED
