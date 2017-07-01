#ifndef Keyword_hpp_19453
#define Keyword_hpp_19453

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>

namespace cmajor { namespace code {

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

} } // namespace cmajor.code

#endif // Keyword_hpp_19453
