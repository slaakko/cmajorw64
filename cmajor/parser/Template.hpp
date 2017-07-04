#ifndef Template_hpp_18661
#define Template_hpp_18661

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Template.hpp>
#include <cmajor/parser/ParsingContext.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class TemplateGrammar : public cmajor::parsing::Grammar
{
public:
    static TemplateGrammar* Create();
    static TemplateGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    TemplateGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class TemplateIdRule;
    class TemplateParameterRule;
    class TemplateParameterListRule;
};

} } // namespace cmajor.parser

#endif // Template_hpp_18661
