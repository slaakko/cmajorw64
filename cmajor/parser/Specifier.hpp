#ifndef Specifier_hpp_28781
#define Specifier_hpp_28781

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Specifier.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class SpecifierGrammar : public cmajor::parsing::Grammar
{
public:
    static SpecifierGrammar* Create();
    static SpecifierGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    Specifiers Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    SpecifierGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class SpecifiersRule;
    class SpecifierRule;
};

} } // namespace cmajor.parser

#endif // Specifier_hpp_28781
