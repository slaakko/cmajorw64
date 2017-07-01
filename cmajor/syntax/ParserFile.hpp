#ifndef ParserFile_hpp_20364
#define ParserFile_hpp_20364

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/syntax/ParserFileContent.hpp>

namespace cmajor { namespace syntax {

class ParserFileGrammar : public cmajor::parsing::Grammar
{
public:
    static ParserFileGrammar* Create();
    static ParserFileGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    ParserFileContent* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, int id_, cmajor::parsing::ParsingDomain* parsingDomain_);
private:
    ParserFileGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ParserFileRule;
    class IncludeDirectivesRule;
    class IncludeDirectiveRule;
    class FileAttributeRule;
    class IncludeFileNameRule;
    class NamespaceContentRule;
    class NamespaceRule;
};

} } // namespace cmajor.syntax

#endif // ParserFile_hpp_20364
