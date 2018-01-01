#ifndef Concept_hpp_391
#define Concept_hpp_391

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/ast/Concept.hpp>
#include <cmajor/parser/ParsingContext.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::ast;
class ConceptGrammar : public cmajor::parsing::Grammar
{
public:
    static ConceptGrammar* Create();
    static ConceptGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    ConceptNode* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx);
private:
    ConceptGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ConceptRule;
    class RefinementRule;
    class ConceptBodyRule;
    class ConceptBodyConstraintRule;
    class TypeNameConstraintRule;
    class SignatureConstraintRule;
    class ConstructorConstraintRule;
    class DestructorConstraintRule;
    class MemberFunctionConstraintRule;
    class FunctionConstraintRule;
    class EmbeddedConstraintRule;
    class WhereConstraintRule;
    class ConstraintExprRule;
    class DisjunctiveConstraintExprRule;
    class ConjunctiveConstraintExprRule;
    class PrimaryConstraintExprRule;
    class AtomicConstraintExprRule;
    class PredicateConstraintRule;
    class IsConstraintRule;
    class ConceptOrTypeNameRule;
    class MultiParamConstraintRule;
    class AxiomRule;
    class AxiomBodyRule;
    class AxiomStatementRule;
};

} } // namespace cmajor.parser

#endif // Concept_hpp_391
