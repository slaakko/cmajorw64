// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_PARSING_NAMESPACE_INCLUDED
#define CMAJOR_PARSING_NAMESPACE_INCLUDED
#include <cmajor/parsing/ParsingObject.hpp>
#include <cmajor/codedom/Declaration.hpp>

namespace cmajor { namespace parsing {

class Namespace: public ParsingObject
{
public:
    Namespace(const std::u32string& name_, Scope* enclosingScope_);
    virtual void Accept(Visitor& visitor);
    virtual bool IsNamespace() const { return true; }
};

class Scope;

class UsingObject: public ParsingObject
{
public:
    UsingObject(cmajor::codedom::UsingObject* subject_, Scope* enclosingScope_);
    virtual void Accept(Visitor& visitor);
    cmajor::codedom::UsingObject* Subject() const { return subject; }
private:
    cmajor::codedom::UsingObject* subject;
    std::unique_ptr<cmajor::codedom::UsingObject> ownedSubject;
};

} } // namespace cmajor::parsing

#endif // CMAJOR_PARSING_NAMESPACE_INCLUDED
