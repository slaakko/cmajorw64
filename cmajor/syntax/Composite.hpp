#ifndef Composite_hpp_20364
#define Composite_hpp_20364

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/parsing/Scope.hpp>
#include <cmajor/parsing/Parser.hpp>

namespace cmajor { namespace syntax {

class CompositeGrammar : public cmajor::parsing::Grammar
{
public:
    static CompositeGrammar* Create();
    static CompositeGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    cmajor::parsing::Parser* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cmajor::parsing::Scope* enclosingScope);
private:
    CompositeGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class AlternativeRule;
    class SequenceRule;
    class DifferenceRule;
    class ExclusiveOrRule;
    class IntersectionRule;
    class ListRule;
    class PostfixRule;
};

} } // namespace cmajor.syntax

#endif // Composite_hpp_20364
