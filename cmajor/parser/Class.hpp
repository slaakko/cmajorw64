#ifndef Class_hpp_391
#define Class_hpp_391

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Class.hpp>
#include <cmajor/parser/ParsingContext.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class ClassGrammar : public cmajor::parsing::Grammar
{
public:
    static ClassGrammar* Create();
    static ClassGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    ClassNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    ClassGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ClassRule;
    class InheritanceAndInterfacesRule;
    class BaseClassOrInterfaceRule;
    class ClassContentRule;
    class ClassMemberRule;
    class StaticConstructorRule;
    class ConstructorRule;
    class DestructorRule;
    class InitializerRule;
    class MemberFunctionRule;
    class ConversionFunctionRule;
    class MemberVariableRule;
};

} } // namespace cmajor.parser

#endif // Class_hpp_391
