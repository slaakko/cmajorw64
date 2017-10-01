// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/util/Json.hpp>
#include <cmajor/util/TextUtils.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace util {

using namespace cmajor::unicode;

JsonValue::JsonValue(JsonValueType type_) : type(type_)
{
}

JsonValue::~JsonValue()
{
}

JsonString::JsonString() : JsonValue(JsonValueType::string), value()
{
}

JsonString::JsonString(const std::u32string& value_) : JsonValue(JsonValueType::string), value(value_)
{
}

void JsonString::Append(char32_t c)
{
    value.append(1, c);
}

std::u32string JsonString::JsonCharStr(char32_t c) const
{
    switch (c)
    {
        case '"': return U"\\\"";
        case '\\': return U"\\\\";
        case '/': return U"\\/";
        case '\b': return U"\\b";
        case '\f': return U"\\f";
        case '\n': return U"\\n";
        case '\r': return U"\\r";
        case '\t': return U"\\t";
        default:
        {
            if (c >= 32 && c <= 126)
            {
                return std::u32string(1, c);
            }
            return U"\\u" + ToUtf32(ToHexString(static_cast<uint16_t>(c)));
        }
    }
}

std::string JsonString::ToString() const
{
    std::u32string s;
    for (char32_t c : value)
    {
        s.append(JsonCharStr(c));
    }
    return "\"" + ToUtf8(s) + "\"";
}

JsonNumber::JsonNumber() : JsonValue(JsonValueType::number), value(0.0)
{
}

JsonNumber::JsonNumber(double value_) : JsonValue(JsonValueType::number), value(value_)
{
}

std::string JsonNumber::ToString() const
{
    return std::to_string(value);
}

JsonBool::JsonBool() : JsonValue(JsonValueType::boolean), value(false)
{
}

JsonBool::JsonBool(bool value_) : JsonValue(JsonValueType::boolean), value(value_)
{
}

std::string JsonBool::ToString() const
{
    return value ? "true" : "false";
}

JsonObject::JsonObject() : JsonValue(JsonValueType::object), fieldValues(), fieldMap()
{
}

void JsonObject::AddField(const std::u32string& fieldName, std::unique_ptr<JsonValue>&& fieldValue)
{
    fieldMap[fieldName] = fieldValue.get();
    fieldValues.push_back(std::move(fieldValue));
}

JsonValue* JsonObject::GetField(const std::u32string& fieldName)
{
    auto it = fieldMap.find(fieldName);
    if (it != fieldMap.cend())
    {
        return it->second;
    }
    else
    {
        return nullptr;
    }
}

std::string JsonObject::ToString() const
{
    std::string str = "{";
    bool first = true;
    for (const auto& p : fieldMap)
    {
        JsonString s(p.first);
        JsonValue* v(p.second);
        if (first)
        {
            first = false;
        }
        else
        {
            str.append(", ");
        }
        str.append(s.ToString()).append(":").append(v->ToString());
    }
    str.append("}");
    return str;
}

JsonArray::JsonArray() : JsonValue(JsonValueType::array)
{
}

void JsonArray::AddItem(std::unique_ptr<JsonValue>&& item)
{
    items.push_back(std::move(item));
}

JsonValue* JsonArray::operator[](int index) const
{
    return items[index].get();
}

std::string JsonArray::ToString() const
{
    std::string str = "[";
    bool first = true;
    for (const std::unique_ptr<JsonValue>& item : items)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            str.append(", ");
        }
        str.append(item->ToString());
    }
    str.append("]");
    return str;
}

JsonNull::JsonNull() : JsonValue(JsonValueType::null)
{
}

std::string JsonNull::ToString() const
{
    return "null";
}

} } // namespace cmajor::util
