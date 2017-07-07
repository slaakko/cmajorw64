#ifndef Delegate_hpp_21397
#define Delegate_hpp_21397

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Delegate.hpp>
#include <cmajor/parser/ParsingContext.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class DelegateGrammar : public cmajor::parsing::Grammar
{
public:
    static DelegateGrammar* Create();
    static DelegateGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    DelegateNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    DelegateGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class DelegateRule;
    class ClassDelegateRule;
};

} } // namespace cmajor.parser

#endif // Delegate_hpp_21397
