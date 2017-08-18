// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/Value.hpp>
#include <cmajor/symbols/TypeSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>

namespace cmajor { namespace symbols {

const char* valueTypeStr[]
{
    "none", "bool", "sbyte", "byte", "short", "ushort", "int", "uint", "long", "ulong", "float", "double", "char", "wchar", "uchar", "string", "wstring", "ustring", "null"
};

std::string ValueTypeStr(ValueType valueType)
{
    return valueTypeStr[uint8_t(valueType)];
}

ValueType commonType[uint8_t(ValueType::maxValue)][uint8_t(ValueType::maxValue)] = 
{
    // ValueType::none
    {   
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::boolValue
    {   
        ValueType::none, ValueType::boolValue, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none,
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },
    
    // ValueType::sbyteValue
    {   
        ValueType::none, ValueType::none, ValueType::sbyteValue, ValueType::shortValue, ValueType::shortValue, ValueType::intValue, ValueType::intValue, ValueType::longValue,
        ValueType::longValue, ValueType::none, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::byteValue
    {
        ValueType::none, ValueType::none, ValueType::shortValue, ValueType::byteValue, ValueType::shortValue, ValueType::ushortValue, ValueType::intValue, ValueType::uintValue,
        ValueType::longValue, ValueType::ulongValue, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::shortValue
    {
        ValueType::none, ValueType::none, ValueType::shortValue, ValueType::shortValue, ValueType::shortValue, ValueType::intValue, ValueType::intValue, ValueType::longValue,
        ValueType::longValue, ValueType::none, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none,
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::ushortValue
    {
        ValueType::none, ValueType::none, ValueType::intValue, ValueType::ushortValue, ValueType::intValue, ValueType::ushortValue, ValueType::intValue, ValueType::uintValue,
        ValueType::longValue, ValueType::ulongValue, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::intValue
    {
        ValueType::none, ValueType::none, ValueType::intValue, ValueType::intValue, ValueType::intValue, ValueType::intValue, ValueType::intValue, ValueType::longValue,
        ValueType::longValue, ValueType::none, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::uintValue
    {
        ValueType::none, ValueType::none, ValueType::longValue, ValueType::uintValue, ValueType::longValue, ValueType::uintValue, ValueType::longValue, ValueType::uintValue,
        ValueType::longValue, ValueType::ulongValue, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::longValue
    {
        ValueType::none, ValueType::none, ValueType::longValue, ValueType::longValue, ValueType::longValue, ValueType::longValue, ValueType::longValue, ValueType::longValue,
        ValueType::longValue, ValueType::none, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::ulongValue
    {
        ValueType::none, ValueType::none, ValueType::none, ValueType::ulongValue, ValueType::none, ValueType::ulongValue, ValueType::none, ValueType::ulongValue,
        ValueType::none, ValueType::ulongValue, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::floatValue
    {
        ValueType::none, ValueType::none, ValueType::floatValue, ValueType::floatValue, ValueType::floatValue, ValueType::floatValue, ValueType::floatValue, ValueType::floatValue,
        ValueType::floatValue, ValueType::floatValue, ValueType::floatValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::doubleValue
    {
        ValueType::none, ValueType::none, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue,
        ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::doubleValue, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::charValue
    {
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::charValue, ValueType::wcharValue, ValueType::ucharValue, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::wcharValue
    {
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none,
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::wcharValue, ValueType::wcharValue, ValueType::ucharValue, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::ucharValue
    {
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none,
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::ucharValue, ValueType::ucharValue, ValueType::ucharValue, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::stringValue
    {
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none,
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::stringValue, ValueType::none, ValueType::none, ValueType::none
    },

    // ValueType::wstringValue
    {
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none,
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none,
        ValueType::none, ValueType::wstringValue, ValueType::none, ValueType::none
    },

    // ValueType::ustringValue
    {
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none,
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none,
        ValueType::none, ValueType::none, ValueType::ustringValue, ValueType::none
    },
        
    // ValueType::nullValue
    {
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none,
        ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, ValueType::none, 
        ValueType::none, ValueType::none, ValueType::none, ValueType::nullValue
    }
};

ValueType CommonType(ValueType left, ValueType right)
{
    return commonType[uint8_t(left)][uint8_t(right)];
}

ValueType GetValueTypeFor(SymbolType symbolType)
{
    switch (symbolType)
    {
        case SymbolType::boolTypeSymbol: return ValueType::boolValue;
        case SymbolType::sbyteTypeSymbol: return ValueType::sbyteValue;
        case SymbolType::byteTypeSymbol: return ValueType::byteValue;
        case SymbolType::shortTypeSymbol: return ValueType::shortValue;
        case SymbolType::ushortTypeSymbol: return ValueType::ushortValue;
        case SymbolType::intTypeSymbol: return ValueType::intValue;
        case SymbolType::uintTypeSymbol: return ValueType::uintValue;
        case SymbolType::longTypeSymbol: return ValueType::longValue;
        case SymbolType::ulongTypeSymbol: return ValueType::ulongValue;
        case SymbolType::floatTypeSymbol: return ValueType::floatValue;
        case SymbolType::doubleTypeSymbol: return ValueType::doubleValue;
        case SymbolType::charTypeSymbol: return ValueType::charValue;
        case SymbolType::wcharTypeSymbol: return ValueType::wcharValue;
        case SymbolType::ucharTypeSymbol: return ValueType::ucharValue;
        default: return ValueType::none;
    }
}

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

Value* BoolValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue: 
        {
            return new BoolValue(span, value);
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(span, static_cast<int16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(span, static_cast<int32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(span, static_cast<int64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(span, static_cast<uint64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::floatValue:
        {
            if (cast)
            {
                return new FloatValue(span, static_cast<float>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::doubleValue:
        {
            if (cast)
            {
                return new DoubleValue(span, static_cast<double>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

SByteValue::SByteValue(const Span& span_, int8_t value_) : Value(span_, ValueType::sbyteValue), value(value_)
{
}

llvm::Value* SByteValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt8(static_cast<uint8_t>(value));
}

Value* SByteValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            return new SByteValue(span, value);
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            return new ShortValue(span, value);
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            return new IntValue(span, value);
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::longValue:
        {
            return new LongValue(span, value);
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(span, static_cast<uint64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::floatValue:
        {
            return new FloatValue(span, value);
        }
        case ValueType::doubleValue:
        {
            return new FloatValue(span, value);
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

ByteValue::ByteValue(const Span& span_, uint8_t value_) : Value(span_, ValueType::byteValue), value(value_)
{
}

llvm::Value* ByteValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt8(value);
}

Value* ByteValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            return new ByteValue(span, value);
        }
        case ValueType::shortValue:
        {
            return new ShortValue(span, value);
        }
        case ValueType::ushortValue:
        {
            return new UShortValue(span, value);
        }
        case ValueType::intValue:
        {
            return new IntValue(span, value);
        }
        case ValueType::uintValue:
        {
            return new UIntValue(span, value);
        }
        case ValueType::longValue:
        {
            return new LongValue(span, value);
        }
        case ValueType::ulongValue:
        {
            return new ULongValue(span, value);
        }
        case ValueType::floatValue:
        {
            return new FloatValue(span, value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(span, value);
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

ShortValue::ShortValue(const Span& span_, int16_t value_) : Value(span_, ValueType::shortValue), value(value_)
{
}

llvm::Value* ShortValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt16(static_cast<uint16_t>(value));
}

Value* ShortValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            return new ShortValue(span, value);
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            return new IntValue(span, value);
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::longValue:
        {
            return new LongValue(span, value);
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(span, static_cast<uint64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::floatValue:
        {
            return new FloatValue(span, value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(span, value);
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

UShortValue::UShortValue(const Span& span_, uint16_t value_) : Value(span_, ValueType::ushortValue), value(value_)
{
}

llvm::Value* UShortValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt16(value);
}

Value* UShortValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(span, static_cast<int16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ushortValue:
        {
            return new UShortValue(span, value);
        }
        case ValueType::intValue:
        {
            return new IntValue(span, value);
        }
        case ValueType::uintValue:
        {
            return new UIntValue(span, value);
        }
        case ValueType::longValue:
        {
            return new LongValue(span, value);
        }
        case ValueType::ulongValue:
        {
            return new ULongValue(span, value);
        }
        case ValueType::floatValue:
        {
            return new FloatValue(span, value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(span, value);
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

IntValue::IntValue(const Span& span_, int32_t value_) : Value(span_, ValueType::intValue), value(value_)
{
}

llvm::Value* IntValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt32(static_cast<uint32_t>(value));
}

Value* IntValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(span, static_cast<int16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            return new IntValue(span, value);
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::longValue:
        {
            return new LongValue(span, value);
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(span, static_cast<uint64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::floatValue:
        {
            return new FloatValue(span, value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(span, value);
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

UIntValue::UIntValue(const Span& span_, uint32_t value_) : Value(span_, ValueType::uintValue), value(value_)
{
}

llvm::Value* UIntValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt32(value);
}

Value* UIntValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(span, static_cast<int16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(span, static_cast<int32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::uintValue:
        {
            return new UIntValue(span, value);
        }
        case ValueType::longValue:
        {
            return new LongValue(span, value);
        }
        case ValueType::ulongValue:
        {
            return new ULongValue(span, value);
        }
        case ValueType::floatValue:
        {
            return new FloatValue(span, value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(span, value);
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

LongValue::LongValue(const Span& span_, int64_t value_) : Value(span_, ValueType::longValue), value(value_)
{
}

llvm::Value* LongValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt64(static_cast<uint64_t>(value));
}

Value* LongValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(span, static_cast<int16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(span, static_cast<int32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::longValue:
        {
            return new LongValue(span, value);
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(span, static_cast<uint64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::floatValue:
        {
            return new FloatValue(span, value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(span, value);
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

ULongValue::ULongValue(const Span& span_, uint64_t value_) : Value(span_, ValueType::ulongValue), value(value_)
{
}

llvm::Value* ULongValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt64(value);
}

Value* ULongValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(span, static_cast<int16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(span, static_cast<int32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(span, static_cast<int64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ulongValue:
        {
            return new ULongValue(span, value);
        }
        case ValueType::floatValue:
        {
            return new FloatValue(span, value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(span, value);
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

FloatValue::FloatValue(const Span& span_, float value_) : Value(span_, ValueType::floatValue), value(value_)
{
}

llvm::Value* FloatValue::IrValue(Emitter& emitter)
{
    return llvm::ConstantFP::get(emitter.Builder().getFloatTy(), value);
}

Value* FloatValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(span, static_cast<int16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(span, static_cast<int32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(span, static_cast<int64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(span, static_cast<uint64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::floatValue:
        {
            return new FloatValue(span, value);
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(span, value);
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

DoubleValue::DoubleValue(const Span& span_, double value_) : Value(span_, ValueType::doubleValue), value(value_)
{
}

llvm::Value* DoubleValue::IrValue(Emitter& emitter)
{
    return llvm::ConstantFP::get(emitter.Builder().getDoubleTy(), value);
}

Value* DoubleValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(span, static_cast<int16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(span, static_cast<int32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(span, static_cast<int64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(span, static_cast<uint64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::floatValue:
        {
            if (cast)
            {
                return new FloatValue(span, static_cast<float>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::doubleValue:
        {
            return new DoubleValue(span, value);
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            if (cast)
            {
                return new UCharValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

CharValue::CharValue(const Span& span_, unsigned char value_) : Value(span_, ValueType::charValue), value(value_)
{
}

llvm::Value* CharValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt8(static_cast<uint8_t>(value));
}

Value* CharValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(span, static_cast<int16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(span, static_cast<int32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(span, static_cast<int64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(span, static_cast<uint64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::floatValue:
        {
            if (cast)
            {
                return new FloatValue(span, static_cast<float>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::doubleValue:
        {
            if (cast)
            {
                return new DoubleValue(span, static_cast<double>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::charValue:
        {
            return new CharValue(span, value);
        }
        case ValueType::wcharValue:
        {
            return new WCharValue(span, value);
        }
        case ValueType::ucharValue:
        {
            return new UCharValue(span, value);
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

WCharValue::WCharValue(const Span& span_, char16_t value_) : Value(span_, ValueType::wcharValue), value(value_)
{
}

llvm::Value* WCharValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt16(static_cast<uint16_t>(value));
}

Value* WCharValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(span, static_cast<int16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(span, static_cast<int32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(span, static_cast<int64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(span, static_cast<uint64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::floatValue:
        {
            if (cast)
            {
                return new FloatValue(span, static_cast<float>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::doubleValue:
        {
            if (cast)
            {
                return new DoubleValue(span, static_cast<double>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            return new WCharValue(span, value);
        }
        case ValueType::ucharValue:
        {
            return new UCharValue(span, value);
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

UCharValue::UCharValue(const Span& span_, char32_t value_) : Value(span_, ValueType::ucharValue), value(value_)
{
}

llvm::Value* UCharValue::IrValue(Emitter& emitter)
{
    return emitter.Builder().getInt32(static_cast<uint32_t>(value));
}

Value* UCharValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::boolValue:
        {
            if (cast)
            {
                return new BoolValue(span, static_cast<bool>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::sbyteValue:
        {
            if (cast)
            {
                return new SByteValue(span, static_cast<int8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::byteValue:
        {
            if (cast)
            {
                return new ByteValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::shortValue:
        {
            if (cast)
            {
                return new ShortValue(span, static_cast<int16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ushortValue:
        {
            if (cast)
            {
                return new UShortValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::intValue:
        {
            if (cast)
            {
                return new IntValue(span, static_cast<int32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::uintValue:
        {
            if (cast)
            {
                return new UIntValue(span, static_cast<uint32_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::longValue:
        {
            if (cast)
            {
                return new LongValue(span, static_cast<int64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ulongValue:
        {
            if (cast)
            {
                return new ULongValue(span, static_cast<uint64_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::floatValue:
        {
            if (cast)
            {
                return new FloatValue(span, static_cast<float>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::doubleValue:
        {
            if (cast)
            {
                return new DoubleValue(span, static_cast<double>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::charValue:
        {
            if (cast)
            {
                return new CharValue(span, static_cast<uint8_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::wcharValue:
        {
            if (cast)
            {
                return new WCharValue(span, static_cast<uint16_t>(value));
            }
            else
            {
                if (dontThrow)
                {
                    return nullptr;
                }
                else
                {
                    throw Exception("cannot convert " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " without a cast", span);
                }
            }
        }
        case ValueType::ucharValue:
        {
            return new UCharValue(span, value);
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

StringValue::StringValue(const Span& span_, int stringId_) : Value(span_, ValueType::stringValue), stringId(stringId_)
{
}

llvm::Value* StringValue::IrValue(Emitter& emitter)
{
    return emitter.GetGlobalStringPtr(stringId);
}

Value* StringValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::stringValue:
        {
            return new StringValue(span, stringId);
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

WStringValue::WStringValue(const Span& span_, int stringId_) : Value(span_, ValueType::wstringValue), stringId(stringId_)
{
}

llvm::Value* WStringValue::IrValue(Emitter& emitter)
{ 
    llvm::Value* wstringConstant = emitter.GetGlobalWStringConstant(stringId);
    ArgVector indeces;
    indeces.push_back(emitter.Builder().getInt32(0));
    indeces.push_back(emitter.Builder().getInt32(0));
    return emitter.Builder().CreateGEP(wstringConstant, indeces);
}

Value* WStringValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::wstringValue:
        {
            return new WStringValue(span, stringId);
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

UStringValue::UStringValue(const Span& span_, int stringId_) : Value(span_, ValueType::ustringValue), stringId(stringId_)
{
}

llvm::Value* UStringValue::IrValue(Emitter& emitter)
{
    llvm::Value* ustringConstant = emitter.GetGlobalUStringConstant(stringId);
    ArgVector indeces;
    indeces.push_back(emitter.Builder().getInt32(0));
    indeces.push_back(emitter.Builder().getInt32(0));
    return emitter.Builder().CreateGEP(ustringConstant, indeces);
}

Value* UStringValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::ustringValue:
        {
            return new UStringValue(span, stringId);
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

NullValue::NullValue(const Span& span_, TypeSymbol* nullPtrType_) : Value(span_, ValueType::nullValue), nullPtrType(nullPtrType_)
{
}

llvm::Value* NullValue::IrValue(Emitter& emitter)
{
    return llvm::Constant::getNullValue(nullPtrType->IrType(emitter));
}

Value* NullValue::As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const
{
    switch (targetType)
    {
        case ValueType::nullValue:
        {
            return new NullValue(span, nullPtrType);
        }
        default:
        {
            if (dontThrow)
            {
                return nullptr;
            }
            else
            {
                throw Exception("conversion from " + ValueTypeStr(GetValueType()) + " to " + ValueTypeStr(targetType) + " is not valid", span);
            }
        }
    }
}

bool operator==(IntegralValue left, IntegralValue right)
{
    if (left.value->GetValueType() != right.value->GetValueType()) return false;
    switch (left.value->GetValueType())
    {
        case ValueType::boolValue: return ValuesEqual(*static_cast<BoolValue*>(left.value), *static_cast<BoolValue*>(right.value));
        case ValueType::sbyteValue: return ValuesEqual(*static_cast<SByteValue*>(left.value), *static_cast<SByteValue*>(right.value));
        case ValueType::byteValue: return ValuesEqual(*static_cast<ByteValue*>(left.value), *static_cast<ByteValue*>(right.value));
        case ValueType::shortValue: return ValuesEqual(*static_cast<ShortValue*>(left.value), *static_cast<ShortValue*>(right.value));
        case ValueType::ushortValue: return ValuesEqual(*static_cast<UShortValue*>(left.value), *static_cast<UShortValue*>(right.value));
        case ValueType::intValue: return ValuesEqual(*static_cast<IntValue*>(left.value), *static_cast<IntValue*>(right.value));
        case ValueType::uintValue: return ValuesEqual(*static_cast<UIntValue*>(left.value), *static_cast<UIntValue*>(right.value));
        case ValueType::longValue: return ValuesEqual(*static_cast<LongValue*>(left.value), *static_cast<LongValue*>(right.value));
        case ValueType::ulongValue: return ValuesEqual(*static_cast<ULongValue*>(left.value), *static_cast<ULongValue*>(right.value));
        case ValueType::charValue: return ValuesEqual(*static_cast<CharValue*>(left.value), *static_cast<CharValue*>(right.value));
        case ValueType::wcharValue: return ValuesEqual(*static_cast<WCharValue*>(left.value), *static_cast<WCharValue*>(right.value));
        case ValueType::ucharValue: return ValuesEqual(*static_cast<UCharValue*>(left.value), *static_cast<UCharValue*>(right.value));
    }
    return false;
}

size_t IntegralValueHash::operator()(IntegralValue integralValue) const
{
    switch (integralValue.value->GetValueType())
    {
        case ValueType::boolValue: return GetHashCode(*static_cast<BoolValue*>(integralValue.value));
        case ValueType::sbyteValue: return GetHashCode(*static_cast<SByteValue*>(integralValue.value));
        case ValueType::byteValue: return GetHashCode(*static_cast<ByteValue*>(integralValue.value));
        case ValueType::shortValue: return GetHashCode(*static_cast<ShortValue*>(integralValue.value));
        case ValueType::ushortValue: return GetHashCode(*static_cast<UShortValue*>(integralValue.value));
        case ValueType::intValue: return GetHashCode(*static_cast<IntValue*>(integralValue.value));
        case ValueType::uintValue: return GetHashCode(*static_cast<UIntValue*>(integralValue.value));
        case ValueType::longValue: return GetHashCode(*static_cast<LongValue*>(integralValue.value));
        case ValueType::ulongValue: return GetHashCode(*static_cast<ULongValue*>(integralValue.value));
        case ValueType::charValue: return GetHashCode(*static_cast<CharValue*>(integralValue.value));
        case ValueType::wcharValue: return GetHashCode(*static_cast<WCharValue*>(integralValue.value));
        case ValueType::ucharValue: return GetHashCode(*static_cast<UCharValue*>(integralValue.value));
    }
    return 0;
}

} } // namespace cmajor::symbols
