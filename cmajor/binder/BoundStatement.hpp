// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_BOUND_STATEMENT_INCLUDED
#define CMAJOR_BINDER_BOUND_STATEMENT_INCLUDED
#include <cmajor/binder/BoundNode.hpp>
#include <cmajor/symbols/VariableSymbol.hpp>
#include <cmajor/symbols/Value.hpp>
#include <cmajor/ast/Statement.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::symbols;
using namespace cmajor::ast;

class BoundFunctionCall;
class BoundExpression;
class BoundCompoundStatement;

class BoundStatement : public BoundNode
{
public:
    BoundStatement(const Span& span_, BoundNodeType boundNodeType_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    BoundStatement* Parent() { return parent; }
    void SetParent(BoundStatement* parent_) { parent = parent_; }
    BoundCompoundStatement* Block();
    void SetLabel(const std::u32string& label_);
    const std::u32string& Label() const { return label; }
    void SetPostfix() { postfix = true; }
    bool Postfix() { return postfix; }
private:
    std::u32string label;
    BoundStatement* parent;
    bool postfix;
};

class BoundSequenceStatement : public BoundStatement
{
public:
    BoundSequenceStatement(const Span& span_, std::unique_ptr<BoundStatement>&& first_, std::unique_ptr<BoundStatement>&& second_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundStatement* First() { return first.get(); }
    BoundStatement* Second() { return second.get(); }
private:
    std::unique_ptr<BoundStatement> first;
    std::unique_ptr<BoundStatement> second;
};

class BoundCompoundStatement : public BoundStatement
{
public:
    BoundCompoundStatement(const Span& span_);
    void Accept(BoundNodeVisitor& visitor) override;
    void AddStatement(std::unique_ptr<BoundStatement>&& statement);
    const std::vector<std::unique_ptr<BoundStatement>>& Statements() const { return statements; }
private:
    std::vector<std::unique_ptr<BoundStatement>> statements;
};

class BoundReturnStatement : public BoundStatement
{
public:
    BoundReturnStatement(std::unique_ptr<BoundFunctionCall>&& returnFunctionCall_, const Span& span_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundFunctionCall* ReturnFunctionCall() { return returnFunctionCall.get(); }
private:
    std::unique_ptr<BoundFunctionCall> returnFunctionCall;
};

class BoundIfStatement : public BoundStatement
{
public:
    BoundIfStatement(const Span& span_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& thenS_, std::unique_ptr<BoundStatement>&& elseS_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundExpression* Condition() { return condition.get(); }
    BoundStatement* ThenS() { return thenS.get(); }
    BoundStatement* ElseS() { return elseS.get(); }
private:
    std::unique_ptr<BoundExpression> condition;
    std::unique_ptr<BoundStatement> thenS;
    std::unique_ptr<BoundStatement> elseS;
};

class BoundWhileStatement : public BoundStatement
{
public:
    BoundWhileStatement(const Span& span_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& statement_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundExpression* Condition() { return condition.get(); }
    BoundStatement* Statement() { return statement.get(); }
private:
    std::unique_ptr<BoundExpression> condition;
    std::unique_ptr<BoundStatement> statement;
};

class BoundDoStatement : public BoundStatement
{
public:
    BoundDoStatement(const Span& span_, std::unique_ptr<BoundStatement>&& statement_, std::unique_ptr<BoundExpression>&& condition_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundStatement* Statement() { return statement.get(); }
    BoundExpression* Condition() { return condition.get(); }
private:
    std::unique_ptr<BoundStatement> statement;
    std::unique_ptr<BoundExpression> condition;
};

class BoundForStatement : public BoundStatement
{
public:
    BoundForStatement(const Span& span_, std::unique_ptr<BoundStatement>&& initS_, std::unique_ptr<BoundExpression>&& condition_, std::unique_ptr<BoundStatement>&& loopS_, std::unique_ptr<BoundStatement>&& actionS_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundStatement* InitS() { return initS.get(); }
    BoundExpression* Condition() { return condition.get(); }
    BoundStatement* LoopS() { return loopS.get(); }
    BoundStatement* ActionS() { return actionS.get(); }
private:
    std::unique_ptr<BoundStatement> initS;
    std::unique_ptr<BoundExpression> condition;
    std::unique_ptr<BoundStatement> loopS;
    std::unique_ptr<BoundStatement> actionS;
};

class BoundCaseStatement;
class BoundDefaultStatement;

class BoundSwitchStatement : public BoundStatement
{
public:
    BoundSwitchStatement(const Span& span_, std::unique_ptr<BoundExpression>&& condition_);
    BoundExpression* Condition() { return condition.get(); }
    const std::vector<std::unique_ptr<BoundCaseStatement>>& CaseStatements() { return caseStatements; }
    void AddCaseStatement(std::unique_ptr<BoundCaseStatement>&& caseStatement);
    BoundDefaultStatement* DefaultStatement() { return defaultStatement.get(); }
    void SetDefaultStatement(std::unique_ptr<BoundDefaultStatement>&& defaultStatement_);
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundExpression> condition;
    std::vector<std::unique_ptr<BoundCaseStatement>> caseStatements;
    std::unique_ptr<BoundDefaultStatement> defaultStatement;
};

class BoundCaseStatement : public BoundStatement
{
public:
    BoundCaseStatement(const Span& span_);
    void AddCaseValue(std::unique_ptr<Value>&& caseValue_);
    const std::vector<std::unique_ptr<Value>>& CaseValues() const { return caseValues; }
    void AddStatement(std::unique_ptr<BoundStatement>&& statement);
    BoundCompoundStatement* CompoundStatement() { return &compoundStatement; }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::vector<std::unique_ptr<Value>> caseValues;
    BoundCompoundStatement compoundStatement;
};

class BoundDefaultStatement : public BoundStatement
{
public:
    BoundDefaultStatement(const Span& span_);
    void AddStatement(std::unique_ptr<BoundStatement>&& statement);
    BoundCompoundStatement* CompoundStatement() { return &compoundStatement; }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    BoundCompoundStatement compoundStatement;
};

class BoundGotoCaseStatement : public BoundStatement
{
public:
    BoundGotoCaseStatement(const Span& span_, std::unique_ptr<Value>&& caseValue_);
    void Accept(BoundNodeVisitor& visitor) override;
    Value* CaseValue() { return caseValue.get(); }
private:
    std::unique_ptr<Value> caseValue;
};

class BoundGotoDefaultStatement : public BoundStatement
{
public:
    BoundGotoDefaultStatement(const Span& span_);
    void Accept(BoundNodeVisitor& visitor) override;
};

class BoundBreakStatement : public BoundStatement
{
public:
    BoundBreakStatement(const Span& span_);
    void Accept(BoundNodeVisitor& visitor) override;
};

class BoundContinueStatement : public BoundStatement
{
public:
    BoundContinueStatement(const Span& span_);
    void Accept(BoundNodeVisitor& visitor) override;
};

class BoundGotoStatement : public BoundStatement
{
public:
    BoundGotoStatement(const Span& span_, const std::u32string& target_);
    void Accept(BoundNodeVisitor& visitor) override;
    const std::u32string& Target() const { return target; }
    void SetTargetStatement(BoundStatement* targetStatement_) { targetStatement = targetStatement_; }
    BoundStatement* TargetStatement() { return targetStatement; }
    void SetTargetBlock(BoundCompoundStatement* targetBlock_) { targetBlock = targetBlock_; }
    BoundCompoundStatement* TargetBlock() { return targetBlock; }
private:
    std::u32string target;
    BoundStatement* targetStatement;
    BoundCompoundStatement* targetBlock;
};

class BoundConstructionStatement : public BoundStatement
{
public:
    BoundConstructionStatement(std::unique_ptr<BoundFunctionCall>&& constructorCall_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundFunctionCall* ConstructorCall() { return constructorCall.get(); }
private:
    std::unique_ptr<BoundFunctionCall> constructorCall;
};

class BoundAssignmentStatement : public BoundStatement
{
public:
    BoundAssignmentStatement(std::unique_ptr<BoundFunctionCall>&& assignmentCall_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundFunctionCall* AssignmentCall() { return assignmentCall.get(); }
private:
    std::unique_ptr<BoundFunctionCall> assignmentCall;
};

class BoundExpressionStatement : public BoundStatement
{
public:
    BoundExpressionStatement(std::unique_ptr<BoundExpression>&& expression_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundExpression* Expression() { return expression.get(); }
private:
    std::unique_ptr<BoundExpression> expression;
};

class BoundEmptyStatement : public BoundStatement
{
public:
    BoundEmptyStatement(const Span& span_);
    void Accept(BoundNodeVisitor& visitor) override;
};

class BoundSetVmtPtrStatement : public BoundStatement
{
public:
    BoundSetVmtPtrStatement(std::unique_ptr<BoundExpression>&& classPtr_, ClassTypeSymbol* classType_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundExpression* ClassPtr() { return classPtr.get(); }
    ClassTypeSymbol* ClassType() { return classType; }
private:
    std::unique_ptr<BoundExpression> classPtr;
    ClassTypeSymbol* classType;
};

class BoundThrowStatement : public BoundStatement
{
public:
    BoundThrowStatement(const Span& span_, std::unique_ptr<BoundExpression>&& throwCall_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundExpression* ThrowCall() { return throwCall.get(); }
private:
    std::unique_ptr<BoundExpression> throwCall;
};

class BoundRethrowStatement : public BoundStatement
{
public:
    BoundRethrowStatement(const Span& span_, std::unique_ptr<BoundExpression>&& releaseCall_);
    void Accept(BoundNodeVisitor& visitor) override;
    BoundExpression* ReleaseCall() { return releaseCall.get(); }
private:
    std::unique_ptr<BoundExpression> releaseCall;
};

class BoundCatchStatement;

class BoundTryStatement : public BoundStatement
{
public:
    BoundTryStatement(const Span& span_);
    void SetTryBlock(std::unique_ptr<BoundStatement>&& tryBlock_);
    BoundStatement* TryBlock() { return tryBlock.get(); }
    void AddCatch(std::unique_ptr<BoundCatchStatement>&& catchStatement);
    const std::vector<std::unique_ptr<BoundCatchStatement>>& Catches() const { return catches; }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    std::unique_ptr<BoundStatement> tryBlock;
    std::vector<std::unique_ptr<BoundCatchStatement>> catches;
};

class BoundCatchStatement : public BoundStatement
{
public:
    BoundCatchStatement(const Span& span_);
    void SetCatchedType(TypeSymbol* catchedType_) { catchedType = catchedType_; }
    TypeSymbol* CatchedType() { return catchedType; }
    void SetCatchVar(LocalVariableSymbol* catchVar_) { catchVar = catchVar_; }
    LocalVariableSymbol* CatchVar() { return catchVar; }
    void SetCatchBlock(std::unique_ptr<BoundStatement>&& catchBlock_);
    BoundStatement* CatchBlock() { return catchBlock.get(); }
    void Accept(BoundNodeVisitor& visitor) override;
private:
    TypeSymbol* catchedType;
    LocalVariableSymbol* catchVar;
    std::unique_ptr<BoundStatement> catchBlock;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_BOUND_STATEMENT_INCLUDED
