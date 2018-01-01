#ifndef SourceToken_hpp_391
#define SourceToken_hpp_391

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/SourceToken.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class SourceTokenGrammar : public cmajor::parsing::Grammar
{
public:
    static SourceTokenGrammar* Create();
    static SourceTokenGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, SourceTokenFormatter* formatter);
private:
    SourceTokenGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class SourceTokensRule;
    class SourceTokenRule;
    class SpacesRule;
    class OtherRule;
};

} } // namespace cmajor.parser

#endif // SourceToken_hpp_391
