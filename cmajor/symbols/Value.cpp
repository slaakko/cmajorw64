// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/Value.hpp>

namespace cmajor { namespace symbols {

Value::Value(const Span& span_, ValueType valueType_) : span(span_), valueType(valueType_)
{
}

Value::~Value()
{
}

BoolValue::BoolValue(const Span& span_, bool value_) : Value(span_, ValueType::boolValue), value(value_)
{
}

llvm::Value* BoolValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt1(value);
}

SByteValue::SByteValue(const Span& span_, int8_t value_) : Value(span_, ValueType::sbyteValue), value(value_)
{
}

llvm::Value* SByteValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt8(static_cast<uint8_t>(value));
}

ByteValue::ByteValue(const Span& span_, uint8_t value_) : Value(span_, ValueType::byteValue), value(value_)
{
}

llvm::Value* ByteValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt8(value);
}

ShortValue::ShortValue(const Span& span_, int16_t value_) : Value(span_, ValueType::shortValue), value(value_)
{
}

llvm::Value* ShortValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt16(static_cast<uint16_t>(value));
}

UShortValue::UShortValue(const Span& span_, uint16_t value_) : Value(span_, ValueType::ushortValue), value(value_)
{
}

llvm::Value* UShortValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt16(value);
}

IntValue::IntValue(const Span& span_, int32_t value_) : Value(span_, ValueType::intValue), value(value_)
{
}

llvm::Value* IntValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt32(static_cast<uint32_t>(value));
}

UIntValue::UIntValue(const Span& span_, uint32_t value_) : Value(span_, ValueType::uintValue), value(value_)
{
}

llvm::Value* UIntValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt32(value);
}

LongValue::LongValue(const Span& span_, int64_t value_) : Value(span_, ValueType::longValue), value(value_)
{
}

llvm::Value* LongValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt64(static_cast<uint64_t>(value));
}

ULongValue::ULongValue(const Span& span_, uint64_t value_) : Value(span_, ValueType::ulongValue), value(value_)
{
}

llvm::Value* ULongValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt64(value);
}

FloatValue::FloatValue(const Span& span_, float value_) : Value(span_, ValueType::floatValue), value(value_)
{
}

llvm::Value* FloatValue::IrValue(Emitter& emitter)
{
    return llvm::ConstantFP::get(emitter.Builder().getFloatTy(), value);
}

DoubleValue::DoubleValue(const Span& span_, double value_) : Value(span_, ValueType::doubleValue), value(value_)
{
}

llvm::Value* DoubleValue::IrValue(Emitter& emitter)
{
    return llvm::ConstantFP::get(emitter.Builder().getDoubleTy(), value);
}

CharValue::CharValue(const Span& span_, unsigned char value_) : Value(span_, ValueType::charValue), value(value_)
{
}

llvm::Value* CharValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt8(static_cast<uint8_t>(value));
}

WCharValue::WCharValue(const Span& span_, char16_t value_) : Value(span_, ValueType::wcharValue), value(value_)
{
}

llvm::Value* WCharValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt16(static_cast<uint16_t>(value));
}

UCharValue::UCharValue(const Span& span_, char32_t value_) : Value(span_, ValueType::ucharValue), value(value_)
{
}

llvm::Value* UCharValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt32(static_cast<uint32_t>(value));
}

} } // namespace cmajor::symbols
