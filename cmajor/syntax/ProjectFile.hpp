#ifndef ProjectFile_hpp_20364
#define ProjectFile_hpp_20364

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/syntax/Project.hpp>

namespace cmajor { namespace syntax {

class ProjectFileGrammar : public cmajor::parsing::Grammar
{
public:
    static ProjectFileGrammar* Create();
    static ProjectFileGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    Project* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    ProjectFileGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ProjectFileRule;
    class ProjectFileContentRule;
    class SourceRule;
    class ReferenceRule;
    class FilePathRule;
};

} } // namespace cmajor.syntax

#endif // ProjectFile_hpp_20364
