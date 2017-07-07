#ifndef Typedef_hpp_21397
#define Typedef_hpp_21397

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Typedef.hpp>
#include <cmajor/parser/ParsingContext.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class TypedefGrammar : public cmajor::parsing::Grammar
{
public:
    static TypedefGrammar* Create();
    static TypedefGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    TypedefNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    TypedefGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class TypedefRule;
};

} } // namespace cmajor.parser

#endif // Typedef_hpp_21397
