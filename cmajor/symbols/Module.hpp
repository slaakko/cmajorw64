// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_MODULE_INCLUDED
#define CMAJOR_SYMBOLS_MODULE_INCLUDED
#include <cmajor/symbols/SymbolTable.hpp>

namespace cmajor { namespace symbols {

class Module
{
public:
    SymbolTable& GetSymbolTable() { return symbolTable; }
    void Write(SymbolWriter& writer);
    void ReadHeader(SymbolReader& reader);
    void ReadSymbolTable(SymbolReader& reader);
private:
    SymbolTable symbolTable;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_MODULE_INCLUDED
