#ifndef Project_hpp_17124
#define Project_hpp_17124

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Project.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class ProjectGrammar : public cmajor::parsing::Grammar
{
public:
    static ProjectGrammar* Create();
    static ProjectGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    Project* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, std::string config);
private:
    ProjectGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ProjectRule;
    class DeclarationRule;
    class ReferenceDeclarationRule;
    class SourceFileDeclarationRule;
    class TextFileDeclarationRule;
    class TargetDeclarationRule;
    class TargetRule;
    class FilePathRule;
};

} } // namespace cmajor.parser

#endif // Project_hpp_17124
