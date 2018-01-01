// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_TYPE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_TYPE_SYMBOL_INCLUDED
#include <cmajor/symbols/ContainerSymbol.hpp>
#include <cmajor/ir/Emitter.hpp>
#include <llvm/IR/Type.h>

namespace cmajor { namespace symbols {

using namespace cmajor::ir;

struct TypeDerivationRec;
enum class ValueType : uint8_t;
class Value;

class TypeSymbol : public ContainerSymbol
{
public:
    TypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    bool IsTypeSymbol() const override { return true; }
    virtual bool IsInComplete() const { return false; }
    virtual bool IsIntegralType() const { return false; }
    virtual bool IsFloatingPointType() const { return false; }
    virtual bool IsUnsignedType() const { return false; }
    virtual bool IsVoidType() const { return false; }
    std::string TypeString() const override { return "type"; }
    virtual bool IsBasicTypeSymbol() const { return false; }
    virtual bool IsDelegateType() const { return false; }
    virtual bool IsClassDelegateType() const { return false; }
    virtual bool IsEnumeratedType() const { return false; }
    virtual const TypeSymbol* BaseType() const { return this; }
    virtual TypeSymbol* BaseType() { return this; }
    virtual TypeSymbol* PlainType(const Span& span) { return this; }
    virtual TypeSymbol* RemoveConst(const Span& span) { return this; }
    virtual TypeSymbol* RemoveReference(const Span& span) { return this; }
    virtual TypeSymbol* RemovePointer(const Span& span) { return this; }
    virtual TypeSymbol* AddConst(const Span& span);
    virtual TypeSymbol* AddLvalueReference(const Span& span);
    virtual TypeSymbol* AddRvalueReference(const Span& span);
    virtual TypeSymbol* AddPointer(const Span& span);
    virtual llvm::Type* IrType(Emitter& emitter) = 0;
    virtual llvm::Constant* CreateDefaultIrValue(Emitter& emitter) = 0;
    virtual bool IsConstType() const { return false; }
    virtual bool IsReferenceType() const { return false; }
    virtual bool IsLvalueReferenceType() const { return false; }
    virtual bool IsRvalueReferenceType() const { return false; }
    virtual bool IsArrayType() const { return false; }
    virtual bool IsPointerType() const { return false; }
    virtual bool IsNullPtrType() const { return false; }
    virtual bool IsVoidPtrType() const { return false; }
    virtual bool IsPolymorphicType() const { return false; }
    virtual bool IsSwitchConditionType() const { return false; }
    virtual int PointerCount() const { return 0; }
    virtual bool HasNontrivialDestructor() const { return false; }
    virtual bool ContainsTemplateParameter() const { return false; }
    void SetTypeId(uint32_t typeId_) { typeId = typeId_; }
    uint32_t TypeId() const { Assert(typeId != 0, "type id not initialized");  return typeId; }
    bool TypeIdNotSet() const { return typeId == 0; }
    virtual const TypeDerivationRec& DerivationRec() const;
    virtual TypeSymbol* RemoveDerivations(const TypeDerivationRec& sourceDerivationRec, const Span& span);
    virtual TypeSymbol* Unify(TypeSymbol* that, const Span& span) { return nullptr; }
    virtual bool IsRecursive(TypeSymbol* type, std::unordered_set<TypeSymbol*>& tested);
    virtual ValueType GetValueType() const;
    virtual Value* MakeValue() const { return nullptr; }
    std::u32string Id() const override;
private:
    uint32_t typeId;
};

bool CompareTypesForEquality(const TypeSymbol* left, const TypeSymbol* right);

inline bool TypesEqual(const TypeSymbol* left, const TypeSymbol* right)
{
    if (left == right) return true;
    return CompareTypesForEquality(left, right);
}

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_TYPE_SYMBOL_INCLUDED
