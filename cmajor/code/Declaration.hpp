#ifndef Declaration_hpp_19449
#define Declaration_hpp_19449

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/codedom/Declaration.hpp>

namespace cmajor { namespace code {

class DeclarationGrammar : public cmajor::parsing::Grammar
{
public:
    static DeclarationGrammar* Create();
    static DeclarationGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    cmajor::codedom::CppObject* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    std::vector<std::u32string> keywords0;
    std::vector<std::u32string> keywords1;
    DeclarationGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class BlockDeclarationRule;
    class SimpleDeclarationRule;
    class DeclSpecifierSeqRule;
    class DeclSpecifierRule;
    class StorageClassSpecifierRule;
    class TypeSpecifierRule;
    class SimpleTypeSpecifierRule;
    class TypeNameRule;
    class TemplateArgumentListRule;
    class TemplateArgumentRule;
    class TypedefRule;
    class CVQualifierRule;
    class NamespaceAliasDefinitionRule;
    class UsingDeclarationRule;
    class UsingDirectiveRule;
};

} } // namespace cmajor.code

#endif // Declaration_hpp_19449
