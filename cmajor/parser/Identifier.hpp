#ifndef Identifier_hpp_28781
#define Identifier_hpp_28781

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Identifier.hpp>

namespace cmajor { namespace parser {

using cmajor::ast::IdentifierNode;
class IdentifierGrammar : public cmajor::parsing::Grammar
{
public:
    static IdentifierGrammar* Create();
    static IdentifierGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    IdentifierNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    IdentifierGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class IdentifierRule;
    class QualifiedIdRule;
};

} } // namespace cmajor.parser

#endif // Identifier_hpp_28781
