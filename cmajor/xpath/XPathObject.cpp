// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/xpath/XPathObject.hpp>

namespace cmajor { namespace xpath {

XPathObject::XPathObject(XPathObjectType type_) : type(type_)
{
}

XPathObject::~XPathObject()
{
}

XPathNodeSet::XPathNodeSet() : XPathObject(XPathObjectType::nodeSet)
{
}

void XPathNodeSet::Add(cmajor::dom::Node* node)
{
    nodes.InternalAddNode(node);
}

XPathBoolean::XPathBoolean(bool value_) : XPathObject(XPathObjectType::boolean), value(value_)
{
}

XPathNumber::XPathNumber(double value_) : XPathObject(XPathObjectType::number), value(value_)
{
}

XPathString::XPathString(const std::u32string& value_) : XPathObject(XPathObjectType::string), value(value_)
{
}

} } // namespace cmajor::xpath
