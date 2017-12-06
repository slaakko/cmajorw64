// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/Meta.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/TemplateSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

IntrinsicFunction::~IntrinsicFunction()
{
}

std::unique_ptr<Value> IntrinsicFunction::Evaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span)
{
    if (arguments.size() != Arity())
    {
        throw Exception("wrong number of parameters for intrinsic " + std::string(GroupName()), span);
    }
    if (templateArguments.size() != NumberOfTypeParameters())
    {
        throw Exception("wrong number of template type arguments for intrinsic " + std::string(GroupName()), span);
    }
    return DoEvaluate(arguments, templateArguments, span);
}

FunctionSymbol* CreateIntrinsic(IntrinsicFunction* intrinsic, SymbolTable& symbolTable)
{
    FunctionSymbol* fun = new FunctionSymbol(Span(), ToUtf32(intrinsic->GroupName()));
    fun->SetGroupName(ToUtf32(intrinsic->GroupName()));
    fun->SetIntrinsic(intrinsic);
    fun->SetAccess(SymbolAccess::public_);
    fun->SetReturnType(intrinsic->ReturnType(symbolTable));
    int n = intrinsic->NumberOfTypeParameters();
    for (int i = 0; i < n; ++i)
    {
        std::u32string p = U"T";
        if (i > 0)
        {
            p.append(ToUtf32(std::to_string(i)));
        }
        fun->AddMember(new TemplateParameterSymbol(Span(), p));
    }
    fun->ComputeName();
    return fun;
}

class TypePredicate : public IntrinsicFunction
{
public:
    int Arity() const override { return 0; }
    int NumberOfTypeParameters() const override { return 1; }
    TypeSymbol* ReturnType(SymbolTable& symbolTable) const override { return symbolTable.GetTypeByName(U"bool"); }
};

class IsIntegralTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsIntegralType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsIntegralType()));
    }
};

class IsSignedTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsSignedType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsIntegralType() && !type->IsUnsignedType()));
    }
};

class IsUnsignedTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsUnsignedType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsUnsignedType()));
    }
};

class IsFloatingPointTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsFloatingPointType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsFloatingPointType()));
    }
};

class IsBasicTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsBasicType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsBasicTypeSymbol()));
    }
};

class IsBoolTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsBoolType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::boolTypeSymbol));
    }
};

class IsSByteTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsSByteType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::sbyteTypeSymbol));
    }
};

class IsByteTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsByteType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::byteTypeSymbol));
    }
};

class IsShortTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsShortType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::shortTypeSymbol));
    }
};

class IsUShortTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsUShortType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::ushortTypeSymbol));
    }
};

class IsIntTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsIntType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::intTypeSymbol));
    }
};

class IsUIntTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsUIntType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::uintTypeSymbol));
    }
};

class IsLongTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsLongType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::longTypeSymbol));
    }
};

class IsULongTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsULongType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::ulongTypeSymbol));
    }
};

class IsFloatTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsFloatType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::floatTypeSymbol));
    }
};

class IsDoubleTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsDoubleType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::doubleTypeSymbol));
    }
};

class IsCharTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsCharType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::charTypeSymbol));
    }
};

class IsWCharTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsWCharType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::wcharTypeSymbol));
    }
};

class IsUCharTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsUCharType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::ucharTypeSymbol));
    }
};

class IsVoidTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsVoidType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::voidTypeSymbol));
    }
};

class IsClassTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsClassType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsClassTypeSymbol()));
    }
};

class IsPolymorphicTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsPolymorphicType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsPolymorphicType()));
    }
};

class IsInterfaceTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsInterfaceType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->GetSymbolType() == SymbolType::interfaceTypeSymbol));
    }
};

class IsDelegateTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsDelegateType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsDelegateType()));
    }
};

class IsClassDelegateTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsClassDelegateType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsClassDelegateType()));
    }
};

class IsEnumeratedTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsEnumeratedType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsEnumeratedType()));
    }
};

class IsConstTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsConstType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsConstType()));
    }
};

class IsReferenceTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsReferenceType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsReferenceType()));
    }
};

class IsLvalueReferenceTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsLvalueReferenceType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsLvalueReferenceType()));
    }
};

class IsRvalueReferenceTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsRvalueReferenceType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsRvalueReferenceType()));
    }
};

class IsArrayTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsArrayType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsArrayType()));
    }
};

class IsPointerTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsPointerType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsPointerType()));
    }
};

class IsGenericPtrTypePredicate : public TypePredicate
{
public:
    const char* GroupName() const override { return "IsGenericPtrType"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new BoolValue(span, type->IsVoidPtrType()));
    }
};

class PointerCountIntrinsicFunction : public IntrinsicFunction
{
public:
    int Arity() const override { return 0; }
    int NumberOfTypeParameters() const override { return 1; }
    TypeSymbol* ReturnType(SymbolTable& symbolTable) const override { return symbolTable.GetTypeByName(U"int"); }
    const char* GroupName() const override { return "PointerCount"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        return std::unique_ptr<Value>(new IntValue(span, type->PointerCount()));
    }
};

class ArrayLengthIntrinsicFunction : public IntrinsicFunction
{
public:
    int Arity() const override { return 0; }
    int NumberOfTypeParameters() const override { return 1; }
    TypeSymbol* ReturnType(SymbolTable& symbolTable) const override { return symbolTable.GetTypeByName(U"long"); }
    const char* GroupName() const override { return "ArrayLength"; }
    std::unique_ptr<Value> DoEvaluate(const std::vector<std::unique_ptr<Value>>& arguments, const std::vector<TypeSymbol*>& templateArguments, const Span& span) override
    {
        TypeSymbol* type = templateArguments.front();
        if (type->IsArrayType())
        {
            ArrayTypeSymbol* arrayType = static_cast<ArrayTypeSymbol*>(type);
            return std::unique_ptr<Value>(new LongValue(span, arrayType->Size()));
        }
        return std::unique_ptr<Value>(new LongValue(span, 0));
    }
};

void MetaInit(SymbolTable& symbolTable)
{
    symbolTable.BeginNamespace(U"System.Meta", Span());
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsIntegralTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsSignedTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsUnsignedTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsFloatingPointTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsBasicTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsBoolTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsSByteTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsByteTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsShortTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsUShortTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsIntTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsUIntTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsLongTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsULongTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsFloatTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsDoubleTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsCharTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsWCharTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsUCharTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsVoidTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsClassTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsPolymorphicTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsInterfaceTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsDelegateTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsClassDelegateTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsEnumeratedTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsConstTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsReferenceTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsLvalueReferenceTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsRvalueReferenceTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsArrayTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsPointerTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new IsGenericPtrTypePredicate(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new PointerCountIntrinsicFunction(), symbolTable));
    symbolTable.Container()->AddMember(CreateIntrinsic(new ArrayLengthIntrinsicFunction(), symbolTable));
    symbolTable.EndNamespace(); 
}

} } // namespace cmajor::symbols
