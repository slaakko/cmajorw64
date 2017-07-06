#ifndef Interface_hpp_28781
#define Interface_hpp_28781

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Interface.hpp>
#include <cmajor/parser/ParsingContext.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class InterfaceGrammar : public cmajor::parsing::Grammar
{
public:
    static InterfaceGrammar* Create();
    static InterfaceGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    InterfaceNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    InterfaceGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class InterfaceRule;
    class InterfaceContentRule;
    class InterfaceMemFunRule;
    class InterfaceFunctionGroupIdRule;
};

} } // namespace cmajor.parser

#endif // Interface_hpp_28781
