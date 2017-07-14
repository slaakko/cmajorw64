// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_OVERLOAD_RESOLUTION_INCLUDED
#define CMAJOR_BINDER_OVERLOAD_RESOLUTION_INCLUDED
#include <cmajor/binder/BoundExpression.hpp>
#include <cmajor/symbols/Scope.hpp>
#include <cmajor/symbols/Exception.hpp>

namespace cmajor { namespace binder {

class BoundExpression;
class BoundFunctionCall;
class BoundCompileUnit;
using namespace cmajor::symbols;

enum class OverloadResolutionFlags : uint8_t
{
    none = 0,
    dontThrow = 1 << 0
};

struct FunctionScopeLookup
{
    FunctionScopeLookup(ScopeLookup scopeLookup_) : scopeLookup(scopeLookup_), scope(nullptr) {}
    FunctionScopeLookup(ScopeLookup scopeLookup_, ContainerScope* scope_) : scopeLookup(scopeLookup_), scope(scope_) {}
    ScopeLookup scopeLookup;
    ContainerScope* scope;
};

std::unique_ptr<BoundFunctionCall> ResolveOverload(const std::u32string& groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups, std::vector<std::unique_ptr<BoundExpression>>& arguments,
    BoundCompileUnit& boundCompileUnit, const Span& span);

std::unique_ptr<BoundFunctionCall> ResolveOverload(const std::u32string& groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups, std::vector<std::unique_ptr<BoundExpression>>& arguments,
    BoundCompileUnit& boundCompileUnit, const Span& span, OverloadResolutionFlags flags, std::unique_ptr<Exception>& exception);

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_OVERLOAD_RESOLUTION_INCLUDED
