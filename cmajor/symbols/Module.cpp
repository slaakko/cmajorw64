// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/Module.hpp>

namespace cmajor { namespace symbols {

void Module::Write(SymbolWriter& writer)
{
    symbolTable.Write(writer);
}

void Module::ReadHeader(SymbolReader& reader)
{

}

void Module::ReadSymbolTable(SymbolReader& reader)
{
    symbolTable.Read(reader);
}


} } // namespace cmajor::symbols
