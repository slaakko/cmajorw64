// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_AST_AST_READER_INCLUDED
#define CMAJOR_AST_AST_READER_INCLUDED
#include <cmajor/ast/Specifier.hpp>
#include <cmajor/util/BinaryReader.hpp>
#include <cmajor/parsing/Scanner.hpp>

namespace cmajor { namespace ast {

using namespace cmajor::util;
using cmajor::parsing::Span;
class Node;
class IdentifierNode;
class LabelNode;
class StatementNode;
class DefaultStatementNode;
class CompoundStatementNode;
class ConstraintNode;
class WhereConstraintNode;
class ConceptIdNode;
class ConceptNode;
class ConditionalCompilationExpressionNode;
class ConditionalCompilationPartNode;

class AstReader
{
public:
    AstReader(const std::string& fileName_);
    BinaryReader& GetBinaryReader() { return binaryReader; }
    Node* ReadNode();
    IdentifierNode* ReadIdentifierNode();
    LabelNode* ReadLabelNode();
    StatementNode* ReadStatementNode();
    DefaultStatementNode* ReadDefaultStatementNode();
    CompoundStatementNode* ReadCompoundStatementNode();
    ConstraintNode* ReadConstraintNode();
    WhereConstraintNode* ReadWhereConstraintNode();
    ConceptIdNode* ReadConceptIdNode();
    ConceptNode* ReadConceptNode();
    ConditionalCompilationExpressionNode* ReadConditionalCompilationExpressionNode();
    ConditionalCompilationPartNode* ReadConditionalCompilationPartNode();
    Specifiers ReadSpecifiers();
    Span ReadSpan();
private:
    BinaryReader binaryReader;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_AST_READER_INCLUDED
