#ifndef Json_hpp_391
#define Json_hpp_391

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/util/Json.hpp>
#include <sstream>

namespace cmajor { namespace parser {

using namespace cmajor::util;
class JsonGrammar : public cmajor::parsing::Grammar
{
public:
    static JsonGrammar* Create();
    static JsonGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    JsonValue* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    JsonGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ValueRule;
    class StringRule;
    class NumberRule;
    class ObjectRule;
    class ArrayRule;
};

} } // namespace cmajor.parser

#endif // Json_hpp_391
