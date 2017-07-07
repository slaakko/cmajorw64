#ifndef Function_hpp_21397
#define Function_hpp_21397

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Function.hpp>
#include <cmajor/parser/ParsingContext.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class FunctionGrammar : public cmajor::parsing::Grammar
{
public:
    static FunctionGrammar* Create();
    static FunctionGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    FunctionNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    FunctionGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class FunctionRule;
    class FunctionGroupIdRule;
    class OperatorFunctionGroupIdRule;
};

} } // namespace cmajor.parser

#endif // Function_hpp_21397
