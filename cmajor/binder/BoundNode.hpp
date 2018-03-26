// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_BOUND_NODE_INCLUDED
#define CMAJOR_BINDER_BOUND_NODE_INCLUDED
#include <cmajor/parsing/Scanner.hpp>
#include <cmajor/ir/GenObject.hpp>

namespace cmajor { namespace binder {

using cmajor::parsing::Span;
using namespace cmajor::ir;

enum class BoundNodeType : uint8_t
{
    boundCompileUnit, boundClass, boundFunction, 
    boundSequenceStatement, boundCompoundStatement, boundReturnStatement, boundIfStatement, boundWhileStatement, boundDoStatement, boundForStatement, 
    boundSwitchStatement, boundCaseStatement, boundDefaultStatement, boundGotoCaseStatement, boundGotoDefaultStatement, boundBreakStatement, boundContinueStatement,
    boundGotoStatement, boundConstructionStatement, boundAssignmentStatement, boundExpressionStatement, boundEmptyStatement, boundSetVmtPtrStatement, 
    boundThrowStatement, boundRethrowStatement, boundTryStatement, boundCatchStatement,
    boundParameter, boundLocalVariable, boundMemberVariable, boundConstant, boundEnumConstant, boundLiteral, boundTemporary, boundBitCast, boundFunctionPtr, boundConstructAndReturnTemporary,
    boundClassOrClassDelegateConversionResult,
    boundFunctionCall, boundDelegateCall, boundClassDelegateCall, boundConversion, boundSizeOfExpression, boundAddressOfExpression, boundDereferenceExpression, boundReferenceToPointerExpression,
    boundConstructExpression, boundIsExpression, boundAsExpression, boundTypeNameExpression, boundDisjunction, boundConjunction,
    boundAtomicConstraint, boundDisjunctiveConstraint, boundConjunctiveConstraint, boundConcept,
    boundTypeExpression, boundNamespaceExpression, boundFunctionGroupExpression, boundMemberExpression,
    boundNamespace, boundEnumTypeDefinition
};

class BoundNodeVisitor;

class BoundNode : public GenObject
{
public:
    BoundNode(const Span& span_, BoundNodeType boundNodeType_);
    virtual void Accept(BoundNodeVisitor& visitor) = 0;
    const Span& GetSpan() const { return span; }
    BoundNodeType GetBoundNodeType() const { return boundNodeType; }
private:
    Span span;
    BoundNodeType boundNodeType;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_BOUND_NODE_INCLUDED
