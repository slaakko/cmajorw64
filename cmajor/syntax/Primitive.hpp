#ifndef Primitive_hpp_20364
#define Primitive_hpp_20364

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/parsing/Primitive.hpp>
#include <cmajor/parsing/Keyword.hpp>

namespace cmajor { namespace syntax {

class PrimitiveGrammar : public cmajor::parsing::Grammar
{
public:
    static PrimitiveGrammar* Create();
    static PrimitiveGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    cmajor::parsing::Parser* Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName);
private:
    PrimitiveGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class PrimitiveRule;
    class CharRule;
    class StringRule;
    class CharSetRule;
    class CharSetRangeRule;
    class CharSetCharRule;
    class KeywordRule;
    class KeywordBodyRule;
    class KeywordListRule;
    class KeywordListBodyRule;
    class RangeRule;
    class CodePointRule;
    class EmptyRule;
    class SpaceRule;
    class AnyCharRule;
    class LetterRule;
    class DigitRule;
    class HexDigitRule;
    class UpperLetterRule;
    class LowerLetterRule;
    class TitleLetterRule;
    class ModifierLetterRule;
    class OtherLetterRule;
    class CasedLetterRule;
    class MarkRule;
    class NonspacingMarkRule;
    class SpacingMarkRule;
    class EnclosingMarkRule;
    class NumberRule;
    class DecimalNumberRule;
    class LetterNumberRule;
    class OtherNumberRule;
    class PunctuationRule;
    class ConnectorPunctuationRule;
    class DashPunctuationRule;
    class OpenPunctuationRule;
    class ClosePunctuationRule;
    class InitialPunctuationRule;
    class FinalPunctuationRule;
    class OtherPunctuationRule;
    class SymbolRule;
    class MathSymbolRule;
    class CurrencySymbolRule;
    class ModifierSymbolRule;
    class OtherSymbolRule;
    class SeparatorRule;
    class SpaceSeparatorRule;
    class LineSeparatorRule;
    class ParagraphSeparatorRule;
    class OtherRule;
    class ControlRule;
    class FormatRule;
    class SurrogateRule;
    class PrivateUseRule;
    class UnassignedRule;
    class GraphicRule;
    class BaseCharRule;
    class AlphabeticRule;
    class IdStartRule;
    class IdContRule;
};

} } // namespace cmajor.syntax

#endif // Primitive_hpp_20364
