#ifndef Solution_hpp_391
#define Solution_hpp_391

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Solution.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class SolutionGrammar : public cmajor::parsing::Grammar
{
public:
    static SolutionGrammar* Create();
    static SolutionGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    Solution* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    SolutionGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class SolutionRule;
    class DeclarationRule;
    class SolutionProjectDeclarationRule;
    class ActiveProjectDeclarationRule;
    class FilePathRule;
};

} } // namespace cmajor.parser

#endif // Solution_hpp_391
