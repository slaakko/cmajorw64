// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_VALUE_INCLUDED
#define CMAJOR_SYMBOLS_VALUE_INCLUDED
#include <cmajor/parsing/Scanner.hpp>
#include <cmajor/ir/Emitter.hpp>

namespace cmajor { namespace symbols {

using cmajor::parsing::Span;
using namespace cmajor::ir;

class TypeSymbol;

enum class ValueType : uint8_t
{
    boolValue, sbyteValue, byteValue, shortValue, ushortValue, intValue, uintValue, longValue, ulongValue, floatValue, doubleValue, charValue, wcharValue, ucharValue, stringValue, nullValue,
    maxValue
};

class Value
{
public:
    Value(const Span& span_, ValueType valueType_);
    virtual ~Value();
    const Span& GetSpan() const { return span; }
    virtual llvm::Value* IrValue(Emitter& emitter) = 0;
    ValueType GetValueType() const { return valueType; }
private:
    Span span;
    ValueType valueType;
};

class BoolValue : public Value
{
public:
    BoolValue(const Span& span_, bool value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    bool value;
};

class SByteValue : public Value
{
public:
    SByteValue(const Span& span_, int8_t value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    int8_t value;
};

class ByteValue : public Value
{
public:
    ByteValue(const Span& span_, uint8_t value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    uint8_t value;
};

class ShortValue : public Value
{
public:
    ShortValue(const Span& span_, int16_t value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    int16_t value;
};

class UShortValue : public Value
{
public:
    UShortValue(const Span& span_, uint16_t value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    uint16_t value;
};

class IntValue : public Value
{
public:
    IntValue(const Span& span_, int32_t value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    int32_t value;
};

class UIntValue : public Value
{
public:
    UIntValue(const Span& span_, uint32_t value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    uint32_t value;
};

class LongValue : public Value
{
public:
    LongValue(const Span& span_, int64_t value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    int64_t value;
};

class ULongValue : public Value
{
public:
    ULongValue(const Span& span_, uint64_t value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    uint64_t value;
};

class FloatValue : public Value
{
public:
    FloatValue(const Span& span_, float value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    float value;
};

class DoubleValue : public Value
{
public:
    DoubleValue(const Span& span_, double value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    double value;
};

class CharValue : public Value
{
public:
    CharValue(const Span& span_, unsigned char value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    unsigned char value;
};

class WCharValue : public Value
{
public:
    WCharValue(const Span& span_, char16_t value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    char16_t value;
};

class UCharValue : public Value
{
public:
    UCharValue(const Span& span_, char32_t value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    char32_t value;
};

class StringValue : public Value
{
public:
    StringValue(const Span& span_, const std::string& value_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    std::string value;
};

class NullValue : public Value
{
public:
    NullValue(const Span& span_, TypeSymbol* nullPtrType_);
    llvm::Value* IrValue(Emitter& emitter) override;
private:
    TypeSymbol* nullPtrType;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_VALUE_INCLUDED
