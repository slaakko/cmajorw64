// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_DELEGATE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_DELEGATE_SYMBOL_INCLUDED
#include <cmajor/symbols/ClassTypeSymbol.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>

namespace cmajor { namespace symbols {

class ParameterSymbol;

class DelegateTypeSymbol : public TypeSymbol
{
public:
    DelegateTypeSymbol(const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol_, int index) override;
    void ComputeExportClosure() override;
    void AddMember(Symbol* member) override;
    std::string TypeString() const override { return "delegate"; }
    void Accept(SymbolCollector* collector) override;
    void Dump(CodeFormatter& formatter) override;
    llvm::Type* IrType(Emitter& emitter) override;
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override;
    void SetSpecifiers(Specifiers specifiers);
    int Arity() const { return parameters.size(); }
    const std::vector<ParameterSymbol*>& Parameters() const { return parameters; }
    const TypeSymbol* ReturnType() const { return returnType; }
    TypeSymbol* ReturnType() { return returnType; }
    void SetReturnType(TypeSymbol* returnType_) { returnType = returnType_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags);
private:
    TypeSymbol* returnType;
    std::vector<ParameterSymbol*> parameters;
    llvm::Type* irType;
};

class DelegateTypeDefaultConstructor : public FunctionSymbol
{
public:
    DelegateTypeDefaultConstructor(const Span& span_, const std::u32string& name_);
    DelegateTypeDefaultConstructor(DelegateTypeSymbol* delegateType_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol, int index) override;
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    DelegateTypeSymbol* delegateType;
};

class DelegateTypeCopyConstructor : public FunctionSymbol
{
public:
    DelegateTypeCopyConstructor(const Span& span_, const std::u32string& name_);
    DelegateTypeCopyConstructor(DelegateTypeSymbol* delegateType);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
};

class DelegateTypeMoveConstructor : public FunctionSymbol
{
public:
    DelegateTypeMoveConstructor(const Span& span_, const std::u32string& name_);
    DelegateTypeMoveConstructor(DelegateTypeSymbol* delegateType);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
};

class DelegateTypeCopyAssignment : public FunctionSymbol
{
public:
    DelegateTypeCopyAssignment(const Span& span_, const std::u32string& name_);
    DelegateTypeCopyAssignment(DelegateTypeSymbol* delegateType, TypeSymbol* voidType);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
};

class DelegateTypeMoveAssignment : public FunctionSymbol
{
public:
    DelegateTypeMoveAssignment(const Span& span_, const std::u32string& name_);
    DelegateTypeMoveAssignment(DelegateTypeSymbol* delegateType, TypeSymbol* voidType);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
};

class DelegateTypeReturn : public FunctionSymbol
{
public:
    DelegateTypeReturn(const Span& span_, const std::u32string& name_);
    DelegateTypeReturn(DelegateTypeSymbol* delegateType);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
};

class DelegateTypeEquality : public FunctionSymbol
{
public:
    DelegateTypeEquality(const Span& span_, const std::u32string& name_);
    DelegateTypeEquality(DelegateTypeSymbol* delegateType, TypeSymbol* boolType);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
};

class FunctionToDelegateConversion : public FunctionSymbol
{
public:
    FunctionToDelegateConversion(const Span& span_, const std::u32string& name_);
    FunctionToDelegateConversion(TypeSymbol* sourceType_, TypeSymbol* targetType_, FunctionSymbol* function_);
    ConversionType GetConversionType() const override { return ConversionType::implicit_; }
    uint8_t ConversionDistance() const override { return 1; }
    TypeSymbol* ConversionSourceType() const override { return sourceType; }
    TypeSymbol* ConversionTargetType() const override { return targetType; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    TypeSymbol* sourceType;
    TypeSymbol* targetType;
    FunctionSymbol* function;
};

class ClassDelegateTypeSymbol : public ClassTypeSymbol
{
public:
    ClassDelegateTypeSymbol(const Span& span_, const std::u32string& name_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol_, int index) override;
    void ComputeExportClosure() override;
    void AddMember(Symbol* member) override;
    std::string TypeString() const override { return "class_delegate"; }
    void Accept(SymbolCollector* collector) override;
    void Dump(CodeFormatter& formatter) override;
    bool IsClassTypeSymbol() const override { return false; }
    void SetSpecifiers(Specifiers specifiers);
    int Arity() const { return parameters.size(); }
    const TypeSymbol* ReturnType() const { return returnType; }
    void SetReturnType(TypeSymbol* returnType_) { returnType = returnType_; }
    llvm::Type* IrType(Emitter& emitter) override { return nullptr; } // todo
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return nullptr; } // todo
private:
    TypeSymbol* returnType;
    std::vector<ParameterSymbol*> parameters;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_DELEGATE_SYMBOL_INCLUDED
