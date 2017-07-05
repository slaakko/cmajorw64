// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_NODE_INCLUDED
#define CMAJOR_AST_NODE_INCLUDED
#include <cmajor/parsing/Scanner.hpp>
#include <cmajor/ast/Clone.hpp>
#include <memory>
#include <stdint.h>

namespace cmajor { namespace ast {

using cmajor::parsing::Span;

class Visitor;
class AstWriter;
class AstReader;
class ParameterNode;
class TemplateParameterNode;

enum class NodeType : uint8_t
{
    boolNode, sbyteNode, byteNode, shortNode, ushortNode, intNode, uintNode, longNode, ulongNode, floatNode, doubleNode, charNode, wcharNode, ucharNode, voidNode,
    booleanLiteralNode, sbyteLiteralNode, byteLiteralNode, shortLiteralNode, ushortLiteralNode, intLiteralNode, uintLiteralNode, longLiteralNode, ulongLiteralNode, 
    floatLiteralNode, doubleLiteralNode, charLiteralNode, wcharLiteralNode, ucharLiteralNode, stringLiteralNode, wstringLiteralNode, ustringLiteralNode, nullLiteralNode,
    compileUnitNode, namespaceNode, aliasNode, namespaceImportNode, identifierNode, templateIdNode, functionNode, 
    disjunctiveConstraintNode, conjunctiveConstraintNode, whereConstraintNode, predicateConstraintNode, isConstraintNode, multiParamConstraintNode, typeNameConstraintNode,
    constructorConstraintNode, destructorConstraintNode, memberFunctionConstraintNode, functionConstraintNode, 
    axiomStatementNode, axiomNode, conceptIdNode, conceptNode, 
    labelNode, compoundStatementNode, returnStatementNode, ifStatementNode, whileStatementNode, doStatementNode, forStatementNode, breakStatementNode, continueStatementNode,
    gotoStatementNode, constructionStatementNode, deleteStatementNode, destroyStatementNode, assignmentStatementNode, expressionStatementNode, emptyStatementNode, 
    incrementStatementNode, decrementStatementNode, rangeForStatementNode, switchStatementNode, caseStatementNode, defaultStatementNode, gotoCaseStatementNode, gotoDefaultStatementNode, 
    throwStatementNode, catchNode, tryStatementNode,
    typedefNode, constantNode, enumTypeNode, enumConstantNode, parameterNode, templateParameterNode,
    delegateNode, classDelegateNode, 
    constNode, lvalueRefNode, rvalueRefNode, pointerNode, arrayNode,
    dotNode, arrowNode, disjunctionNode, conjunctionNode, bitOrNode, bitXorNode, bitAndNode, equalNode, notEqualNode, lessNode, greaterNode, lessOrEqualNode, greaterOrEqualNode, shiftLeftNode, shiftRightNode,
    addNode, subNode, mulNode, divNode, remNode, notNode, unaryPlusNode, unaryMinusNode, complementNode, derefNode, addrOfNode,
    isNode, asNode, indexingNode, invokeNode, sizeOfNode, typeNameNode, castNode, constructNode, newNode, thisNode, baseNode,
    maxNode
};

std::string NodeTypeStr(NodeType nodeType);

class Node
{
public:
    Node(NodeType nodeType_, const Span& span_);
    virtual ~Node();
    NodeType GetNodeType() const { return nodeType; }
    virtual Node* Clone(CloneContext& cloneContext) const = 0;
    virtual void Accept(Visitor& visitor) = 0;
    virtual void Write(AstWriter& writer);
    virtual void Read(AstReader& reader);
    virtual void AddArgument(Node* argument) { Assert(false, "AddArgument not overridden");  }
    virtual void AddParameter(ParameterNode* parameter) { Assert(false, "AddParameter not overridden"); }
    virtual void AddTemplateParameter(TemplateParameterNode* templateParameter) { Assert(false, "AddTemplateParameter not overridden"); }
    virtual bool IsUnsignedTypeNode() const { return false; }
    virtual bool IsStatementNode() const { return false; }
    virtual bool IsConstraintNode() const { return false; }
    const Span& GetSpan() const { return span; }
    Span& GetSpan() { return span; }
    const Node* Parent() const { return parent; }
    void SetParent(Node* parent_) { parent = parent_; }
private:
    NodeType nodeType;
    Span span;
    Node* parent;
};

class BinaryNode : public Node
{
public:
    BinaryNode(NodeType nodeType, const Span& span_);
    BinaryNode(NodeType nodeType, const Span& span_, Node* left_, Node* right_);
    void Write(AstWriter& writer) override;
    void Read(AstReader& reader) override;
    const Node* Left() const { return left.get(); }
    const Node* Right() const { return right.get(); }
private:
    std::unique_ptr<Node> left;
    std::unique_ptr<Node> right;
};

class NodeCreator
{
public:
    virtual ~NodeCreator();
    virtual Node* CreateNode(const Span& span) = 0;
};

class NodeFactory
{
public:
    static NodeFactory& Instance() { Assert(instance, "node factory not initialized"); return *instance; }
    static void Init();
    static void Done();
    void Register(NodeType nodeType, NodeCreator* creator);
    Node* CreateNode(NodeType nodeType, const Span& span);
private:
    static std::unique_ptr<NodeFactory> instance;
    std::vector<std::unique_ptr<NodeCreator>> creators;
    NodeFactory();
};

void NodeInit();
void NodeDone();

} } // namespace cmajor::ast

#endif // CMAJOR_AST_NODE_INCLUDED
