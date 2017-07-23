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
};

class BoolTypeSymbol : public BasicTypeSymbol
{
public:
    BoolTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "bool"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt1Ty(emitter.Context()); }
};

class SByteTypeSymbol : public BasicTypeSymbol
{
public:
    SByteTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "sbyte"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt8Ty(emitter.Context()); }
};

class ByteTypeSymbol : public BasicTypeSymbol
{
public:
    ByteTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "byte"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt8Ty(emitter.Context()); }
    bool IsUnsignedType() const override { return true; }
};

class ShortTypeSymbol : public BasicTypeSymbol
{
public:
    ShortTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "short"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt16Ty(emitter.Context()); }
};

class UShortTypeSymbol : public BasicTypeSymbol
{
public:
    UShortTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "ushort"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt16Ty(emitter.Context()); }
    bool IsUnsignedType() const override { return true; }
};

class IntTypeSymbol : public BasicTypeSymbol
{
public:
    IntTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "int"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt32Ty(emitter.Context()); }
};

class UIntTypeSymbol : public BasicTypeSymbol
{
public:
    UIntTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "uint"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt32Ty(emitter.Context()); }
    bool IsUnsignedType() const override { return true; }
};

class LongTypeSymbol : public BasicTypeSymbol
{
public:
    LongTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "long"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt64Ty(emitter.Context()); }
};

class ULongTypeSymbol : public BasicTypeSymbol
{
public:
    ULongTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "ulong"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt64Ty(emitter.Context()); }
    bool IsUnsignedType() const override { return true; }
};

class FloatTypeSymbol : public BasicTypeSymbol
{
public:
    FloatTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "float"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getFloatTy(emitter.Context()); }
};

class DoubleTypeSymbol : public BasicTypeSymbol
{
public:
    DoubleTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "double"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getDoubleTy(emitter.Context()); }
};

class CharTypeSymbol : public BasicTypeSymbol
{
public:
    CharTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "char"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt8Ty(emitter.Context()); }
};

class WCharTypeSymbol : public BasicTypeSymbol
{
public:
    WCharTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "wchar"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt16Ty(emitter.Context()); }
};

class UCharTypeSymbol : public BasicTypeSymbol
{
public:
    UCharTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "uchar"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getInt32Ty(emitter.Context()); }
};

class VoidTypeSymbol : public BasicTypeSymbol
{
public:
    VoidTypeSymbol(const Span& span_, const std::u32string& name_);
    std::string TypeString() const override { return "void"; }
    llvm::Type* IrType(Emitter& emitter) const override { return llvm::Type::getVoidTy(emitter.Context()); }
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_BASIC_TYPE_SYMBOL_INCLUDED
