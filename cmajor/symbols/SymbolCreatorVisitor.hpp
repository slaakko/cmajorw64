// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_SYMBOL_CREATOR_VISITOR_INCLUDED
#define CMAJOR_SYMBOLS_SYMBOL_CREATOR_VISITOR_INCLUDED
#include <cmajor/ast/Visitor.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::ast;

class SymbolTable;
class ClassTemplateSpecializationSymbol;

class SymbolCreatorVisitor : public Visitor
{
public:
    SymbolCreatorVisitor(SymbolTable& symbolTable_);
    void SetClassInstanceNode(ClassNode* classInstanceNode_);
    void SetClassTemplateSpecialization(ClassTemplateSpecializationSymbol* classTemplateSpecialization_);
    void Visit(CompileUnitNode& compileUnitNode) override;
    void Visit(NamespaceNode& namespaceNode) override;
    void Visit(FunctionNode& functionNode) override;
    void Visit(ParameterNode& parameterNode) override;
    void Visit(ClassNode& classNode) override;
    void Visit(StaticConstructorNode& staticConstructorNode) override;
    void Visit(ConstructorNode& constructorNode) override;
    void Visit(DestructorNode& destructorNode) override;
    void Visit(MemberFunctionNode& memberFunctionNode) override;
    void Visit(MemberVariableNode& memberVariableNode) override;
    void Visit(InterfaceNode& interfaceNode) override;
    void Visit(DelegateNode& delegateNode) override;
    void Visit(ClassDelegateNode& classDelegateNode) override;
    void Visit(ConceptNode& conceptNode) override;
    void Visit(CompoundStatementNode& compoundStatementNode) override;
    void Visit(IfStatementNode& ifStatementNode) override;
    void Visit(WhileStatementNode& whileStatementNode) override;
    void Visit(DoStatementNode& doStatementNode) override;
    void Visit(ForStatementNode& forStatementNode) override;
    void Visit(ConstructionStatementNode& constructionStatementNode) override;
    void Visit(SwitchStatementNode& switchStatementNode) override;
    void Visit(CaseStatementNode& caseStatementNode) override;
    void Visit(DefaultStatementNode& defaultStatementNode) override;
    void Visit(CatchNode& catchNode) override;
    void Visit(TryStatementNode& tryStatementNode) override;
    void Visit(TypedefNode& typedefNode) override;
    void Visit(ConstantNode& constantNode) override;
    void Visit(EnumTypeNode& enumTypeNode) override;
    void Visit(EnumConstantNode& enumConstantNode) override;
private:
    SymbolTable& symbolTable;
    ClassNode* classInstanceNode;
    ClassTemplateSpecializationSymbol* classTemplateSpecialization;
};

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_SYMBOL_CREATOR_VISITOR_INCLUDED