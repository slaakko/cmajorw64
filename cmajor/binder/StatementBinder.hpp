// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_STATEMENT_BINDER_INCLUDED
#define CMAJOR_BINDER_STATEMENT_BINDER_INCLUDED
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/ast/Visitor.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::ast;

class BoundStatement;
class BoundFunction;

class StatementBinder : public Visitor
{
public:
    StatementBinder(BoundCompileUnit& boundCompileUnit_);
    void Visit(CompileUnitNode& compileUnitNode) override;
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(ClassNode& classNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(CompoundStatementNode& compoundStatementNode) override;
    void Visit(ReturnStatementNode& returnStatementNode) override;
    void Visit(IfStatementNode& ifStatementNode) override;
    void Visit(WhileStatementNode& whileStatementNode) override;
    void Visit(DoStatementNode& doStatementNode) override;
    void Visit(ForStatementNode& forStatementNode) override;
    void Visit(BreakStatementNode& breakStatementNode) override;
    void Visit(ContinueStatementNode& continueStatementNode) override;
    void Visit(GotoStatementNode& gotoStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
    void Visit(DeleteStatementNode& deleteStatementNode) override;
    void Visit(DestroyStatementNode& destroyStatementNode) override;
    void Visit(AssignmentStatementNode& assignmentStatementNode) override;
    void Visit(ExpressionStatementNode& expressionStatementNode) override;
    void Visit(EmptyStatementNode& emptyStatementNode) override;
    void Visit(RangeForStatementNode& rangeForStatementNode) override;
    void Visit(SwitchStatementNode& switchStatementNode) override;
    void Visit(CaseStatementNode& caseStatementNode) override;
    void Visit(DefaultStatementNode& defaultStatementNode) override;
    void Visit(GotoCaseStatementNode& gotoCaseStatementNode) override;
    void Visit(GotoDefaultStatementNode& gotoDefaultStatementNode) override;
    void Visit(ThrowStatementNode& throwStatementNode) override;
    void Visit(CatchNode& catchNode) override;
    void Visit(TryStatementNode& tryStatementNode) override;
    void Visit(AssertStatementNode& assertStatementNode) override;
    void CompileStatement(Node* statementNode, bool setPostfix);
private:
    BoundCompileUnit& boundCompileUnit;
    SymbolTable& symbolTable;
    ContainerScope* containerScope;
    std::unique_ptr<BoundStatement> statement;
    BoundFunction* currentFunction;
    bool postfix;
    void AddStatement(BoundStatement* boundStatement);
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_STATEMENT_BINDER_INCLUDED
