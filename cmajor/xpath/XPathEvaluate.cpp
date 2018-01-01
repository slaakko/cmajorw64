// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/xpath/XPathEvaluate.hpp>
#include <cmajor/xpath/XPathContext.hpp>
#include <cmajor/xpath/XPath.hpp>
#include <cmajor/dom/Document.hpp>

namespace cmajor { namespace xpath {

XPathGrammar* xpathGrammar = nullptr;

std::unique_ptr<XPathObject> Evaluate(const std::u32string& xpathExpression, cmajor::dom::Node* node)
{
    if (!xpathGrammar)
    {
        xpathGrammar = XPathGrammar::Create();
    }
    std::unique_ptr<XPathExpr> xpathExpr(xpathGrammar->Parse(&xpathExpression[0], &xpathExpression[0] + xpathExpression.length(), 0, ""));
    XPathContext context(node, 1, 1);
    std::unique_ptr<XPathObject> result = xpathExpr->Evaluate(context);
    return result;
}

std::unique_ptr<XPathObject> Evaluate(const std::u32string& xpathExpression, cmajor::dom::Document* document)
{
    return Evaluate(xpathExpression, static_cast<cmajor::dom::Node*>(document));
}

} } // namespace cmajor::xpath
