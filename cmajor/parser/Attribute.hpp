#ifndef Attribute_hpp_12774
#define Attribute_hpp_12774

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Attribute.hpp>

namespace cmajor { namespace parser {

class AttributeGrammar : public cmajor::parsing::Grammar
{
public:
    static AttributeGrammar* Create();
    static AttributeGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    cmajor::ast::Attributes* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    AttributeGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class AttributesRule;
    class AttributeRule;
};

} } // namespace cmajor.parser

#endif // Attribute_hpp_12774
