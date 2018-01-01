#ifndef Constant_hpp_391
#define Constant_hpp_391

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Constant.hpp>
#include <cmajor/parser/ParsingContext.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class ConstantGrammar : public cmajor::parsing::Grammar
{
public:
    static ConstantGrammar* Create();
    static ConstantGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    ConstantNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    ConstantGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ConstantRule;
};

} } // namespace cmajor.parser

#endif // Constant_hpp_391
