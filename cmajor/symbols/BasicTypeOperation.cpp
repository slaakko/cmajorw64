// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/BasicTypeOperation.hpp>
#include <cmajor/symbols/FunctionSymbol.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/ir/Emitter.hpp>

namespace cmajor { namespace symbols {

struct BasicTypeUnaryPlus
{
    static const char32_t* GroupName() { return U"operator+"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* arg) { return arg; }
};

struct BasicTypeIntUnaryMinus
{
    static const char32_t* GroupName() { return U"operator-"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* arg) { return builder.CreateNeg(arg); }
};

struct BasicTypeFloatUnaryMinus
{
    static const char32_t* GroupName() { return U"operator-"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* arg) { return builder.CreateFNeg(arg); }
};

struct BasicTypeComplement
{
    static const char32_t* GroupName() { return U"operator~"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* arg) { return builder.CreateNot(arg); }
};

struct BasicTypeAdd
{
    static const char32_t* GroupName() { return U"operator+";  }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateAdd(left, right); }
};

struct BasicTypeFAdd
{
    static const char32_t* GroupName() { return U"operator+"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateFAdd(left, right); }
};

struct BasicTypeSub
{
    static const char32_t* GroupName() { return U"operator-"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateSub(left, right); }
};

struct BasicTypeFSub
{
    static const char32_t* GroupName() { return U"operator-"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateFSub(left, right); }
};

struct BasicTypeMul
{
    static const char32_t* GroupName() { return U"operator*"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateMul(left, right); }
};

struct BasicTypeFMul
{
    static const char32_t* GroupName() { return U"operator*"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateFMul(left, right); }
};

struct BasicTypeUDiv
{
    static const char32_t* GroupName() { return U"operator/"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateUDiv(left, right); }
};

struct BasicTypeSDiv
{
    static const char32_t* GroupName() { return U"operator/"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateSDiv(left, right); }
};

struct BasicTypeFDiv
{
    static const char32_t* GroupName() { return U"operator/"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateFDiv(left, right); }
};

struct BasicTypeURem
{
    static const char32_t* GroupName() { return U"operator%"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateURem(left, right); }
};

struct BasicTypeSRem
{
    static const char32_t* GroupName() { return U"operator%"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateSRem(left, right); }
};

struct BasicTypeAnd
{
    static const char32_t* GroupName() { return U"operator&"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateAnd(left, right); }
};

struct BasicTypeOr
{
    static const char32_t* GroupName() { return U"operator|"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateOr(left, right); }
};

struct BasicTypeXor
{
    static const char32_t* GroupName() { return U"operator^"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateXor(left, right); }
};

struct BasicTypeShl
{
    static const char32_t* GroupName() { return U"operator<<"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateShl(left, right); }
};

struct BasicTypeAShr
{
    static const char32_t* GroupName() { return U"operator>>"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateAShr(left, right); }
};

struct BasicTypeLShr
{
    static const char32_t* GroupName() { return U"operator>>"; }
    static llvm::Value* Generate(llvm::IRBuilder<>& builder, llvm::Value* left, llvm::Value* right) { return builder.CreateLShr(left, right); }
};

template<typename UnOp>
class BasicTypeUnaryOperation : public FunctionSymbol
{
public:
    BasicTypeUnaryOperation(TypeSymbol* type);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
};

template<typename UnOp>
BasicTypeUnaryOperation<UnOp>::BasicTypeUnaryOperation(TypeSymbol* type) : FunctionSymbol(Span(), UnOp::GroupName())
{
    SetGroupName(UnOp::GroupName());
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* operandParam = new ParameterSymbol(Span(), U"operand");
    operandParam->SetType(type);
    AddMember(operandParam);
    ComputeName();
}

template<typename UnOp>
void BasicTypeUnaryOperation<UnOp>::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 1, "unary operation needs one object");
    genObjects[0]->Load(emitter);
    llvm::Value* arg = emitter.Stack().Pop();
    emitter.Stack().Push(UnOp::Generate(emitter.Builder(), arg));
}

template<typename BinOp>
class BasicTypeBinaryOperation : public FunctionSymbol
{
public:
    BasicTypeBinaryOperation(TypeSymbol* type);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
};

template<typename BinOp>
BasicTypeBinaryOperation<BinOp>::BasicTypeBinaryOperation(TypeSymbol* type) : FunctionSymbol(Span(), BinOp::GroupName())
{
    SetGroupName(BinOp::GroupName());
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* leftParam = new ParameterSymbol(Span(), U"left");
    leftParam->SetType(type);
    AddMember(leftParam);
    ParameterSymbol* rightParam = new ParameterSymbol(Span(), U"right");
    rightParam->SetType(type);
    AddMember(rightParam);
    ComputeName();
}

template<typename BinOp>
void BasicTypeBinaryOperation<BinOp>::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 2, "binary operation needs two objects");
    genObjects[0]->Load(emitter);
    llvm::Value* left = emitter.Stack().Pop();
    genObjects[1]->Load(emitter);
    llvm::Value* right = emitter.Stack().Pop();
    emitter.Stack().Push(BinOp::Generate(emitter.Builder(), left, right));
}

struct DefaultInt1
{
    static llvm::Value* Generate(llvm::IRBuilder<>& builder) { return builder.getInt1(false); }
};

struct DefaultInt8
{
    static llvm::Value* Generate(llvm::IRBuilder<>& builder) { return builder.getInt8(0); }
};

struct DefaultInt16
{
    static llvm::Value* Generate(llvm::IRBuilder<>& builder) { return builder.getInt16(0); }
};

struct DefaultInt32
{
    static llvm::Value* Generate(llvm::IRBuilder<>& builder) { return builder.getInt32(0); }
};

struct DefaultInt64
{
    static llvm::Value* Generate(llvm::IRBuilder<>& builder) { return builder.getInt64(0); }
};

template<typename DefaultOp>
class BasicTypeDefaultCtor : public FunctionSymbol
{
public:
    BasicTypeDefaultCtor(TypeSymbol* type);
    SymbolAccess DeclaredAccess() const override { return SymbolAccess::public_; }
    void GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects) override;
};

template<typename DefaultOp>
BasicTypeDefaultCtor<DefaultOp>::BasicTypeDefaultCtor(TypeSymbol* type) : FunctionSymbol(Span(), U"@constructor")
{
    SetGroupName(U"@constructor");
    SetAccess(SymbolAccess::public_);
    ParameterSymbol* thisParam = new ParameterSymbol(Span(), U"this");
    thisParam->SetType(type);
    AddMember(thisParam);
    ComputeName();
}

template<typename DefaultOp>
void BasicTypeDefaultCtor<DefaultOp>::GenerateCall(Emitter& emitter, std::vector<GenObject*>& genObjects)
{
    Assert(genObjects.size() == 1, "default constructor needs one object");
    emitter.Stack().Push(DefaultOp::Generate(emitter.Builder()));
    genObjects[0]->Store(emitter);
}

void MakeSignedIntegerTypeOperations(SymbolTable& symbolTable, TypeSymbol* type)
{
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeUnaryOperation<BasicTypeUnaryPlus>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeUnaryOperation<BasicTypeIntUnaryMinus>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeUnaryOperation<BasicTypeComplement>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeAdd>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeSub>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeMul>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeSDiv>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeSRem>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeAnd>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeOr>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeXor>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeShl>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeAShr>(type));
}

void MakeUnsignedIntegerTypeOperations(SymbolTable& symbolTable, TypeSymbol* type)
{
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeUnaryOperation<BasicTypeUnaryPlus>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeUnaryOperation<BasicTypeIntUnaryMinus>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeUnaryOperation<BasicTypeComplement>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeAdd>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeSub>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeMul>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeUDiv>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeURem>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeAnd>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeOr>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeXor>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeShl>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeLShr>(type));
}

void MakeFloatingPointTypeOperations(SymbolTable& symbolTable, TypeSymbol* type)
{
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeUnaryOperation<BasicTypeUnaryPlus>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeUnaryOperation<BasicTypeFloatUnaryMinus>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeFAdd>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeFSub>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeFMul>(type));
    symbolTable.AddFunctionSymbolToGlobalScope(new BasicTypeBinaryOperation<BasicTypeFDiv>(type));
}

void MakeBasicTypeOperations(SymbolTable& symbolTable,
    BoolTypeSymbol* boolType, SByteTypeSymbol* sbyteType, ByteTypeSymbol* byteType, ShortTypeSymbol* shortType, UShortTypeSymbol* ushortType, IntTypeSymbol* intType, UIntTypeSymbol* uintType,
    LongTypeSymbol* longType, ULongTypeSymbol* ulongType, FloatTypeSymbol* floatType, DoubleTypeSymbol* doubleType, CharTypeSymbol* charType, WCharTypeSymbol* wcharType, UCharTypeSymbol* ucharType)
{
    MakeSignedIntegerTypeOperations(symbolTable, sbyteType);
    MakeUnsignedIntegerTypeOperations(symbolTable, byteType);
    MakeSignedIntegerTypeOperations(symbolTable, shortType);
    MakeUnsignedIntegerTypeOperations(symbolTable, ushortType);
    MakeSignedIntegerTypeOperations(symbolTable, intType);
    MakeUnsignedIntegerTypeOperations(symbolTable, uintType);
    MakeSignedIntegerTypeOperations(symbolTable, longType);
    MakeUnsignedIntegerTypeOperations(symbolTable, ulongType);
    MakeFloatingPointTypeOperations(symbolTable, floatType);
    MakeFloatingPointTypeOperations(symbolTable, doubleType);
}

} } // namespace cmajor::symbols
