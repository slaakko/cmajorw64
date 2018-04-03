// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/parser/FileRegistry.hpp>
#include <cmajor/parsing/Exception.hpp>
#include <cmajor/dom/CharacterData.hpp>
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

std::unique_ptr<cmajor::dom::Element> SpanToDomElement(const Span& span)
{
    if (!span.Valid()) return std::unique_ptr<cmajor::dom::Element>();
    const std::string& fileName = FileRegistry::Instance().GetFilePath(span.FileIndex());
    if (fileName.empty()) return std::unique_ptr<cmajor::dom::Element>();
    std::unique_ptr<cmajor::dom::Element> spanElement(new cmajor::dom::Element(U"span"));
    std::unique_ptr<cmajor::dom::Element> fileElement(new cmajor::dom::Element(U"file"));
    std::unique_ptr<cmajor::dom::Text> fileText(new cmajor::dom::Text(ToUtf32(fileName)));
    fileElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(fileText.release()));
    std::unique_ptr<cmajor::dom::Element> lineElement(new cmajor::dom::Element(U"line"));
    std::unique_ptr<cmajor::dom::Text> lineText(new cmajor::dom::Text(ToUtf32(std::to_string(span.LineNumber()))));
    lineElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(lineText.release()));
    MappedInputFile file(fileName);
    std::string s(file.Begin(), file.End());
    std::u32string t(ToUtf32(s));
    std::u32string text = cmajor::parsing::GetErrorLines(&t[0], &t[0] + t.length(), span);
    int32_t startCol = 0;
    int32_t endCol = 0;
    GetColumns(&t[0], &t[0] + t.length(), span, startCol, endCol);
    spanElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(fileElement.release()));
    spanElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(lineElement.release()));
    std::unique_ptr<cmajor::dom::Element> startColElement(new cmajor::dom::Element(U"startCol"));
    std::unique_ptr<cmajor::dom::Text> startColText(new cmajor::dom::Text(ToUtf32(std::to_string(startCol))));
    startColElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(startColText.release()));
    spanElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(startColElement.release()));
    std::unique_ptr<cmajor::dom::Element> endColElement(new cmajor::dom::Element(U"endCol"));
    std::unique_ptr<cmajor::dom::Text> endColText(new cmajor::dom::Text(ToUtf32(std::to_string(endCol))));
    endColElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(endColText.release()));
    spanElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(endColElement.release()));
    std::unique_ptr<cmajor::dom::Element> textElement(new cmajor::dom::Element(U"text"));
    std::unique_ptr<cmajor::dom::Text> textText(new cmajor::dom::Text(text));
    textElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(textText.release()));
    spanElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(textElement.release()));
    return spanElement;
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

void Exception::AddToDiagnosticsElement(cmajor::dom::Element* diagnosticsElement) const
{
    std::unique_ptr<cmajor::dom::Element> diagnosticElement(new cmajor::dom::Element(U"diagnostic"));
    std::unique_ptr<cmajor::dom::Element> categoryElement(new cmajor::dom::Element(U"category"));
    std::unique_ptr<cmajor::dom::Text> categoryText(new cmajor::dom::Text(U"error"));
    categoryElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(categoryText.release()));
    std::unique_ptr<cmajor::dom::Element> subcategoryElement(new cmajor::dom::Element(U"subcategory"));
    std::unique_ptr<cmajor::dom::Text> subcategoryText(new cmajor::dom::Text(U"error"));
    subcategoryElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(subcategoryText.release()));
    std::unique_ptr<cmajor::dom::Element> messageElement(new cmajor::dom::Element(U"message"));
    std::unique_ptr<cmajor::dom::Text> messageText(new cmajor::dom::Text(ToUtf32(message)));
    messageElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(messageText.release()));
    diagnosticElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(categoryElement.release()));
    diagnosticElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(subcategoryElement.release()));
    diagnosticElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(messageElement.release()));
    std::unique_ptr<cmajor::dom::Element> spanElement = SpanToDomElement(defined);
    if (spanElement)
    {
        diagnosticElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(spanElement.release()));
    }
    diagnosticsElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(diagnosticElement.release()));
    for (const Span& span : references)
    {
        if (!span.Valid()) continue;
        std::unique_ptr<cmajor::dom::Element> diagnosticElement(new cmajor::dom::Element(U"diagnostic"));
        std::unique_ptr<cmajor::dom::Element> categoryElement(new cmajor::dom::Element(U"category"));
        std::unique_ptr<cmajor::dom::Text> categoryText(new cmajor::dom::Text(U"info"));
        categoryElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(categoryText.release()));
        std::unique_ptr<cmajor::dom::Element> messageElement(new cmajor::dom::Element(U"message"));
        std::unique_ptr<cmajor::dom::Text> messageText(new cmajor::dom::Text(ToUtf32("see reference to")));
        messageElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(messageText.release()));
        diagnosticElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(categoryElement.release()));
        diagnosticElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(messageElement.release()));
        std::unique_ptr<cmajor::dom::Element> spanElement = SpanToDomElement(span);
        if (spanElement)
        {
            diagnosticElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(spanElement.release()));
            diagnosticsElement->AppendChild(std::unique_ptr<cmajor::dom::Node>(diagnosticElement.release()));
        }
    }

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
