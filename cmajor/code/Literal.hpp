#ifndef Literal_hpp_19453
#define Literal_hpp_19453

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/codedom/Literal.hpp>

namespace cmajor { namespace code {

class LiteralGrammar : public cmajor::parsing::Grammar
{
public:
    static LiteralGrammar* Create();
    static LiteralGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    cmajor::codedom::Literal* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    LiteralGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class LiteralRule;
    class IntegerLiteralRule;
    class CharacterLiteralRule;
    class CCharSequenceRule;
    class FloatingLiteralRule;
    class StringLiteralRule;
    class BooleanLiteralRule;
    class PointerLiteralRule;
};

} } // namespace cmajor.code

#endif // Literal_hpp_19453
