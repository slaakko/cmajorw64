// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_VALUE_INCLUDED
#define CMAJOR_SYMBOLS_VALUE_INCLUDED
#include <cmajor/symbols/Symbol.hpp>
#include <cmajor/parsing/Scanner.hpp>
#include <cmajor/ir/Emitter.hpp>

namespace cmajor { namespace symbols {

using cmajor::parsing::Span;
using namespace cmajor::ir;

class TypeSymbol;

enum class ValueType : uint8_t
{
    none, boolValue, sbyteValue, byteValue, shortValue, ushortValue, intValue, uintValue, 
    longValue, ulongValue, floatValue, doubleValue, charValue, wcharValue, ucharValue, 
    stringValue, wstringValue, ustringValue, nullValue,
    maxValue
};

std::string ValueTypeStr(ValueType valueType);

ValueType CommonType(ValueType left, ValueType right);

ValueType GetValueTypeFor(SymbolType symbolType);

class Value
{
public:
    Value(const Span& span_, ValueType valueType_);
    virtual ~Value();
    virtual Value* Clone() = 0;
    virtual Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const = 0;
    virtual llvm::Value* IrValue(Emitter& emitter) = 0;
    virtual void Write(BinaryWriter& writer) = 0;
    virtual void Read(BinaryReader& reader) = 0;
    virtual bool IsComplete() const { return true; }
    virtual bool IsScopedValue() const { return false; }
    virtual std::string ToString() const { return std::string(); }
    const Span& GetSpan() const { return span; }
    ValueType GetValueType() const { return valueType; }
private:
    Span span;
    ValueType valueType;
};

class BoolValue : public Value
{
public:
    typedef bool OperandType;
    BoolValue(const Span& span_, bool value_);
    Value* Clone() override { return new BoolValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return value ? "true" : "false"; }
    bool GetValue() const { return value; }
private:
    bool value;
};

class SByteValue : public Value
{
public:
    typedef int8_t OperandType;
    SByteValue(const Span& span_, int8_t value_);
    Value* Clone() override { return new SByteValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    int8_t GetValue() const { return value; }
private:
    int8_t value;
};

class ByteValue : public Value
{
public:
    typedef uint8_t OperandType;
    ByteValue(const Span& span_, uint8_t value_);
    Value* Clone() override { return new ByteValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    uint8_t GetValue() const { return value; }
private:
    uint8_t value;
};

class ShortValue : public Value
{
public:
    typedef int16_t OperandType;
    ShortValue(const Span& span_, int16_t value_);
    Value* Clone() override { return new ShortValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    int16_t GetValue() const { return value; }
private:
    int16_t value;
};

class UShortValue : public Value
{
public:
    typedef uint16_t OperandType;
    UShortValue(const Span& span_, uint16_t value_);
    Value* Clone() override { return new UShortValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    uint16_t GetValue() const { return value; }
private:
    uint16_t value;
};

class IntValue : public Value
{
public:
    typedef int32_t OperandType;
    IntValue(const Span& span_, int32_t value_);
    Value* Clone() override { return new IntValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    int32_t GetValue() const { return value; }
private:
    int32_t value;
};

class UIntValue : public Value
{
public:
    typedef uint32_t OperandType;
    UIntValue(const Span& span_, uint32_t value_);
    Value* Clone() override { return new UIntValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    uint32_t GetValue() const { return value; }
private:
    uint32_t value;
};

class LongValue : public Value
{
public:
    typedef int64_t OperandType;
    LongValue(const Span& span_, int64_t value_);
    Value* Clone() override { return new LongValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    int64_t GetValue() const { return value; }
private:
    int64_t value;
};

class ULongValue : public Value
{
public:
    typedef uint64_t OperandType;
    ULongValue(const Span& span_, uint64_t value_);
    Value* Clone() override { return new ULongValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    uint64_t GetValue() const { return value; }
private:
    uint64_t value;
};

class FloatValue : public Value
{
public:
    typedef float OperandType;
    FloatValue(const Span& span_, float value_);
    Value* Clone() override { return new FloatValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    float GetValue() const { return value; }
private:
    float value;
};

class DoubleValue : public Value
{
public:
    typedef double OperandType;
    DoubleValue(const Span& span_, double value_);
    Value* Clone() override { return new DoubleValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    double GetValue() const { return value; }
private:
    double value;
};

class CharValue : public Value
{
public:
    typedef unsigned char OperandType;
    CharValue(const Span& span_, unsigned char value_);
    Value* Clone() override { return new CharValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    unsigned char GetValue() const { return value; }
private:
    unsigned char value;
};

class WCharValue : public Value
{
public:
    typedef char16_t OperandType;
    WCharValue(const Span& span_, char16_t value_);
    Value* Clone() override { return new WCharValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    char16_t GetValue() const { return value; }
private:
    char16_t value;
};

class UCharValue : public Value
{
public:
    typedef char32_t OperandType;
    UCharValue(const Span& span_, char32_t value_);
    Value* Clone() override { return new UCharValue(GetSpan(), value); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return std::to_string(value); }
    char32_t GetValue() const { return value; }
private:
    char32_t value;
};

class StringValue : public Value
{
public:
    StringValue(const Span& span_, int stringId_);
    Value* Clone() override { return new StringValue(GetSpan(), stringId); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
private:
    int stringId;
};

class WStringValue : public Value
{
public:
    WStringValue(const Span& span_, int stringId_);
    Value* Clone() override { return new WStringValue(GetSpan(), stringId); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
private:
    int stringId;
};

class UStringValue : public Value
{
public:
    UStringValue(const Span& span_, int stringId_);
    Value* Clone() override { return new UStringValue(GetSpan(), stringId); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
private:
    int stringId;
};

class NullValue : public Value
{
public:
    NullValue(const Span& span_, TypeSymbol* nullPtrType_);
    Value* Clone() override { return new NullValue(GetSpan(), nullPtrType); }
    llvm::Value* IrValue(Emitter& emitter) override;
    void Write(BinaryWriter& writer) override;
    void Read(BinaryReader& reader) override;
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override;
    std::string ToString() const override { return "null"; }
private:
    TypeSymbol* nullPtrType;
};

template<typename ValueT>
inline bool ValuesEqual(const ValueT& left, const ValueT& right)
{
    return left.GetValue() == right.GetValue();
}

template<typename ValueT>
inline size_t GetHashCode(const ValueT& value)
{
    return static_cast<size_t>(value.GetValue());
}

struct IntegralValue
{
    IntegralValue(Value* value_) : value(value_) {}
    Value* value;
};

bool operator==(IntegralValue left, IntegralValue right);

inline bool operator!=(IntegralValue left, IntegralValue right)
{
    return !(left == right);
}

struct IntegralValueHash
{
    size_t operator()(IntegralValue integralValue) const;
};

void WriteValue(Value* value, BinaryWriter& writer);
Value* ReadValue(BinaryReader& reader);

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_VALUE_INCLUDED
