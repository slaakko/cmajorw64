// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/OverloadResolution.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>

namespace cmajor { namespace binder {

std::unique_ptr<BoundFunctionCall> ResolveOverload(const std::u32string& groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups, std::vector<std::unique_ptr<BoundExpression>>& arguments,
    BoundCompileUnit& boundCompileUnit, const Span& span)
{
    std::unique_ptr<Exception> exception;
    return ResolveOverload(groupName, functionScopeLookups, arguments, boundCompileUnit, span, OverloadResolutionFlags::none, exception);
}

std::unique_ptr<BoundFunctionCall> ResolveOverload(const std::u32string& groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups, std::vector<std::unique_ptr<BoundExpression>>& arguments,
    BoundCompileUnit& boundCompileUnit, const Span& span, OverloadResolutionFlags flags, std::unique_ptr<Exception>& exception)
{
    int arity = int(arguments.size());
    std::unordered_set<FunctionSymbol*> viableFunctions;
    std::unordered_set<ContainerScope*> scopesLookedUp;
    bool fileScopesLookedUp = false;
    for (const FunctionScopeLookup& functionScopeLookup : functionScopeLookups)
    {
        if (functionScopeLookup.scopeLookup == ScopeLookup::fileScopes && !fileScopesLookedUp)
        {
            fileScopesLookedUp = true;
            for (const std::unique_ptr<FileScope>& fileScope : boundCompileUnit.FileScopes())
            {
                fileScope->CollectViableFunctions(arity, groupName, scopesLookedUp, viableFunctions);
            }
        }
        else
        {
            functionScopeLookup.scope->CollectViableFunctions(arity, groupName, scopesLookedUp, functionScopeLookup.scopeLookup, viableFunctions);
        }
    }
    return std::unique_ptr<BoundFunctionCall>();
}

} } // namespace cmajor::binder
