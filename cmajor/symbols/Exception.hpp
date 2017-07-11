// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_EXCEPTION_INCLUDED
#define CMAJOR_SYMBOLS_EXCEPTION_INCLUDED
#include <cmajor/parsing/Scanner.hpp>

namespace cmajor { namespace symbols {

using cmajor::parsing::Span;

std::string Expand(const std::string& errorMessage, const Span& span);
std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan);
std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan, const std::string& title);
std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references);
std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references, const std::string& title);

class Exception
{
public:
    Exception(const std::string& message_, const Span& defined_);
    Exception(const std::string& message_, const Span& defined_, const Span& referenced_);
    Exception(const std::string& message_, const Span& defined_, const std::vector<Span>& references_);
    virtual ~Exception();
    const std::string& What() const { return what; }
    const std::string& Message() const { return message; }
    const Span& Defined() const { return defined; }
    const std::vector<Span>& References() const { return references; }
private:
    std::string what;
    std::string message;
    Span defined;
    std::vector<Span> references;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_EXCEPTION_INCLUDED
