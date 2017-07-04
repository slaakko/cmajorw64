#ifndef TypeExpr_hpp_18661
#define TypeExpr_hpp_18661

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/TypeExpr.hpp>
#include <cmajor/parser/ParsingContext.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class TypeExprGrammar : public cmajor::parsing::Grammar
{
public:
    static TypeExprGrammar* Create();
    static TypeExprGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    TypeExprGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class TypeExprRule;
    class PrefixTypeExprRule;
    class PostfixTypeExprRule;
    class PrimaryTypeExprRule;
};

} } // namespace cmajor.parser

#endif // TypeExpr_hpp_18661
