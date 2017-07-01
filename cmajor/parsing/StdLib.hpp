#ifndef StdLib_hpp_18303
#define StdLib_hpp_18303

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <stdint.h>

namespace cmajor { namespace parsing {

class stdlib : public cmajor::parsing::Grammar
{
public:
    static stdlib* Create();
    static stdlib* Create(cmajor::parsing::ParsingDomain* parsingDomain);
private:
    stdlib(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class intRule;
    class uintRule;
    class longRule;
    class ulongRule;
    class hexuintRule;
    class hexRule;
    class hex_literalRule;
    class realRule;
    class urealRule;
    class numRule;
    class boolRule;
    class identifierRule;
    class qualified_idRule;
    class escapeRule;
    class charRule;
    class stringRule;
};

} } // namespace cmajor.parsing

#endif // StdLib_hpp_18303
