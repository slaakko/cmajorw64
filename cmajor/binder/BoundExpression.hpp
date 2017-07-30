// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_BOUND_EXPRESSION_INCLUDED
#define CMAJOR_BINDER_BOUND_EXPRESSION_INCLUDED
#include <cmajor/binder/BoundNode.hpp>
#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/ConstantSymbol.hpp>
#include <cmajor/symbols/EnumSymbol.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::symbols;

enum class BoundExpressionFlags : uint8_t
{
    none = 0, 
    argIsExplicitThisOrBasePtr = 1 << 0,
    virtualCall = 1 << 1
};

inline BoundExpressionFlags operator|(BoundExpressionFlags left, BoundExpressionFlags right)
{
    return BoundExpressionFlags(uint8_t(left) | uint8_t(right));
}

inline BoundExpressionFlags operator&(BoundExpressionFlags left, BoundExpressionFlags right)
{
    return BoundExpressionFlags(uint8_t(left) & uint8_t(right));
}

class BoundExpression : public BoundNode
{
public:
    BoundExpression(const Span& span_, BoundNodeType boundNodeType_, TypeSymbol* type_);
    virtual bool IsComplete() const { return true; }
    virtual bool IsLvalueExpression() const { return false; }
    virtual bool HasValue() const { return false; }
    virtual std::string TypeString() const { return "expression"; }
    const TypeSymbol* GetType() const { return type; }
    TypeSymbol* GetType() { return type; }
    bool GetFlag(BoundExpressionFlags flag) const { return (flags & flag) != BoundExpressionFlags::none;  }
    void SetFlag(BoundExpressionFlags flag) { flags = flags | flag; }
private:
    TypeSymbol* type;
    BoundExpressionFlags flags;
};

class BoundParameter : public BoundExpression
{
public:
    BoundParameter(ParameterSymbol* parameterSymbol_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    bool IsLvalueExpression() const override { return true; }
    std::string TypeString() const override { return "parameter"; }
private:
    ParameterSymbol* parameterSymbol;
};

class BoundLocalVariable : public BoundExpression
{
public:
    BoundLocalVariable(LocalVariableSymbol* localVariableSymbol_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    bool IsLvalueExpression() const override { return true; }
    std::string TypeString() const override { return "local variable"; }
private:
    LocalVariableSymbol* localVariableSymbol;
};

class BoundMemberVariable : public BoundExpression
{
public:
    BoundMemberVariable(MemberVariableSymbol* memberVariableSymbol_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    bool IsLvalueExpression() const override { return true; }
    void SetClassPtr(std::unique_ptr<BoundExpression>&& classPtr_);
    void SetStaticInitNeeded() { staticInitNeeded = true; }
    std::string TypeString() const override { return "member variable"; }
    MemberVariableSymbol* GetMemberVariableSymbol() { return memberVariableSymbol; }
private:
    MemberVariableSymbol* memberVariableSymbol;
    std::unique_ptr<BoundExpression> classPtr;
    bool staticInitNeeded;
};

class BoundConstant : public BoundExpression
{
public:
    BoundConstant(ConstantSymbol* constantSymbol_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    std::string TypeString() const override { return "constant"; }
private:
    ConstantSymbol* constantSymbol;
};

class BoundEnumConstant : public BoundExpression
{
public:
    BoundEnumConstant(EnumConstantSymbol* enumConstantSymbol_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    std::string TypeString() const override { return "enumeration constant"; }
private:
    EnumConstantSymbol* enumConstantSymbol;
};

class BoundLiteral : public BoundExpression
{
public:
    BoundLiteral(std::unique_ptr<Value>&& value_, TypeSymbol* type_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    std::string TypeString() const override { return "literal"; }
    bool HasValue() const override { return true; }
private:
    std::unique_ptr<Value> value;
};

class BoundTemporary : public BoundExpression
{
public:
    BoundTemporary(std::unique_ptr<BoundExpression>&& rvalueExpr_, std::unique_ptr<BoundLocalVariable>&& backingStore_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    bool IsLvalueExpression() const override { return true; }
    std::string TypeString() const override { return "temporary"; }
private:
    std::unique_ptr<BoundExpression> rvalueExpr;
    std::unique_ptr<BoundLocalVariable> backingStore;
};

class BoundSizeOfExpression : public BoundExpression
{
public:
    BoundSizeOfExpression(const Span& span_, TypeSymbol* type_, TypeSymbol* pointerType_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    std::string TypeString() const override { return "sizeof"; }
private:
    TypeSymbol* pointerType;
};

class BoundAddressOfExpression : public BoundExpression
{
public:
    BoundAddressOfExpression(std::unique_ptr<BoundExpression>&& subject_, TypeSymbol* type_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    std::string TypeString() const override { return "address of expression"; }
    std::unique_ptr<BoundExpression>& Subject() { return subject; }
private:
    std::unique_ptr<BoundExpression> subject;
};

class BoundDereferenceExpression : public BoundExpression
{
public:
    BoundDereferenceExpression(std::unique_ptr<BoundExpression>&& subject_, TypeSymbol* type_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    std::string TypeString() const override { return "deference expression"; }
    bool IsLvalueExpression() const override { return true; }
    BoundExpression* Subject() { return subject.get(); }
private:
    std::unique_ptr<BoundExpression> subject;
};

class BoundReferenceToPointerExpression : public BoundExpression
{
public:
    BoundReferenceToPointerExpression(std::unique_ptr<BoundExpression>&& subject_, TypeSymbol* type_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    std::string TypeString() const override { return "reference to pointer expression"; }
private:
    std::unique_ptr<BoundExpression> subject;
};

class BoundFunctionCall : public BoundExpression
{
public:
    BoundFunctionCall(const Span& span_, FunctionSymbol* functionSymbol_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override;
    std::string TypeString() const override { return "function call"; }
    bool IsLvalueExpression() const override;
    const FunctionSymbol* GetFunctionSymbol() const { return functionSymbol; }
    FunctionSymbol* GetFunctionSymbol() { return functionSymbol; }
    void AddArgument(std::unique_ptr<BoundExpression>&& argument);
    void SetArguments(std::vector<std::unique_ptr<BoundExpression>>&& arguments_);
    const std::vector<std::unique_ptr<BoundExpression>>& Arguments() const { return arguments; }
private:
    FunctionSymbol* functionSymbol;
    std::vector<std::unique_ptr<BoundExpression>> arguments;
};

class BoundConstructExpression : public BoundExpression
{
public:
    BoundConstructExpression(std::unique_ptr<BoundExpression>&& constructorCall_, TypeSymbol* resultType_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    std::string TypeString() const override { return "construct expression"; }
private:
    std::unique_ptr<BoundExpression> constructorCall;
};

class BoundConversion : public BoundExpression
{
public:
    BoundConversion(std::unique_ptr<BoundExpression>&& sourceExpr_, FunctionSymbol* conversionFun_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    std::string TypeString() const override { return "conversion"; }
private:
    std::unique_ptr<BoundExpression> sourceExpr;
    FunctionSymbol* conversionFun;
};

class BoundTypeExpression : public BoundExpression
{
public:
    BoundTypeExpression(const Span& span_, TypeSymbol* type_);
    bool IsComplete() const override { return false; }
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    std::string TypeString() const override { return "type expression"; }
};

class BoundNamespaceExpression : public BoundExpression
{
public:
    BoundNamespaceExpression(const Span& span_, NamespaceSymbol* ns_);
    bool IsComplete() const override { return false; }
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    std::string TypeString() const override { return "namespace expression"; }
    NamespaceSymbol* Ns() { return ns; }
private:
    NamespaceSymbol* ns;
    std::unique_ptr<TypeSymbol> nsType;
};

class BoundFunctionGroupExpression : public BoundExpression
{
public:
    BoundFunctionGroupExpression(const Span& span_, FunctionGroupSymbol* functionGroupSymbol_);
    bool IsComplete() const override { return false; }
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    std::string TypeString() const override { return "function group expression"; }
    const FunctionGroupSymbol* FunctionGroup() const { return functionGroupSymbol; }
    FunctionGroupSymbol* FunctionGroup() { return functionGroupSymbol; }
    void SetClassPtr(std::unique_ptr<BoundExpression>&& classPtr_);
    bool IsScopeQualified() const { return scopeQualified; }
    void SetScopeQualified() { scopeQualified = true; }
    ContainerScope* QualifiedScope() const { return qualifiedScope; }
    void SetQualifiedScope(ContainerScope* qualifiedScope_) { qualifiedScope = qualifiedScope_; }
private:
    FunctionGroupSymbol* functionGroupSymbol;
    std::unique_ptr<TypeSymbol> functionGroupType;
    std::unique_ptr<BoundExpression> classPtr;
    bool scopeQualified;
    ContainerScope* qualifiedScope;
};

class BoundMemberExpression : public BoundExpression
{
public:
    BoundMemberExpression(const Span& span_, std::unique_ptr<BoundExpression>&& classPtr_, std::unique_ptr<BoundExpression>&& member_);
    bool IsComplete() const override { return false; }
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    std::string TypeString() const override { return "member expression"; }
    BoundExpression* ClassPtr() { return classPtr.get(); }
    BoundExpression* ReleaseClassPtr() { return classPtr.release(); }
    BoundExpression* Member() { return member.get(); }
private:
    std::unique_ptr<BoundExpression> classPtr;
    std::unique_ptr<BoundExpression> member;
    std::unique_ptr<TypeSymbol> memberExpressionType;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_BOUND_EXPRESSION_INCLUDED
