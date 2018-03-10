// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/parser/FileRegistry.hpp>
#include <cmajor/parsing/Exception.hpp>
#include <cmajor/util/MappedInputFile.hpp>
#include <cmajor/util/Unicode.hpp>
#include <algorithm>

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
    std::vector<Span> referenceSpans = references;
    referenceSpans.erase(std::unique(referenceSpans.begin(), referenceSpans.end()), referenceSpans.end());
    std::string expandedMessage = title + ": " + errorMessage;
    if (span.Valid())
    {
        std::string fileName = FileRegistry::Instance().GetFilePath(span.FileIndex());
        if (!fileName.empty())
        {
            expandedMessage.append(" (file '" + fileName + "', line " + std::to_string(span.LineNumber()) + ")");
            MappedInputFile file(fileName);
            std::string s(file.Begin(), file.End());
            std::u32string t(ToUtf32(s));
            expandedMessage.append(":\n").append(ToUtf8(cmajor::parsing::GetErrorLines(&t[0], &t[0] + t.length(), span)));
        }
    }
    for (const Span& referenceSpan : referenceSpans)
    {
        if (!referenceSpan.Valid()) continue;
        if (referenceSpan == span) continue;
        std::string fileName = FileRegistry::Instance().GetFilePath(referenceSpan.FileIndex());
        if (!fileName.empty())
        {
            expandedMessage.append("\nsee reference to file '" + fileName + "', line " + std::to_string(referenceSpan.LineNumber()));
            MappedInputFile file(fileName);
            std::string s(file.Begin(), file.End());
            std::u32string t(ToUtf32(s));
            expandedMessage.append(":\n").append(ToUtf8(cmajor::parsing::GetErrorLines(&t[0], &t[0] + t.length(), referenceSpan)));
        }
    }
    return expandedMessage;
}

std::unique_ptr<JsonObject> SpanToJson(const Span& span)
{
    if (!span.Valid()) return std::unique_ptr<JsonObject>();
    const std::string& fileName = FileRegistry::Instance().GetFilePath(span.FileIndex());
    if (fileName.empty()) return std::unique_ptr<JsonObject>();
    std::unique_ptr<JsonObject> json(new JsonObject());
    json->AddField(U"file", std::unique_ptr<JsonValue>(new JsonString(ToUtf32(fileName))));
    json->AddField(U"line", std::unique_ptr<JsonValue>(new JsonString(ToUtf32(std::to_string(span.LineNumber())))));
    MappedInputFile file(fileName);
    std::string s(file.Begin(), file.End());
    std::u32string t(ToUtf32(s));
    std::u32string text = cmajor::parsing::GetErrorLines(&t[0], &t[0] + t.length(), span);
    int32_t startCol = 0;
    int32_t endCol = 0;
    GetColumns(&t[0], &t[0] + t.length(), span, startCol, endCol);
    json->AddField(U"startCol", std::unique_ptr<JsonValue>(new JsonString(ToUtf32(std::to_string(startCol)))));
    json->AddField(U"endCol", std::unique_ptr<JsonValue>(new JsonString(ToUtf32(std::to_string(endCol)))));
    json->AddField(U"text", std::unique_ptr<JsonValue>(new JsonString(text)));
    return json;
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

std::unique_ptr<JsonValue> Exception::ToJson() const
{
    std::unique_ptr<JsonObject> json(new JsonObject());
    json->AddField(U"tool", std::unique_ptr<JsonValue>(new JsonString(GetCurrentToolName())));
    json->AddField(U"kind", std::unique_ptr<JsonValue>(new JsonString(U"error")));
    json->AddField(U"project", std::unique_ptr<JsonValue>(new JsonString(GetCurrentProjectName())));
    json->AddField(U"message", std::unique_ptr<JsonValue>(new JsonString(ToUtf32(message))));
    std::unique_ptr<JsonArray> refs(new JsonArray());
    std::unique_ptr<JsonObject> ref = SpanToJson(defined);
    if (ref)
    {
        refs->AddItem(std::move(ref));
    }
    std::vector<Span> referenceSpans = references;
    referenceSpans.erase(std::unique(referenceSpans.begin(), referenceSpans.end()), referenceSpans.end());
    for (const Span& referenceSpan : referenceSpans)
    {
        if (!referenceSpan.Valid()) continue;
        if (referenceSpan == defined) continue;
        std::unique_ptr<JsonObject> ref = SpanToJson(referenceSpan);
        if (ref)
        {
            refs->AddItem(std::move(ref));
        }
    }
    json->AddField(U"references", std::move(refs));
    return std::unique_ptr<JsonValue>(json.release());
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

NoViableFunctionException::NoViableFunctionException(const std::string& message_, const Span& defined_) : Exception(message_, defined_)
{
}

NoViableFunctionException::NoViableFunctionException(const std::string& message_, const Span& defined_, const Span& referenced_) :
    Exception(message_, defined_, referenced_)
{
}

NoViableFunctionException::NoViableFunctionException(const std::string& message_, const Span& defined_, const std::vector<Span>& references_) :
    Exception(message_, defined_, references_)
{
}

} } // namespace cmajor::symbols
