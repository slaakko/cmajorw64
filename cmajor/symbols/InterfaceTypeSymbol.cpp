// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/InterfaceTypeSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>

namespace cmajor { namespace symbols {

InterfaceTypeSymbol::InterfaceTypeSymbol(const Span& span_, const std::u32string& name_) : TypeSymbol(SymbolType::interfaceTypeSymbol, span_, name_)
{
}

void InterfaceTypeSymbol::SetSpecifiers(Specifiers specifiers)
{
    Specifiers accessSpecifiers = specifiers & Specifiers::access_;
    SetAccess(accessSpecifiers);
    if ((specifiers & Specifiers::static_) != Specifiers::none)
    {
        throw Exception("interface cannot be static", GetSpan());
    }
    if ((specifiers & Specifiers::virtual_) != Specifiers::none)
    {
        throw Exception("interface cannot be virtual", GetSpan());
    }
    if ((specifiers & Specifiers::override_) != Specifiers::none)
    {
        throw Exception("interface cannot be override", GetSpan());
    }
    if ((specifiers & Specifiers::abstract_) != Specifiers::none)
    {
        throw Exception("interface cannot be abstract", GetSpan());
    }
    if ((specifiers & Specifiers::inline_) != Specifiers::none)
    {
        throw Exception("interface cannot be inline", GetSpan());
    }
    if ((specifiers & Specifiers::explicit_) != Specifiers::none)
    {
        throw Exception("interface cannot be explicit", GetSpan());
    }
    if ((specifiers & Specifiers::external_) != Specifiers::none)
    {
        throw Exception("interface cannot be external", GetSpan());
    }
    if ((specifiers & Specifiers::suppress_) != Specifiers::none)
    {
        throw Exception("interface cannot be suppressed", GetSpan());
    }
    if ((specifiers & Specifiers::default_) != Specifiers::none)
    {
        throw Exception("interface cannot be default", GetSpan());
    }
    if ((specifiers & Specifiers::constexpr_) != Specifiers::none)
    {
        throw Exception("interface cannot be constexpr", GetSpan());
    }
    if ((specifiers & Specifiers::cdecl_) != Specifiers::none)
    {
        throw Exception("interface cannot be cdecl", GetSpan());
    }
    if ((specifiers & Specifiers::nothrow_) != Specifiers::none)
    {
        throw Exception("interface cannot be nothrow", GetSpan());
    }
    if ((specifiers & Specifiers::throw_) != Specifiers::none)
    {
        throw Exception("interface cannot be throw", GetSpan());
    }
    if ((specifiers & Specifiers::new_) != Specifiers::none)
    {
        throw Exception("interface cannot be new", GetSpan());
    }
    if ((specifiers & Specifiers::const_) != Specifiers::none)
    {
        throw Exception("interface cannot be const", GetSpan());
    }
    if ((specifiers & Specifiers::unit_test_) != Specifiers::none)
    {
        throw Exception("interface cannot be unit_test", GetSpan());
    }
}

} } // namespace cmajor::symbols
