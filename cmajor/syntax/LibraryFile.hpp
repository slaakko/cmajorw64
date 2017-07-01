#ifndef LibraryFile_hpp_20364
#define LibraryFile_hpp_20364

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/parsing/ParsingDomain.hpp>

namespace cmajor { namespace syntax {

class LibraryFileGrammar : public cmajor::parsing::Grammar
{
public:
    static LibraryFileGrammar* Create();
    static LibraryFileGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, cmajor::parsing::ParsingDomain* parsingDomain);
private:
    LibraryFileGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class LibraryFileRule;
    class NamespaceContentRule;
    class NamespaceRule;
    class GrammarRule;
    class GrammarContentRule;
    class RuleRule;
};

} } // namespace cmajor.syntax

#endif // LibraryFile_hpp_20364
