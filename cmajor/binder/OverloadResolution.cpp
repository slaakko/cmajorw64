// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/OverloadResolution.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::unicode;

bool BetterFunctionMatch::operator()(const FunctionMatch& left, const FunctionMatch& right) const
{
    int leftBetterArgumentMatches = 0;
    int rightBetterArgumentMatches = 0;
    int n = std::max(int(left.argumentMatches.size()), int(right.argumentMatches.size()));
    for (int i = 0; i < n; ++i)
    {
        ArgumentMatch leftMatch;
        if (i < int(left.argumentMatches.size()))
        {
            leftMatch = left.argumentMatches[i];
        }
        ArgumentMatch rightMatch;
        if (i < int(right.argumentMatches.size()))
        {
            rightMatch = right.argumentMatches[i];
        }
        if (BetterArgumentMatch(leftMatch, rightMatch))
        {
            ++leftBetterArgumentMatches;
        }
        else if (BetterArgumentMatch(rightMatch, leftMatch))
        {
            ++rightBetterArgumentMatches;
        }
    }
    if (leftBetterArgumentMatches > rightBetterArgumentMatches)
    {
        return true;
    }
    if (rightBetterArgumentMatches > leftBetterArgumentMatches)
    {
        return false;
    }
    if (left.numConversions < right.numConversions)
    {
        return true;
    }
    return false;
}

bool FindAssignmentConversion(int argumentIndex, TypeSymbol* sourceType, TypeSymbol* targetType, FunctionMatch& functionMatch, const Span& span)
{
    if (argumentIndex == 0)
    {
        if (targetType->IsConstType())
        {
            functionMatch.cannotAssignToConstObject = true;
            return false;
        }
        else if (sourceType->IsReferenceType())
        {
            functionMatch.argumentMatches.push_back(ArgumentMatch(nullptr, BoundExpressionFlags::load, 1));
            ++functionMatch.numConversions;
            return true;
        }
    }
    else if (argumentIndex == 1)
    {
        if (TypesEqual(sourceType->PlainType(span), targetType->PlainType(span)))
        {
            if (sourceType->IsReferenceType())
            {
                functionMatch.argumentMatches.push_back(ArgumentMatch(nullptr, BoundExpressionFlags::deref, 1));
                ++functionMatch.numConversions;
                return true;
            }
            else
            {
                functionMatch.argumentMatches.push_back(ArgumentMatch());
                return true;
            }
        }
    }
    return false;
}

bool FindQualificationConversion(TypeSymbol* sourceType, TypeSymbol* targetType, ConversionType conversionType, FunctionSymbol* conversionFun, FunctionMatch& functionMatch, const Span& span)
{
    int distance = 0;
    if (conversionFun)
    {
        distance = conversionFun->ConversionDistance();
    }
    if (sourceType->IsConstType())
    {
        if (targetType->IsConstType())
        {
            ++distance;
        }
        else if (conversionType == ConversionType::implicit_)
        {
            functionMatch.cannotBindConstArgToNonConstParam = true;
            functionMatch.sourceType = sourceType;
            functionMatch.targetType = targetType;
            return false;
        }
        else
        {
            distance = 255;
        }
    }
    else
    {
        if (targetType->IsConstType())
        {
            distance += 2;
        }
        else
        {
            distance += 3;
        }
    }
    if (sourceType->IsReferenceType() && !targetType->IsReferenceType())
    {
        functionMatch.argumentMatches.push_back(ArgumentMatch(conversionFun, BoundExpressionFlags::deref, distance));
        ++functionMatch.numConversions;
        return true;
    }
    else if (!sourceType->IsReferenceType() && targetType->IsReferenceType())
    {
        if (targetType->IsConstType())
        {
            functionMatch.argumentMatches.push_back(ArgumentMatch(conversionFun, BoundExpressionFlags::addr, distance));
            ++functionMatch.numConversions;
            return true;
        }
        else
        {
            functionMatch.cannotBindConstArgToNonConstParam = true;
            functionMatch.sourceType = sourceType;
            functionMatch.targetType = targetType;
        }
    }
    else if (sourceType->IsConstType() && !targetType->IsConstType())
    {
        functionMatch.argumentMatches.push_back(ArgumentMatch(conversionFun, BoundExpressionFlags::none, distance));
        ++functionMatch.numConversions;
        return true;
    }
    else if (!sourceType->IsConstType() && targetType->IsConstType())
    {
        functionMatch.argumentMatches.push_back(ArgumentMatch(conversionFun, BoundExpressionFlags::none, distance));
        ++functionMatch.numConversions;
        return true;
    }
    else if (conversionFun)
    {
        functionMatch.argumentMatches.push_back(ArgumentMatch(conversionFun, BoundExpressionFlags::none, distance));
        return true;
    }
    return false;
}

bool FindConversions(BoundCompileUnit& boundCompileUnit, FunctionSymbol* function, std::vector<std::unique_ptr<BoundExpression>>& arguments, FunctionMatch& functionMatch, 
    ConversionType conversionType, const Span& span)
{
    int arity = arguments.size();
    Assert(arity == function->Arity(), "wrong arity");
    for (int i = 0; i < arity; ++i)
    {
        BoundExpression* argument = arguments[i].get();
        TypeSymbol* sourceType = argument->GetType();
        ParameterSymbol* parameter = function->Parameters()[i];
        TypeSymbol* targetType = parameter->GetType();
        if (arity == 2 && function->GroupName() == U"operator=")
        {
            if (FindAssignmentConversion(i, sourceType, targetType, functionMatch, span))
            {
                continue;
            }
            else
            {
                return false;
            }
        }
        else if (TypesEqual(sourceType, targetType))    // exact match
        {
            functionMatch.argumentMatches.push_back(ArgumentMatch());
        }
        else
        {
            if (arity == 2 && (function->GroupName() == U"@constructor" || function->GroupName() == U"operator="))
            {
                if (i == 0)
                {
                    return false;
                }
            }
            bool qualificationConversionMatch = false;
            if (TypesEqual(sourceType->PlainType(span), targetType->PlainType(span)))
            {
                qualificationConversionMatch = FindQualificationConversion(sourceType, targetType, conversionType, nullptr, functionMatch, span);
            }
            if (!qualificationConversionMatch)
            {
                FunctionSymbol* conversionFun = boundCompileUnit.GetConversion(sourceType, targetType, span);
                if (conversionFun)
                {
                    if (conversionFun->GetConversionType() == conversionType || conversionFun->GetConversionType() == ConversionType::implicit_)
                    {
                        ++functionMatch.numConversions;
                        if (FindQualificationConversion(sourceType, targetType, conversionType, conversionFun, functionMatch, span))
                        {
                            continue;
                        }
                        else
                        {
                            return false;
                        }
                    }
                    else
                    {
                        if (arity == 2 && i == 1 && conversionType == ConversionType::implicit_ && conversionFun->GetConversionType() == ConversionType::explicit_)
                        {
                            functionMatch.castRequired = true;
                            functionMatch.sourceType = sourceType;
                            functionMatch.targetType = targetType;
                        }
                        return false;
                    }
                }
                else
                {
                    return false;
                }
            }
        }
    }
    return true;
}

std::string MakeOverloadName(const std::u32string& groupName, const std::vector<std::unique_ptr<BoundExpression>>& arguments)
{
    std::string overloadName = ToUtf8(groupName);
    overloadName.append(1, '(');
    bool first = true;
    for (const std::unique_ptr<BoundExpression>& argument : arguments)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            overloadName.append(", ");
        }
        overloadName.append(ToUtf8(argument->GetType()->FullName()));
    }
    overloadName.append(1, ')');
    return overloadName;
}

std::unique_ptr<BoundFunctionCall> FailWithNoViableFunction(const std::u32string& groupName, const std::vector<std::unique_ptr<BoundExpression>>& arguments, const Span& span, OverloadResolutionFlags flags, 
    std::unique_ptr<Exception>& exception)
{
    std::string overloadName = MakeOverloadName(groupName, arguments);
    int arity = arguments.size();
    if (groupName == U"@constructor" && arity == 1 && arguments[0]->GetType()->IsReferenceType())
    {
        if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
        {
            exception.reset(new Exception("overload resolution failed: '" + overloadName + "' not found. Note: reference must be initialized.", span));
            return std::unique_ptr<BoundFunctionCall>();
        }
        else
        {
            throw Exception("overload resolution failed: '" + overloadName + "' not found. Note: reference must be initialized.", span);
        }
    }
    else
    {
        if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
        {
            exception.reset(new Exception("overload resolution failed: '" + overloadName + "' not found. " +
                "No viable functions taking " + std::to_string(arity) + " arguments found in function group '" + ToUtf8(groupName) + "'", span));
            return std::unique_ptr<BoundFunctionCall>();
        }
        else
        {
            throw Exception("overload resolution failed: '" + overloadName + "' not found. " +
                "No viable functions taking " + std::to_string(arity) + " arguments found in function group '" + ToUtf8(groupName) + "'", span);
        }
    }
}

std::unique_ptr<BoundFunctionCall> FailWithOverloadNotFound(const std::unordered_set<FunctionSymbol*>& viableFunctions, const std::u32string& groupName, const std::vector<std::unique_ptr<BoundExpression>>& arguments,
    const std::vector<FunctionMatch>& failedFunctionMatches, const Span& span, OverloadResolutionFlags flags, std::unique_ptr<Exception>& exception)
{
    std::string overloadName = MakeOverloadName(groupName, arguments);
    bool castRequired = false;
    bool cannotBindConstArgToNonConstParam = false;
    bool cannotAssignToConstObject = false;
    TypeSymbol* sourceType = nullptr;
    TypeSymbol* targetType = nullptr;
    std::vector<Span> references;
    if (!failedFunctionMatches.empty())
    {
        int n = int(failedFunctionMatches.size());
        for (int i = 0; i < n; ++i)
        {
            if (failedFunctionMatches[i].castRequired)
            {
                castRequired = true;
                sourceType = failedFunctionMatches[i].sourceType;
                targetType = failedFunctionMatches[i].targetType;
                references.push_back(failedFunctionMatches[i].fun->GetSpan());
                break;
            }
        }
        if (!castRequired)
        {
            for (int i = 0; i < n; ++i)
            {
                if (failedFunctionMatches[i].cannotBindConstArgToNonConstParam)
                {
                    cannotBindConstArgToNonConstParam = true;
                    sourceType = failedFunctionMatches[i].sourceType;
                    targetType = failedFunctionMatches[i].targetType;
                    references.push_back(failedFunctionMatches[i].fun->GetSpan());
                    break;
                }
            }
        }
        if (!cannotBindConstArgToNonConstParam)
        {
            for (int i = 0; i < n; ++i)
            {
                if (failedFunctionMatches[i].cannotAssignToConstObject)
                {
                    cannotAssignToConstObject = true;
                    references.push_back(failedFunctionMatches[i].fun->GetSpan());
                    break;
                }
            }
        }
    }
    if (groupName == U"@constructor" && arguments.size() == 1 && arguments[0]->GetType()->IsReferenceType())
    {
        if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
        {
            exception.reset(new Exception("overload resolution failed: '" + overloadName + "' not found. Note: reference must be initialized.", span, references));
            return std::unique_ptr<BoundFunctionCall>();
        }
        else
        {
            throw Exception("overload resolution failed: '" + overloadName + "' not found. Note: reference must be initialized.", span, references);
        }
    }
    else if (castRequired)
    {
        Assert(sourceType, "source type not set");
        Assert(targetType, "target type not set");
        if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
        {
            exception.reset(new CastOverloadException("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                std::to_string(viableFunctions.size()) + " viable functions examined. Note: cannot convert implicitly from '" +
                ToUtf8(sourceType->FullName()) + "' to '" + ToUtf8(targetType->FullName()) + "' but explicit conversion (cast) exists.", span, references));
            return std::unique_ptr<BoundFunctionCall>();
        }
        else
        {
            throw CastOverloadException("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                std::to_string(viableFunctions.size()) + " viable functions examined. Note: cannot convert implicitly from '" +
                ToUtf8(sourceType->FullName()) + "' to '" + ToUtf8(targetType->FullName()) + "' but explicit conversion (cast) exists.", span, references);
        }
    }
    else if (cannotBindConstArgToNonConstParam)
    {
        Assert(sourceType, "source type not set");
        Assert(targetType, "target type not set");
        if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
        {
            exception.reset(new CannotBindConstToNonconstOverloadException("overload resolution failed: '" + overloadName + 
                "' not found, or there are no acceptable conversions for all argument types. " +
                std::to_string(viableFunctions.size()) + " viable functions examined. Note: cannot bind constant argument type '" + ToUtf8(sourceType->FullName()) + 
                "' to nonconstant parameter type '" + ToUtf8(targetType->FullName()) + "'", span, references));
            return std::unique_ptr<BoundFunctionCall>();
        }
        else
        {
            throw CannotBindConstToNonconstOverloadException("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                std::to_string(viableFunctions.size()) + " viable functions examined. Note: cannot bind constant argument type '" + ToUtf8(sourceType->FullName()) +
                "' to nonconstant parameter type '" + ToUtf8(targetType->FullName()) + "'", span, references);
        }
    }
    else if (cannotAssignToConstObject)
    {
        if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
        {
            exception.reset(new CannotAssignToConstOverloadException("overload resolution failed: '" + overloadName +
                "' not found, or there are no acceptable conversions for all argument types. " +
                std::to_string(viableFunctions.size()) + " viable functions examined. Note: cannot assign to const object.", span, references));
            return std::unique_ptr<BoundFunctionCall>();
        }
        else
        {
            throw CannotAssignToConstOverloadException("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                std::to_string(viableFunctions.size()) + " viable functions examined. Note: cannot assign to const object. ", span, references);
        }
    }
    else
    {
        if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
        {
            exception.reset(new Exception("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                std::to_string(viableFunctions.size()) + " viable functions examined.", span, references));
            return std::unique_ptr<BoundFunctionCall>();
        }
        else
        {
            throw Exception("overload resolution failed: '" + overloadName + "' not found, or there are no acceptable conversions for all argument types. " +
                std::to_string(viableFunctions.size()) + " viable functions examined.", span, references);
        }
    }
}

std::unique_ptr<BoundFunctionCall> FailWithAmbiguousOverload(const std::u32string& groupName, std::vector<std::unique_ptr<BoundExpression>>& arguments, const std::vector<FunctionMatch>& functionMatches, 
    const Span& span, OverloadResolutionFlags flags, std::unique_ptr<Exception>& exception)
{
    std::string overloadName = MakeOverloadName(groupName, arguments);
    std::string matchedFunctionNames;
    bool first = true;
    FunctionMatch equalMatch = std::move(functionMatches[0]);
    std::vector<FunctionMatch> equalMatches;
    equalMatches.push_back(std::move(equalMatch));
    int n = int(functionMatches.size());
    for (int i = 1; i < n; ++i)
    {
        FunctionMatch match = std::move(functionMatches[i]);
        if (!BetterFunctionMatch()(equalMatches[0], match))
        {
            equalMatches.push_back(std::move(match));
        }
    }
    std::vector<Span> references;
    for (const FunctionMatch& match : equalMatches)
    {
        if (first)
        {
            first = false;
        }
        else
        {
            matchedFunctionNames.append(", or ");
        }
        matchedFunctionNames.append(ToUtf8(match.fun->FullName()));
        references.push_back(match.fun->GetSpan());
    }
    if ((flags & OverloadResolutionFlags::dontThrow) != OverloadResolutionFlags::none)
    {
        exception.reset(new Exception("overload resolution for overload name '" + overloadName + "' failed: call is ambiguous: \n" + matchedFunctionNames, span, references));
        return std::unique_ptr<BoundFunctionCall>();
    }
    else
    {
        throw Exception("overload resolution for overload name '" + overloadName + "' failed: call is ambiguous: \n" + matchedFunctionNames, span, references);
    }
}

std::unique_ptr<BoundFunctionCall> CreateBoundFunctionCall(FunctionSymbol* bestFun, std::vector<std::unique_ptr<BoundExpression>>& arguments, BoundCompileUnit& boundCompileUnit, 
    BoundFunction* boundFunction, const FunctionMatch& bestMatch, const Span& span)
{
    std::unique_ptr<BoundFunctionCall> boundFunctionCall(new BoundFunctionCall(span, bestFun));
    int arity = arguments.size();
    for (int i = 0; i < arity; ++i)
    {
        std::unique_ptr<BoundExpression>& argument = arguments[i];
        const ArgumentMatch& argumentMatch = bestMatch.argumentMatches[i];
        if (argumentMatch.conversionFun)
        {
            BoundConversion* conversion = new BoundConversion(std::move(argument), argumentMatch.conversionFun);
            argument.reset(conversion);
        }
        if (argumentMatch.referenceConversionFlags != BoundExpressionFlags::none)
        {
            if (argumentMatch.referenceConversionFlags == BoundExpressionFlags::addr)
            {
                if (!argument->IsLvalueExpression())
                {
                    BoundLocalVariable* backingStore = new BoundLocalVariable(boundFunction->GetFunctionSymbol()->CreateTemporary(argument->GetType(), span));
                    backingStore->SetFlag(BoundExpressionFlags::addr);
                    argument.reset(new BoundTemporary(std::move(argument), std::unique_ptr<BoundLocalVariable>(backingStore)));
                }
                argument->SetFlag(BoundExpressionFlags::addr);
                TypeSymbol* type = boundCompileUnit.GetSymbolTable().MakeLvalueReferenceType(argument->GetType(), span);
                BoundAddressOfExpression* addressOfExpression = new BoundAddressOfExpression(std::move(argument), type);
                argument.reset(addressOfExpression);
            }
            else if (argumentMatch.referenceConversionFlags == BoundExpressionFlags::deref)
            {
                argument->SetFlag(BoundExpressionFlags::deref);
                TypeSymbol* type = argument->GetType()->RemoveReference(span);
                BoundDereferenceExpression* dereferenceExpression = new BoundDereferenceExpression(std::move(argument), type);
                argument.reset(dereferenceExpression);
            }
            else if (argumentMatch.referenceConversionFlags == BoundExpressionFlags::load)
            {
                TypeSymbol* type = argument->GetType();
                BoundDereferenceExpression* dereferenceExpression = new BoundDereferenceExpression(std::move(argument), type);
                argument.reset(dereferenceExpression);
            }
        }
        boundFunctionCall->AddArgument(std::move(argument));
    }
    return boundFunctionCall;
}

std::unique_ptr<BoundFunctionCall> SelectViableFunction(const std::unordered_set<FunctionSymbol*>& viableFunctions, const std::u32string& groupName, 
    std::vector<std::unique_ptr<BoundExpression>>& arguments, BoundCompileUnit& boundCompileUnit, BoundFunction* boundFunction, const Span& span, 
    OverloadResolutionFlags flags, std::unique_ptr<Exception>& exception)
{
    std::vector<FunctionMatch> functionMatches;
    std::vector<FunctionMatch> failedFunctionMatches;
    for (FunctionSymbol* viableFunction : viableFunctions)
    {
        FunctionMatch functionMatch(viableFunction);
        if (FindConversions(boundCompileUnit, viableFunction, arguments, functionMatch, ConversionType::implicit_, span))
        {
            functionMatches.push_back(functionMatch);
        }
        else
        {
            failedFunctionMatches.push_back(functionMatch);
        }
    }
    if (functionMatches.empty())
    {
        return FailWithOverloadNotFound(viableFunctions, groupName, arguments, failedFunctionMatches, span, flags, exception);
    }
    else if (functionMatches.size() > 1)
    {
        std::sort(functionMatches.begin(), functionMatches.end(), BetterFunctionMatch());
        if (BetterFunctionMatch()(functionMatches[0], functionMatches[1]))
        {
            const FunctionMatch& bestMatch = functionMatches[0];
            FunctionSymbol* bestFun = bestMatch.fun;
            return CreateBoundFunctionCall(bestFun, arguments, boundCompileUnit, boundFunction, bestMatch, span);
        }
        else
        {
            return FailWithAmbiguousOverload(groupName, arguments, functionMatches, span, flags, exception);
        }
    }
    else
    {
        const FunctionMatch& bestMatch = functionMatches[0];
        FunctionSymbol* singleBest = bestMatch.fun;
        return CreateBoundFunctionCall(singleBest, arguments, boundCompileUnit, boundFunction, bestMatch, span);
    }
}

void CollectViableFunctionsFromSymbolTable(int arity, const std::u32string& groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups, BoundCompileUnit& boundCompileUnit,
    std::unordered_set<FunctionSymbol*>& viableFunctions)
{
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
}

std::unique_ptr<BoundFunctionCall> ResolveOverload(const std::u32string& groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups, 
    std::vector<std::unique_ptr<BoundExpression>>& arguments, BoundCompileUnit& boundCompileUnit, BoundFunction* currentFunction, const Span& span)
{
    std::unique_ptr<Exception> exception;
    return ResolveOverload(groupName, functionScopeLookups, arguments, boundCompileUnit, currentFunction, span, OverloadResolutionFlags::none, exception);
}

std::unique_ptr<BoundFunctionCall> ResolveOverload(const std::u32string& groupName, const std::vector<FunctionScopeLookup>& functionScopeLookups, 
    std::vector<std::unique_ptr<BoundExpression>>& arguments, BoundCompileUnit& boundCompileUnit, BoundFunction* currentFunction, const Span& span, 
    OverloadResolutionFlags flags, std::unique_ptr<Exception>& exception)
{
    int arity = arguments.size();
    std::unordered_set<FunctionSymbol*> viableFunctions;
    boundCompileUnit.CollectViableFunctions(groupName, arguments, viableFunctions);
    if (viableFunctions.empty())
    {
        CollectViableFunctionsFromSymbolTable(arity, groupName, functionScopeLookups, boundCompileUnit, viableFunctions);
    }
    if (viableFunctions.empty())
    {
        return FailWithNoViableFunction(groupName, arguments, span, flags, exception);
    }
    else
    {
        return SelectViableFunction(viableFunctions, groupName, arguments, boundCompileUnit, currentFunction, span, flags, exception);
    }
}

} } // namespace cmajor::binder
