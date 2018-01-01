#ifndef CommandLine_hpp_16265
#define CommandLine_hpp_16265

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>

namespace cmajor { namespace parser {

class CommandLineGrammar : public cmajor::parsing::Grammar
{
public:
    static CommandLineGrammar* Create();
    static CommandLineGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    std::vector<std::string> Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    CommandLineGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class CommandLineRule;
    class ArgumentRule;
    class ArgElementRule;
    class OddBackslashesAndLiteralQuotationMarkRule;
    class EvenBackslashesAndQuotationMarkRule;
    class StringCharRule;
};

} } // namespace cmajor.parser

#endif // CommandLine_hpp_16265
