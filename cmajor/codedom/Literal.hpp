// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_CODEDOM_LITERAL_INCLUDED
#define CMAJOR_CODEDOM_LITERAL_INCLUDED
#include <cmajor/codedom/Object.hpp>

namespace cmajor { namespace codedom {

class Literal: public CppObject
{
public:
    Literal(const std::u32string& name_);
    virtual int Rank() const { return 24; }
    virtual void Accept(Visitor& visitor);
};

} } // namespace cmajor::codedom

#endif // CMAJOR_CODEDOM_LITERAL_INCLUDED

