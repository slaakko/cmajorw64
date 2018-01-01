// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_SCOPE_INCLUDED
#define CMAJOR_SYMBOLS_SCOPE_INCLUDED
#include <cmajor/ast/Namespace.hpp>
#include <cmajor/parsing/Scanner.hpp>
#include <unordered_map>
#include <unordered_set>

namespace cmajor { namespace symbols {

using cmajor::parsing::Span;
using namespace cmajor::ast;

class Symbol;
class ContainerSymbol;
class NamespaceSymbol;
class FunctionSymbol;

enum class ScopeLookup : uint8_t
{
    none = 0,
    this_ = 1 << 0,
    base = 1 << 1,
    parent = 1 << 2,
    this_and_base = this_ | base,
    this_and_parent = this_ | parent,
    this_and_base_and_parent = this_ | base | parent,
    fileScopes = 1 << 3
};

inline ScopeLookup operator&(ScopeLookup left, ScopeLookup right)
{
    return ScopeLookup(uint8_t(left) & uint8_t(right));
}

inline ScopeLookup operator~(ScopeLookup subject)
{
    return ScopeLookup(~uint8_t(subject));
}

class Scope
{
public:
    virtual ~Scope();
    virtual Symbol* Lookup(const std::u32string& name) const = 0;
    virtual Symbol* Lookup(const std::u32string& name, ScopeLookup lookup) const = 0;
};

class ContainerScope : public Scope
{
public:
    ContainerScope();
    ContainerScope* Base() const { return base; }
    void SetBase(ContainerScope* base_) { base = base_; }
    ContainerScope* Parent() const { return parent; }
    void SetParent(ContainerScope* parent_) { parent = parent_; }
    ContainerSymbol* Container() { return container; }
    void SetContainer(ContainerSymbol* container_) { container = container_; }
    void Install(Symbol* symbol);
    Symbol* Lookup(const std::u32string& name) const override;
    Symbol* Lookup(const std::u32string& name, ScopeLookup lookup) const override;
    Symbol* LookupQualified(const std::vector<std::u32string>& components, ScopeLookup lookup) const;
    const NamespaceSymbol* Ns() const;
    NamespaceSymbol* Ns();
    void Clear();
    NamespaceSymbol* CreateNamespace(const std::u32string& qualifiedNsName, const Span& span);
    void CollectViableFunctions(int arity, const std::u32string& groupName, std::unordered_set<ContainerScope*>& scopesLookedUp, ScopeLookup scopeLookup, std::unordered_set<FunctionSymbol*>& viableFunctions);
private:
    ContainerScope* base;
    ContainerScope* parent;
    ContainerSymbol* container;
    std::unordered_map<std::u32string, Symbol*> symbolMap;
};

class FileScope : public Scope
{
public:
    void InstallAlias(ContainerScope* containerScope, AliasNode* aliasNode);
    void AddContainerScope(ContainerScope* containerScope);
    void InstallNamespaceImport(ContainerScope* containerScope, NamespaceImportNode* namespaceImportNode);
    Symbol* Lookup(const std::u32string& name) const override;
    Symbol* Lookup(const std::u32string& name, ScopeLookup lookup) const override;
    void CollectViableFunctions(int arity, const std::u32string&  groupName, std::unordered_set<ContainerScope*>& scopesLookedUp, std::unordered_set<FunctionSymbol*>& viableFunctions);
private:
    std::vector<ContainerScope*> containerScopes;
    std::unordered_map<std::u32string, Symbol*> aliasSymbolMap;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_SCOPE_INCLUDED
