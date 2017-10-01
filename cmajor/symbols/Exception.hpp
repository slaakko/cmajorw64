// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_EXCEPTION_INCLUDED
#define CMAJOR_SYMBOLS_EXCEPTION_INCLUDED
#include <cmajor/parsing/Scanner.hpp>
#include <cmajor/util/Json.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::util;
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
    std::unique_ptr<JsonValue> ToJson() const;
private:
    std::string what;
    std::string message;
    Span defined;
    std::vector<Span> references;
};

class CastOverloadException : public Exception
{
public:
    CastOverloadException(const std::string& message_, const Span& defined_);
    CastOverloadException(const std::string& message_, const Span& defined_, const Span& referenced_);
    CastOverloadException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_);
};

class CannotBindConstToNonconstOverloadException : public Exception
{
public:
    CannotBindConstToNonconstOverloadException(const std::string& message_, const Span& defined_);
    CannotBindConstToNonconstOverloadException(const std::string& message_, const Span& defined_, const Span& referenced_);
    CannotBindConstToNonconstOverloadException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_);
};

class CannotAssignToConstOverloadException : public Exception
{
public:
    CannotAssignToConstOverloadException(const std::string& message_, const Span& defined_);
    CannotAssignToConstOverloadException(const std::string& message_, const Span& defined_, const Span& referenced_);
    CannotAssignToConstOverloadException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_);
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_EXCEPTION_INCLUDED
