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
    bindToRvalueReference = 1 << 1,
    virtualCall = 1 << 2,
    deref = 1 << 3
};

inline BoundExpressionFlags operator|(BoundExpressionFlags left, BoundExpressionFlags right)
{
    return BoundExpressionFlags(uint8_t(left) | uint8_t(right));
}

inline BoundExpressionFlags operator&(BoundExpressionFlags left, BoundExpressionFlags right)
{
    return BoundExpressionFlags(uint8_t(left) & uint8_t(right));
}

class BoundFunctionCall;
class BoundFunction;

class BoundExpression : public BoundNode
{
public:
    BoundExpression(const Span& span_, BoundNodeType boundNodeType_, TypeSymbol* type_);
    virtual BoundExpression* Clone() = 0;
    virtual bool IsComplete() const { return true; }
    virtual bool IsLvalueExpression() const { return false; }
    virtual bool HasValue() const { return false; }
    virtual std::string TypeString() const { return "expression"; }
    const TypeSymbol* GetType() const { return type; }
    TypeSymbol* GetType() { return type; }
    bool GetFlag(BoundExpressionFlags flag) const { return (flags & flag) != BoundExpressionFlags::none;  }
    void SetFlag(BoundExpressionFlags flag) { flags = flags | flag; }
    void AddTemporaryDestructorCall(std::unique_ptr<BoundFunctionCall>&& destructorCall);
    void DestroyTemporaries(Emitter& emitter);
private:
    TypeSymbol* type;
    BoundExpressionFlags flags;
    std::vector<std::unique_ptr<BoundFunctionCall>> temporaryDestructorCalls;
};

class BoundParameter : public BoundExpression
{
public:
    BoundParameter(ParameterSymbol* parameterSymbol_);
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    std::string TypeString() const override { return "deference expression"; }
    bool IsLvalueExpression() const override { return true; }
    std::unique_ptr<BoundExpression>& Subject() { return subject; }
private:
    std::unique_ptr<BoundExpression> subject;
};

class BoundReferenceToPointerExpression : public BoundExpression
{
public:
    BoundReferenceToPointerExpression(std::unique_ptr<BoundExpression>&& subject_, TypeSymbol* type_);
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    std::string TypeString() const override { return "construct expression"; }
private:
    std::unique_ptr<BoundExpression> constructorCall;
};

class BoundConstructAndReturnTemporaryExpression : public BoundExpression
{
public:
    BoundConstructAndReturnTemporaryExpression(std::unique_ptr<BoundExpression>&& constructorCall_, std::unique_ptr<BoundExpression>&& boundTemporary_);
    BoundExpression* Clone() override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    bool IsLvalueExpression() const override { return true; }
    std::string TypeString() const override { return "construct and return temporary expression"; }
private:
    std::unique_ptr<BoundExpression> constructorCall;
    std::unique_ptr<BoundExpression> boundTemporary;
};

class BoundConversion : public BoundExpression
{
public:
    BoundConversion(std::unique_ptr<BoundExpression>&& sourceExpr_, FunctionSymbol* conversionFun_);
    BoundExpression* Clone() override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    std::string TypeString() const override { return "conversion"; }
private:
    std::unique_ptr<BoundExpression> sourceExpr;
    FunctionSymbol* conversionFun;
};

class BoundIsExpression : public BoundExpression
{
public:
    BoundIsExpression(std::unique_ptr<BoundExpression>&& expr_, ClassTypeSymbol* rightClassType_, TypeSymbol* boolType_);
    BoundExpression* Clone() override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    std::string TypeString() const override { return "is expression"; }
private:
    std::unique_ptr<BoundExpression> expr;
    ClassTypeSymbol* rightClassType;
};

class BoundAsExpression : public BoundExpression
{
public:
    BoundAsExpression(std::unique_ptr<BoundExpression>&& expr_, ClassTypeSymbol* rightClassType_, std::unique_ptr<BoundLocalVariable>&& variable_);
    BoundExpression* Clone() override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    std::string TypeString() const override { return "as expression"; }
private:
    std::unique_ptr<BoundExpression> expr;
    ClassTypeSymbol* rightClassType;
    std::unique_ptr<BoundLocalVariable> variable;
};

class BoundTypeNameExpression : public BoundExpression
{
public:
    BoundTypeNameExpression(std::unique_ptr<BoundExpression>&& classPtr_, TypeSymbol* constCharPtrType_);
    BoundExpression* Clone() override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
private:
    std::unique_ptr<BoundExpression> classPtr;
};

class BoundBitCast : public BoundExpression
{
public:
    BoundBitCast(std::unique_ptr<BoundExpression>&& expr_, TypeSymbol* type_);
    BoundExpression* Clone() override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
private:
    std::unique_ptr<BoundExpression> expr;
};

class BoundFunctionPtr : public BoundExpression
{
public:
    BoundFunctionPtr(const Span& span_, FunctionSymbol* function_, TypeSymbol* type_);
    BoundExpression* Clone() override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
private:
    FunctionSymbol* function;
};

class BoundDisjunction : public BoundExpression
{
public:
    BoundDisjunction(const Span& span_, std::unique_ptr<BoundExpression>&& left_, std::unique_ptr<BoundExpression>&& right_, TypeSymbol* boolType_);
    BoundExpression* Clone() override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    BoundExpression* Left() { return left.get(); }
    BoundExpression* Right() { return right.get(); }
    void SetTemporary(BoundLocalVariable* temporary_);
private:
    std::unique_ptr<BoundExpression> left;
    std::unique_ptr<BoundExpression> right;
    std::unique_ptr<BoundLocalVariable> temporary;
};

class BoundConjunction : public BoundExpression
{
public:
    BoundConjunction(const Span& span_, std::unique_ptr<BoundExpression>&& left_, std::unique_ptr<BoundExpression>&& right_, TypeSymbol* boolType_);
    BoundExpression* Clone() override;
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    bool HasValue() const override { return true; }
    BoundExpression* Left() { return left.get(); }
    BoundExpression* Right() { return right.get(); }
    void SetTemporary(BoundLocalVariable* temporary_);
private:
    std::unique_ptr<BoundExpression> left;
    std::unique_ptr<BoundExpression> right;
    std::unique_ptr<BoundLocalVariable> temporary;
};

class BoundTypeExpression : public BoundExpression
{
public:
    BoundTypeExpression(const Span& span_, TypeSymbol* type_);
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
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
    BoundExpression* Clone() override;
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
    void SetTemplateArgumentTypes(const std::vector<TypeSymbol*>& templateArgumentTypes_);
    const std::vector<TypeSymbol*>& TemplateArgumentTypes() const { return templateArgumentTypes; }
private:
    FunctionGroupSymbol* functionGroupSymbol;
    std::unique_ptr<TypeSymbol> functionGroupType;
    std::unique_ptr<BoundExpression> classPtr;
    bool scopeQualified;
    ContainerScope* qualifiedScope;
    std::vector<TypeSymbol*> templateArgumentTypes;
};

class BoundMemberExpression : public BoundExpression
{
public:
    BoundMemberExpression(const Span& span_, std::unique_ptr<BoundExpression>&& classPtr_, std::unique_ptr<BoundExpression>&& member_);
    BoundExpression* Clone() override;
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