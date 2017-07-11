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
};

class BoolTypeSymbol : public BasicTypeSymbol
{
public:
    BoolTypeSymbol(const Span& span_, const std::u32string& name_);
};

class SByteTypeSymbol : public BasicTypeSymbol
{
public:
    SByteTypeSymbol(const Span& span_, const std::u32string& name_);
};

class ByteTypeSymbol : public BasicTypeSymbol
{
public:
    ByteTypeSymbol(const Span& span_, const std::u32string& name_);
};

class ShortTypeSymbol : public BasicTypeSymbol
{
public:
    ShortTypeSymbol(const Span& span_, const std::u32string& name_);
};

class UShortTypeSymbol : public BasicTypeSymbol
{
public:
    UShortTypeSymbol(const Span& span_, const std::u32string& name_);
};

class IntTypeSymbol : public BasicTypeSymbol
{
public:
    IntTypeSymbol(const Span& span_, const std::u32string& name_);
};

class UIntTypeSymbol : public BasicTypeSymbol
{
public:
    UIntTypeSymbol(const Span& span_, const std::u32string& name_);
};

class LongTypeSymbol : public BasicTypeSymbol
{
public:
    LongTypeSymbol(const Span& span_, const std::u32string& name_);
};

class ULongTypeSymbol : public BasicTypeSymbol
{
public:
    ULongTypeSymbol(const Span& span_, const std::u32string& name_);
};

class FloatTypeSymbol : public BasicTypeSymbol
{
public:
    FloatTypeSymbol(const Span& span_, const std::u32string& name_);
};

class DoubleTypeSymbol : public BasicTypeSymbol
{
public:
    DoubleTypeSymbol(const Span& span_, const std::u32string& name_);
};

class CharTypeSymbol : public BasicTypeSymbol
{
public:
    CharTypeSymbol(const Span& span_, const std::u32string& name_);
};

class WCharTypeSymbol : public BasicTypeSymbol
{
public:
    WCharTypeSymbol(const Span& span_, const std::u32string& name_);
};

class UCharTypeSymbol : public BasicTypeSymbol
{
public:
    UCharTypeSymbol(const Span& span_, const std::u32string& name_);
};

class VoidTypeSymbol : public BasicTypeSymbol
{
public:
    VoidTypeSymbol(const Span& span_, const std::u32string& name_);
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_BASIC_TYPE_SYMBOL_INCLUDED
