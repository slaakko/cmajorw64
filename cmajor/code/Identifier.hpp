#ifndef Identifier_hpp_19453
#define Identifier_hpp_19453

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>

namespace cmajor { namespace code {

class IdentifierGrammar : public cmajor::parsing::Grammar
{
public:
    static IdentifierGrammar* Create();
    static IdentifierGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    std::u32string Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    IdentifierGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class IdentifierRule;
    class QualifiedIdRule;
};

} } // namespace cmajor.code

#endif // Identifier_hpp_19453
