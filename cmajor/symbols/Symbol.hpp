// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_SYMBOL_INCLUDED
#define CMAJOR_SYMBOLS_SYMBOL_INCLUDED
#include <stdint.h>

namespace cmajor { namespace symbols {

enum class SymbolType : uint8_t
{

};

class Symbol
{
public:
    Symbol(SymbolType symbolType_);
    virtual ~Symbol();
    SymbolType GetSymbolType() const { return symbolType; }
private:
    SymbolType symbolType;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_SYMBOL_INCLUDED
