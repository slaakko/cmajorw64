// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/parsing/Parser.hpp>

namespace cmajor { namespace parsing {

Object::~Object()
{
}

Parser::Parser(const std::u32string& name_, const std::u32string& info_): ParsingObject(name_), info(info_)
{
}

} } // namespace cmajor::parsing
