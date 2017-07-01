#ifndef Declarator_hpp_19449
#define Declarator_hpp_19449

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/codedom/Declaration.hpp>

namespace cmajor { namespace code {

class DeclaratorGrammar : public cmajor::parsing::Grammar
{
public:
    static DeclaratorGrammar* Create();
    static DeclaratorGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    cmajor::codedom::InitDeclaratorList* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    DeclaratorGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class InitDeclaratorListRule;
    class InitDeclaratorRule;
    class DeclaratorRule;
    class DirectDeclaratorRule;
    class DeclaratorIdRule;
    class TypeIdRule;
    class TypeRule;
    class TypeSpecifierSeqRule;
    class AbstractDeclaratorRule;
    class DirectAbstractDeclaratorRule;
    class CVQualifierSeqRule;
    class InitializerRule;
    class InitializerClauseRule;
    class InitializerListRule;
};

} } // namespace cmajor.code

#endif // Declarator_hpp_19449
