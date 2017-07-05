// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_PARSER_PARSING_CONTEXT_INCLUDED
#define CMAJOR_PARSER_PARSING_CONTEXT_INCLUDED
#include <stack>

namespace cmajor { namespace parser {

class ParsingContext
{
public:
    ParsingContext();
    bool ParsingExpressionStatement() const { return parsingExpressionStatement; }
    void PushParsingExpressionStatement(bool enable);
    void PopParsingExpressionStatement();
    bool ParsingLvalue() const { return parsingLvalue; }
    void PushParsingLvalue(bool enable);
    void PopParsingLvalue();
    bool ParsingArguments() const { return parsingArguments; }
    void BeginParsingArguments();
    void EndParsingArguments();
    bool ParsingIsOrAs() const { return parsingIsOrAs; }
    void PushParsingIsOrAs(bool enable);
    void PopParsingIsOrAs();
    bool ParsingTypeExpr() const { return parsingTypeExpr; }
    void BeginParsingTypeExpr();
    void EndParsingTypeExpr();
    bool ParsingConcept() const { return parsingConcept; }
    void BeginParsingConcept();
    void EndParsingConcept();
private:
    bool parsingExpressionStatement;
    std::stack<bool> parsingExpressionStatementStack;
    bool parsingLvalue;
    std::stack<bool> parsingLvalueStack;
    bool parsingArguments;
    std::stack<bool> parsingArgumentsStack;
    bool parsingIsOrAs;
    std::stack<bool> parsingIsOrAsStack;
    bool parsingTypeExpr;
    std::stack<bool> parsingTypeExprStack;
    bool parsingConcept;
    std::stack<bool> parsingConceptStack;
};

} } // namespace cmajor::parser

#endif // CMAJOR_PARSER_PARSING_CONTEXT_INCLUDED
