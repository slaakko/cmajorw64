// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/Exception.hpp>
#include <cmajor/parser/FileRegistry.hpp>
#include <cmajor/parsing/Exception.hpp>
#include <cmajor/util/MappedInputFile.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::parser;
using namespace cmajor::util;
using namespace cmajor::unicode;

std::string Expand(const std::string& errorMessage, const Span& span)
{
    std::vector<Span> references;
    return Expand(errorMessage, span, references);
}

std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan)
{
    std::vector<Span> references(1, referenceSpan);
    return Expand(errorMessage, primarySpan, references, "Error");
}

std::string Expand(const std::string& errorMessage, const Span& primarySpan, const Span& referenceSpan, const std::string& title)
{
    std::vector<Span> references(1, referenceSpan);
    return Expand(errorMessage, primarySpan, references, title);
}

std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references)
{
    return Expand(errorMessage, span, references, "Error");
}

std::string Expand(const std::string& errorMessage, const Span& span, const std::vector<Span>& references, const std::string& title)
{
    std::string expandedMessage = title + ": " + errorMessage;
    if (span.Valid())
    {
        const std::string& fileName = FileRegistry::Instance().GetFilePath(span.FileIndex());
        if (!fileName.empty())
        {
            expandedMessage.append(" (file '" + fileName + "', line " + std::to_string(span.LineNumber()) + ")");
            MappedInputFile file(fileName);
            std::string s(file.Begin(), file.End());
            std::u32string t(ToUtf32(s));
            expandedMessage.append(":\n").append(ToUtf8(cmajor::parsing::GetErrorLines(&t[0], &t[0] + t.length(), span)));
        }
        for (const Span& referenceSpan : references)
        {
            const std::string& fileName = FileRegistry::Instance().GetFilePath(referenceSpan.FileIndex());
            if (!fileName.empty())
            {
                expandedMessage.append("\nsee reference to file '" + fileName + "', line " + std::to_string(referenceSpan.LineNumber()));
                MappedInputFile file(fileName);
                std::string s(file.Begin(), file.End());
                std::u32string t(ToUtf32(s));
                expandedMessage.append(":\n").append(ToUtf8(cmajor::parsing::GetErrorLines(&t[0], &t[0] + t.length(), referenceSpan)));
            }
        }
    }
    return expandedMessage;
}

Exception::Exception(const std::string& message_, const Span& defined_) : what(Expand(message_, defined_)), message(message_), defined(defined_)
{
}

Exception::Exception(const std::string& message_, const Span& defined_, const Span& referenced_) : what(Expand(message_, defined_, referenced_)), message(message_), defined(defined_)
{
    references.push_back(referenced_);
}

Exception::Exception(const std::string& message_, const Span& defined_, const std::vector<Span>& references_) :
    what(Expand(message_, defined_, references_)), message(message_), defined(defined_), references(references_)
{
}

Exception::~Exception()
{
}

CastOverloadException::CastOverloadException(const std::string& message_, const Span& defined_) : Exception(message_, defined_)
{
}

CastOverloadException::CastOverloadException(const std::string& message_, const Span& defined_, const Span& referenced_) : Exception(message_, defined_, referenced_)
{
}

CastOverloadException::CastOverloadException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_) : Exception(message_, defined_, references_)
{
}

CannotBindConstToNonconstOverloadException::CannotBindConstToNonconstOverloadException(const std::string& message_, const Span& defined_) : Exception(message_, defined_)
{
}

CannotBindConstToNonconstOverloadException::CannotBindConstToNonconstOverloadException(const std::string& message_, const Span& defined_, const Span& referenced_) :
    Exception(message_, defined_, referenced_)
{
}

CannotBindConstToNonconstOverloadException::CannotBindConstToNonconstOverloadException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_) :
    Exception(message_, defined_, references_)
{
}

CannotAssignToConstOverloadException::CannotAssignToConstOverloadException(const std::string& message_, const Span& defined_) : Exception(message_, defined_)
{
}

CannotAssignToConstOverloadException::CannotAssignToConstOverloadException(const std::string& message_, const Span& defined_, const Span& referenced_) :
    Exception(message_, defined_, referenced_)
{
}

CannotAssignToConstOverloadException::CannotAssignToConstOverloadException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_) :
    Exception(message_, defined_, references_)
{
}

} } // namespace cmajor::symbols
