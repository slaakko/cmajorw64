#ifndef Expression_hpp_19453
#define Expression_hpp_19453

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/codedom/Expression.hpp>

namespace cmajor { namespace code {

class ExpressionGrammar : public cmajor::parsing::Grammar
{
public:
    static ExpressionGrammar* Create();
    static ExpressionGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    cmajor::codedom::CppObject* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    ExpressionGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class ExpressionRule;
    class ConstantExpressionRule;
    class AssignmentExpressionRule;
    class AssingmentOpRule;
    class ThrowExpressionRule;
    class ConditionalExpressionRule;
    class LogicalOrExpressionRule;
    class LogicalAndExpressionRule;
    class InclusiveOrExpressionRule;
    class ExclusiveOrExpressionRule;
    class AndExpressionRule;
    class EqualityExpressionRule;
    class EqOpRule;
    class RelationalExpressionRule;
    class RelOpRule;
    class ShiftExpressionRule;
    class ShiftOpRule;
    class AdditiveExpressionRule;
    class AddOpRule;
    class MultiplicativeExpressionRule;
    class MulOpRule;
    class PmExpressionRule;
    class PmOpRule;
    class CastExpressionRule;
    class UnaryExpressionRule;
    class UnaryOperatorRule;
    class NewExpressionRule;
    class NewTypeIdRule;
    class NewDeclaratorRule;
    class DirectNewDeclaratorRule;
    class NewPlacementRule;
    class NewInitializerRule;
    class DeleteExpressionRule;
    class PostfixExpressionRule;
    class TypeSpecifierOrTypeNameRule;
    class PostCastExpressionRule;
    class ExpressionListRule;
    class PrimaryExpressionRule;
    class IdExpressionRule;
};

} } // namespace cmajor.code

#endif // Expression_hpp_19453
