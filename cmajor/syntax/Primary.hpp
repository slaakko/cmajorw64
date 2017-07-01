#ifndef Primary_hpp_20364
#define Primary_hpp_20364

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/parsing/Parser.hpp>
#include <cmajor/parsing/Scope.hpp>

namespace cmajor { namespace syntax {

class PrimaryGrammar : public cmajor::parsing::Grammar
{
public:
    static PrimaryGrammar* Create();
    static PrimaryGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    cmajor::parsing::Parser* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cmajor::parsing::Scope* enclosingScope);
private:
    PrimaryGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class PrimaryRule;
    class RuleCallRule;
    class NonterminalRule;
    class AliasRule;
    class GroupingRule;
    class TokenRule;
    class ExpectationRule;
    class ActionRule;
};

} } // namespace cmajor.syntax

#endif // Primary_hpp_20364
