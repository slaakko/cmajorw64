#ifndef Element_hpp_20364
#define Element_hpp_20364

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/parsing/Scope.hpp>
#include <cmajor/parsing/Grammar.hpp>

namespace cmajor { namespace syntax {

class ElementGrammar : public cmajor::parsing::Grammar
{
public:
    static ElementGrammar* Create();
    static ElementGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cmajor::parsing::Grammar* grammar);
private:
    std::vector<std::u32string> keywords0;
    ElementGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class RuleLinkRule;
    class SignatureRule;
    class ParameterListRule;
    class VariableRule;
    class ParameterRule;
    class ReturnTypeRule;
    class IdentifierRule;
    class QualifiedIdRule;
    class StringArrayRule;
};

} } // namespace cmajor.syntax

#endif // Element_hpp_20364
