#ifndef Rule_hpp_20364
#define Rule_hpp_20364

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/parsing/Rule.hpp>

namespace cmajor { namespace syntax {

class RuleGrammar : public cmajor::parsing::Grammar
{
public:
    static RuleGrammar* Create();
    static RuleGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    cmajor::parsing::Rule* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cmajor::parsing::Scope* enclosingScope);
private:
    RuleGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class RuleRule;
    class RuleHeaderRule;
    class RuleBodyRule;
};

} } // namespace cmajor.syntax

#endif // Rule_hpp_20364
