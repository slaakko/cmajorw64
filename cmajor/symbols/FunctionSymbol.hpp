// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/ir/GenObject.hpp>
#include <unordered_set>

namespace cmajor {  namespace symbols {

using namespace cmajor::ir;

class FunctionGroupSymbol : public Symbol
{
public:
    FunctionGroupSymbol(const Span& span_, const std::u32string& name_);
    bool IsExportSymbol() const override { return false; }
    std::string TypeString() const override { return "function_group"; }
    void AddFunction(FunctionSymbol* function);
    void CollectViableFunctions(int arity, std::unordered_set<FunctionSymbol*>& viableFunctions);
private:
    std::unordered_map<int, std::vector<FunctionSymbol*>> arityFunctionListMap;
};

enum class ConversionType : uint8_t
{
    implicit_, explicit_
};

enum class FunctionSymbolFlags : uint16_t
{
    none = 0,
    inline_ = 1 << 0,
    external_ = 1 << 1,
    constExpr = 1 << 2,
    cdecl_ = 1 << 3,
    suppress = 1 << 4,
    default_ = 1 << 5,
    explicit_ = 1 << 6,
    virtual_ = 1 << 7,
    override_ = 1 << 8,
    abstract_ = 1 << 9,
    new_ = 1 << 10,
    const_ = 1 << 11,
    conversion = 1 << 12
};

inline FunctionSymbolFlags operator|(FunctionSymbolFlags left, FunctionSymbolFlags right)
{
    return FunctionSymbolFlags(uint16_t(left) | uint16_t(right));
}

inline FunctionSymbolFlags operator&(FunctionSymbolFlags left, FunctionSymbolFlags right)
{
    return FunctionSymbolFlags(uint16_t(left) & uint16_t(right));
}

std::string FunctionSymbolFlagStr(FunctionSymbolFlags flags);

class ParameterSymbol;
class LocalVariableSymbol;
class TypeSymbol;
class TemplateParameterSymbol;

class FunctionSymbol : public ContainerSymbol
{
public:
    FunctionSymbol(const Span& span_, const std::u32string& name_);
    FunctionSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol_, int index) override;
    void AddMember(Symbol* member) override;
    bool IsFunctionSymbol() const override { return true; }
    std::string TypeString() const override { return "function"; }
    virtual void ComputeName();
    std::u32string FullName() const override;
    std::u32string FullNameWithSpecifiers() const override;
    virtual ConversionType GetConversionType() const { return ConversionType::implicit_; }
    virtual uint8_t ConversionDistance() const { return 0; }
    virtual TypeSymbol* ConversionSourceType() const { return nullptr; }
    virtual TypeSymbol* ConversionTargetType() const { return nullptr; }
    virtual bool IsBasicTypeOperation() const { return false; }
    virtual void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects);
    virtual ParameterSymbol* GetThisParam() const { return nullptr; }
    const std::u32string& GroupName() const { return groupName; }
    void SetGroupName(const std::u32string& groupName_);
    void SetSpecifiers(Specifiers specifiers);
    bool IsInline() const { return GetFlag(FunctionSymbolFlags::inline_); }
    void SetInline() { SetFlag(FunctionSymbolFlags::inline_); }
    bool IsExternal() const { return GetFlag(FunctionSymbolFlags::external_); }
    void SetExternal() { SetFlag(FunctionSymbolFlags::external_); }
    bool IsConstExpr() const { return GetFlag(FunctionSymbolFlags::constExpr); }
    void SetConstExpr() { SetFlag(FunctionSymbolFlags::constExpr); }
    bool IsCDecl() const { return GetFlag(FunctionSymbolFlags::cdecl_); }
    void SetCDecl() { SetFlag(FunctionSymbolFlags::cdecl_); }
    bool IsDefault() const { return GetFlag(FunctionSymbolFlags::default_); }
    void SetDefault() { SetFlag(FunctionSymbolFlags::default_); }
    bool IsSuppressed() const { return GetFlag(FunctionSymbolFlags::suppress); }
    void SetSuppressed() { SetFlag(FunctionSymbolFlags::suppress); }
    bool IsExplicit() const { return GetFlag(FunctionSymbolFlags::explicit_); }
    void SetExplicit() { SetFlag(FunctionSymbolFlags::explicit_); }
    bool IsVirtual() const { return GetFlag(FunctionSymbolFlags::virtual_); }
    void SetVirtual() { SetFlag(FunctionSymbolFlags::virtual_); }
    bool IsOverride() const { return GetFlag(FunctionSymbolFlags::override_); }
    void SetOverride() { SetFlag(FunctionSymbolFlags::override_); }
    bool IsAbstract() const { return GetFlag(FunctionSymbolFlags::abstract_); }
    void SetAbstract() { SetFlag(FunctionSymbolFlags::abstract_); }
    bool IsNew() const { return GetFlag(FunctionSymbolFlags::new_); }
    void SetNew() { SetFlag(FunctionSymbolFlags::new_); }
    bool IsConst() const { return GetFlag(FunctionSymbolFlags::const_); }
    void SetConst() { SetFlag(FunctionSymbolFlags::const_); }
    bool IsConversion() const { return GetFlag(FunctionSymbolFlags::conversion); }
    void SetConversion() { SetFlag(FunctionSymbolFlags::conversion); }
    FunctionSymbolFlags GetFunctionSymbolFlags() const { return flags; }
    bool GetFlag(FunctionSymbolFlags flag) const { return (flags & flag) != FunctionSymbolFlags::none; }
    void SetFlag(FunctionSymbolFlags flag) { flags = flags | flag; }
    void ComputeMangledName();
    int Arity() const { return parameters.size(); }
    const std::vector<ParameterSymbol*>& Parameters() const { return parameters; }
    void AddLocalVariable(LocalVariableSymbol* localVariable);
    const std::vector<LocalVariableSymbol*>& LocalVariables() const { return localVariables; }
    void SetReturnType(TypeSymbol* returnType_) { returnType = returnType_; }
    TypeSymbol* ReturnType() const { return returnType; }
    bool IsFunctionTemplate() const { return !templateParameters.empty(); }
    void CloneUsingNodes(const std::vector<Node*>& usingNodes_);
    LocalVariableSymbol* CreateTemporary(TypeSymbol* type, const Span& span);
    llvm::FunctionType* IrType(Emitter& emitter);
private:
    std::u32string groupName;
    std::vector<TemplateParameterSymbol*> templateParameters;
    std::vector<ParameterSymbol*> parameters;
    std::vector<LocalVariableSymbol*> localVariables;
    TypeSymbol* returnType;
    FunctionSymbolFlags flags;
    NodeList<Node> usingNodes;
    llvm::FunctionType* irType;
    int nextTemporaryIndex;
};

class StaticConstructorSymbol : public FunctionSymbol
{
public:
    StaticConstructorSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "static_constructor"; }
    void SetSpecifiers(Specifiers specifiers);
};

class ConstructorSymbol : public FunctionSymbol
{
public:
    ConstructorSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "constructor"; }
    ParameterSymbol* GetThisParam() const override { return Parameters()[0]; }
    void SetSpecifiers(Specifiers specifiers);
};

class DestructorSymbol : public FunctionSymbol
{
public:
    DestructorSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "destructor"; }
    ParameterSymbol* GetThisParam() const override { return Parameters()[0]; }
    void SetSpecifiers(Specifiers specifiers);
};

class MemberFunctionSymbol : public FunctionSymbol
{
public:
    MemberFunctionSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "member_function"; }
    ParameterSymbol* GetThisParam() const override { if (IsStatic()) return nullptr; else return Parameters()[0]; }
    void SetSpecifiers(Specifiers specifiers);
};

class FunctionGroupTypeSymbol : public TypeSymbol
{
public:
    FunctionGroupTypeSymbol(FunctionGroupSymbol* functionGroup_);
    const FunctionGroupSymbol* FunctionGroup() const { return functionGroup; }
    llvm::Type* IrType(Emitter& emitter) override { return nullptr; } 
private:
    FunctionGroupSymbol* functionGroup;
};

void InitFunctionSymbol();
void DoneFunctionSymbol();

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_FUNCTION_SYMBOL_INCLUDED
