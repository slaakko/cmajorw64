#ifndef Keyword_hpp_19780
#define Keyword_hpp_19780

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>

namespace cmajor { namespace parser {

class KeywordGrammar : public cmajor::parsing::Grammar
{
public:
    static KeywordGrammar* Create();
    static KeywordGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
private:
    std::vector<std::u32string> keywords0;
    KeywordGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
};

} } // namespace cmajor.parser

#endif // Keyword_hpp_19780
