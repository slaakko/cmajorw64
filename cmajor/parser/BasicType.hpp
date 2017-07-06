#ifndef BasicType_hpp_28778
#define BasicType_hpp_28778

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/BasicType.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class BasicTypeGrammar : public cmajor::parsing::Grammar
{
public:
    static BasicTypeGrammar* Create();
    static BasicTypeGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    Node* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    BasicTypeGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class BasicTypeRule;
};

} } // namespace cmajor.parser

#endif // BasicType_hpp_28778
