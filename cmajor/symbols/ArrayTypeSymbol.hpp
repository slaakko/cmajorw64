// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_ARRAY_TYPE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_ARRAY_TYPE_SYMBOL_INCLUDED
#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>

namespace cmajor { namespace symbols {

class ArrayTypeSymbol : public TypeSymbol
{
public:
    ArrayTypeSymbol(const Span& span_, const std::u32string& name_);
    ArrayTypeSymbol(const Span& span_, const std::u32string& name_, TypeSymbol* elementType_, uint64_t size_);
    void Write(SymbolWriter& writer) override;
    void Read(SymbolReader& reader) override;
    void EmplaceType(TypeSymbol* typeSymbol, int index) override;
    llvm::Type* IrType(Emitter& emitter) override;
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override;
    bool IsArrayType() const override { return true; }
    TypeSymbol* ElementType() const { return elementType; }
    uint64_t Size() const { return size; }
private:
    TypeSymbol* elementType;
    uint64_t size;
    llvm::Type* irType;
};

class ArrayTypeDefaultConstructor : public FunctionSymbol
{
public:
    ArrayTypeDefaultConstructor(ArrayTypeSymbol* arrayType_, LocalVariableSymbol* loopVar_, FunctionSymbol* elementTypeDefaultConstructor_, const Span& span_);
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    ArrayTypeSymbol* arrayType;
    LocalVariableSymbol* loopVar;
    FunctionSymbol* elementTypeDefaultConstructor;
};

class ArrayTypeCopyConstructor : public FunctionSymbol
{
public:
    ArrayTypeCopyConstructor(ArrayTypeSymbol* arrayType_, LocalVariableSymbol* loopVar_, FunctionSymbol* elementTypeCopyConstructor_, const Span& span_);
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    ArrayTypeSymbol* arrayType;
    LocalVariableSymbol* loopVar;
    FunctionSymbol* elementTypeCopyConstructor;
};

class ArrayTypeMoveConstructor : public FunctionSymbol
{
public:
    ArrayTypeMoveConstructor(ArrayTypeSymbol* arrayType_, LocalVariableSymbol* loopVar_, FunctionSymbol* elementTypeMoveConstructor_, const Span& span_);
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    ArrayTypeSymbol* arrayType;
    LocalVariableSymbol* loopVar;
    FunctionSymbol* elementTypeMoveConstructor;
};

class ArrayTypeCopyAssignment : public FunctionSymbol
{
public:
    ArrayTypeCopyAssignment(ArrayTypeSymbol* arrayType_, LocalVariableSymbol* loopVar_, FunctionSymbol* elementTypeCopyAssignment_, const Span& span_);
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    ArrayTypeSymbol* arrayType;
    LocalVariableSymbol* loopVar;
    FunctionSymbol* elementTypeCopyAssignment;
};

class ArrayTypeMoveAssignment : public FunctionSymbol
{
public:
    ArrayTypeMoveAssignment(ArrayTypeSymbol* arrayType_, LocalVariableSymbol* loopVar_, FunctionSymbol* elementTypeMoveAssignment_, const Span& span_);
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
private:
    ArrayTypeSymbol* arrayType;
    LocalVariableSymbol* loopVar;
    FunctionSymbol* elementTypeMoveAssignment;
};

class ArrayTypeElementAccess : public FunctionSymbol
{
public:
    ArrayTypeElementAccess(ArrayTypeSymbol* arrayType_, const Span& span_);
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects, OperationFlags flags) override;
    bool IsBasicTypeOperation() const override { return true; }
    bool IsArrayElementAccess() const override { return true; }
private:
    ArrayTypeSymbol* arrayType;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_ARRAY_TYPE_SYMBOL_INCLUDED
