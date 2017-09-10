// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_BASIC_TYPE_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_BASIC_TYPE_SYMBOL_INCLUDED
#include <cmajor/symbols/TypeSymbol.hpp>

namespace cmajor { namespace symbols {

class BasicTypeSymbol : public TypeSymbol
{
public:
    BasicTypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "basic_type"; }
    bool IsBasicTypeSymbol() const override { return true; }
};

class BoolTypeSymbol : public BasicTypeSymbol
{
public:
    BoolTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "bool"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt1Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt1(false); }
    bool IsSwitchConditionType() const override { return true; }
};

class SByteTypeSymbol : public BasicTypeSymbol
{
public:
    SByteTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "sbyte"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt8Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt8(0); }
    bool IsIntegralType() const override { return true; }
    bool IsSwitchConditionType() const override { return true; }
};

class ByteTypeSymbol : public BasicTypeSymbol
{
public:
    ByteTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "byte"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt8Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt8(0); }
    bool IsIntegralType() const override { return true; }
    bool IsUnsignedType() const override { return true; }
    bool IsSwitchConditionType() const override { return true; }
};

class ShortTypeSymbol : public BasicTypeSymbol
{
public:
    ShortTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "short"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt16Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt16(0); }
    bool IsIntegralType() const override { return true; }
    bool IsSwitchConditionType() const override { return true; }
};

class UShortTypeSymbol : public BasicTypeSymbol
{
public:
    UShortTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "ushort"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt16Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt16(0); }
    bool IsIntegralType() const override { return true; }
    bool IsUnsignedType() const override { return true; }
    bool IsSwitchConditionType() const override { return true; }
};

class IntTypeSymbol : public BasicTypeSymbol
{
public:
    IntTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "int"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt32Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt32(0); }
    bool IsIntegralType() const override { return true; }
    bool IsSwitchConditionType() const override { return true; }
};

class UIntTypeSymbol : public BasicTypeSymbol
{
public:
    UIntTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "uint"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt32Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt32(0); }
    bool IsIntegralType() const override { return true; }
    bool IsUnsignedType() const override { return true; }
    bool IsSwitchConditionType() const override { return true; }
};

class LongTypeSymbol : public BasicTypeSymbol
{
public:
    LongTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "long"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt64Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt64(0); }
    bool IsIntegralType() const override { return true; }
    bool IsSwitchConditionType() const override { return true; }
};

class ULongTypeSymbol : public BasicTypeSymbol
{
public:
    ULongTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "ulong"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt64Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt64(0); }
    bool IsIntegralType() const override { return true; }
    bool IsUnsignedType() const override { return true; }
    bool IsSwitchConditionType() const override { return true; }
};

class FloatTypeSymbol : public BasicTypeSymbol
{
public:
    FloatTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "float"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getFloatTy(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return llvm::ConstantFP::get(llvm::Type::getFloatTy(emitter.Context()), 0.0); }
};

class DoubleTypeSymbol : public BasicTypeSymbol
{
public:
    DoubleTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "double"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getDoubleTy(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return llvm::ConstantFP::get(llvm::Type::getDoubleTy(emitter.Context()), 0.0); }
};

class CharTypeSymbol : public BasicTypeSymbol
{
public:
    CharTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "char"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt8Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt8(0); }
    bool IsSwitchConditionType() const override { return true; }
};

class WCharTypeSymbol : public BasicTypeSymbol
{
public:
    WCharTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "wchar"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt16Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt16(0); }
    bool IsSwitchConditionType() const override { return true; }
};

class UCharTypeSymbol : public BasicTypeSymbol
{
public:
    UCharTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "uchar"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getInt32Ty(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { return emitter.Builder().getInt32(0); }
    bool IsSwitchConditionType() const override { return true; }
};

class VoidTypeSymbol : public BasicTypeSymbol
{
public:
    VoidTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "void"; }
    llvm::Type* IrType(Emitter& emitter) override { return llvm::Type::getVoidTy(emitter.Context()); }
    llvm::Constant* CreateDefaultIrValue(Emitter& emitter) override { Assert(false, "tried to create default value of void"); return llvm::Constant::getNullValue(emitter.Builder().getInt8PtrTy()); }
    bool IsVoidType() const override { return true; }
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_BASIC_TYPE_SYMBOL_INCLUDED
