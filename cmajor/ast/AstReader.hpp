// =================================
// Copyright (c) 2017 Seppo Laakko
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
class ConceptIdNode;

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
    ConceptIdNode* ReadConceptIdNode();
    Specifiers ReadSpecifiers();
    Span ReadSpan();
    void SetReplaceFileIndex(int32_t replaceFileIndex_) { replaceFileIndex = replaceFileIndex_; }
    void SetFileIndexOffset(int32_t fileIndexOffset_) { fileIndexOffset = fileIndexOffset_; }
private:
    BinaryReader binaryReader;
    int32_t replaceFileIndex;
    int32_t fileIndexOffset;
};

} } // namespace cmajor::ast

#endif // CMAJOR_AST_AST_READER_INCLUDED
