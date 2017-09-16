// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/BasicTypeSymbol.hpp>
#include <cmajor/symbols/SymbolCollector.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

    using namespace cmajor::unicode;

BasicTypeSymbol::BasicTypeSymbol(SymbolType symbolType_, const Span& span_, const std::u32string& name_) : TypeSymbol(symbolType_, span_, name_)
{
}

void BasicTypeSymbol::Accept(SymbolCollector* collector)
{
    if (IsProject())
    {
        collector->AddBasicType(this);
    }
}

void BasicTypeSymbol::Dump(CodeFormatter& formatter)
{
    formatter.WriteLine(ToUtf8(Name()));
    formatter.WriteLine("typeid: " + std::to_string(TypeId()));
}

BoolTypeSymbol::BoolTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::boolTypeSymbol, span_, name_)
{
}

SByteTypeSymbol::SByteTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::sbyteTypeSymbol, span_, name_)
{
}

ByteTypeSymbol::ByteTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::byteTypeSymbol, span_, name_)
{
}

ShortTypeSymbol::ShortTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::shortTypeSymbol, span_, name_)
{
}

UShortTypeSymbol::UShortTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::ushortTypeSymbol, span_, name_)
{
}

IntTypeSymbol::IntTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::intTypeSymbol, span_, name_)
{
}

UIntTypeSymbol::UIntTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::uintTypeSymbol, span_, name_)
{
}

LongTypeSymbol::LongTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::longTypeSymbol, span_, name_)
{
}

ULongTypeSymbol::ULongTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::ulongTypeSymbol, span_, name_)
{
}

FloatTypeSymbol::FloatTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::floatTypeSymbol, span_, name_)
{
}

DoubleTypeSymbol::DoubleTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::doubleTypeSymbol, span_, name_)
{
}

CharTypeSymbol::CharTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::charTypeSymbol, span_, name_)
{
}

WCharTypeSymbol::WCharTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::wcharTypeSymbol, span_, name_)
{
}

UCharTypeSymbol::UCharTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::ucharTypeSymbol, span_, name_)
{
}

VoidTypeSymbol::VoidTypeSymbol(const Span& span_, const std::u32string& name_) : BasicTypeSymbol(SymbolType::voidTypeSymbol, span_, name_)
{
}

} } // namespace cmajor::symbols
