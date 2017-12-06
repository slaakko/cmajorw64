#ifndef Literal_hpp_24813
#define Literal_hpp_24813

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Literal.hpp>
#include <cmajor/parser/ParsingContext.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class LiteralGrammar : public cmajor::parsing::Grammar
{
public:
    static LiteralGrammar* Create();
    static LiteralGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    LiteralGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class LiteralRule;
    class BooleanLiteralRule;
    class FloatingLiteralRule;
    class FloatingLiteralValueRule;
    class IntegerLiteralRule;
    class IntegerLiteralValueRule;
    class HexIntegerLiteralRule;
    class DecIntegerLiteralRule;
    class CharLiteralRule;
    class StringLiteralRule;
    class NullLiteralRule;
    class ArrayLiteralRule;
    class CharEscapeRule;
    class DecDigitSequenceRule;
    class HexDigitSequenceRule;
    class HexDigit4Rule;
    class HexDigit8Rule;
    class OctalDigitSequenceRule;
};

} } // namespace cmajor.parser

#endif // Literal_hpp_24813
