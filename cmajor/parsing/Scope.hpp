// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_PARSING_SCOPE_INCLUDED
#define CMAJOR_PARSING_SCOPE_INCLUDED
#include <cmajor/parsing/ParsingObject.hpp>
#include <unordered_map>

namespace cmajor { namespace parsing {

class Namespace;

class Scope: public ParsingObject
{
public:
    Scope(const std::u32string& name_, Scope* enclosingScope_);
    virtual std::u32string FullName() const;
    virtual void Accept(Visitor& visitor);
    void SetNs(Namespace* ns_) { ns = ns_; }
    Namespace* Ns() const { return ns; }
    void Add(ParsingObject* object);
    void AddNamespace(Namespace* ns);
    ParsingObject* Get(const std::u32string& objectName) const;
    Namespace* GetNamespace(const std::u32string& fullNamespaceName) const;
private:
    Namespace* ns;
    mutable bool fullNameComputed;
    mutable std::u32string fullName;
    typedef std::unordered_map<std::u32string, ParsingObject*> FullNameMap;
    typedef FullNameMap::const_iterator FullNameMapIt;
    FullNameMap fullNameMap;
    typedef std::unordered_map<std::u32string, ParsingObject*> ShortNameMap;
    typedef ShortNameMap::const_iterator ShortNameMapIt;
    ShortNameMap shortNameMap;
    ParsingObject* GetQualifiedObject(const std::u32string& qualifiedObjectName) const;
    Scope* GetGlobalScope() const;
};

} } // namespace cmajor::parsing

#endif // CMAJOR_PARSING_SCOPE_INCLUDED

