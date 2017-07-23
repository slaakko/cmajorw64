// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_BOUND_NODE_VISITOR_INCLUDED
#define CMAJOR_BINDER_BOUND_NODE_VISITOR_INCLUDED

namespace cmajor { namespace binder {

class BoundCompileUnit;
class BoundClass;
class BoundFunction;
class BoundSequenceStatement;
class BoundCompoundStatement;
class BoundReturnStatement;
class BoundIfStatement;
class BoundWhileStatement;
class BoundDoStatement;
class BoundForStatement;
class BoundBreakStatement;
class BoundContinueStatement;
class BoundGotoStatement;
class BoundConstructionStatement;
class BoundAssignmentStatement;
class BoundExpressionStatement;
class BoundEmptyStatement;
class BoundParameter;
class BoundLocalVariable;
class BoundMemberVariable;
class BoundConstant;
class BoundEnumConstant;
class BoundLiteral;
class BoundFunctionCall;
class BoundConversion;

class BoundNodeVisitor
{
public:
    virtual ~BoundNodeVisitor();
    virtual void Visit(BoundCompileUnit& boundCompileUnit) {}
    virtual void Visit(BoundClass& boundClass) {}
    virtual void Visit(BoundFunction& boundFunction) {}
    virtual void Visit(BoundSequenceStatement& boundSequenceStatement) {}
    virtual void Visit(BoundCompoundStatement& boundCompoundStatement) {}
    virtual void Visit(BoundReturnStatement& boundReturnStatement) {}
    virtual void Visit(BoundIfStatement& boundIfStatement) {}
    virtual void Visit(BoundWhileStatement& boundWhileStatement) {}
    virtual void Visit(BoundDoStatement& boundDoStatement) {}
    virtual void Visit(BoundForStatement& boundForStatement) {}
    virtual void Visit(BoundBreakStatement& boundBreakStatement) {}
    virtual void Visit(BoundContinueStatement& boundContinueStatement) {}
    virtual void Visit(BoundGotoStatement& boundGotoStatement) {}
    virtual void Visit(BoundConstructionStatement& boundConstructionStatement) {}
    virtual void Visit(BoundAssignmentStatement& boundAssignmentStatement) {}
    virtual void Visit(BoundExpressionStatement& boundExpressionStatement) {}
    virtual void Visit(BoundEmptyStatement& boundEmptyStatement) {}
    virtual void Visit(BoundParameter& boundParameter) {}
    virtual void Visit(BoundLocalVariable& boundLocalVariable) {}
    virtual void Visit(BoundMemberVariable& boundMemberVariable) {}
    virtual void Visit(BoundConstant& boundConstant) {}
    virtual void Visit(BoundEnumConstant& boundEnumConstant) {}
    virtual void Visit(BoundLiteral& boundLiteral) {}
    virtual void Visit(BoundFunctionCall& boundFunctionCall) {}
    virtual void Visit(BoundConversion& boundConversion) {}
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_BOUND_NODE_VISITOR_INCLUDED
