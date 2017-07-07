#ifndef Parameter_hpp_21397
#define Parameter_hpp_21397

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Parameter.hpp>
#include <cmajor/parser/ParsingContext.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class ParameterGrammar : public cmajor::parsing::Grammar
{
public:
    static ParameterGrammar* Create();
    static ParameterGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx, Node* owner);
private:
    ParameterGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ParameterListRule;
    class ParameterRule;
};

} } // namespace cmajor.parser

#endif // Parameter_hpp_21397
