#ifndef XmlGrammar_hpp_12276
#define XmlGrammar_hpp_12276

#include <cmajor/parsing/Grammar.hpp>
#include <cmajor/parsing/Keyword.hpp>
#include <cmajor/xml/XmlProcessor.hpp>

namespace cmajor { namespace xml {

class XmlGrammar : public cmajor::parsing::Grammar
{
public:
    static XmlGrammar* Create();
    static XmlGrammar* Create(cmajor::parsing::ParsingDomain* parsingDomain);
    void Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, XmlProcessor* processor);
private:
    XmlGrammar(cmajor::parsing::ParsingDomain* parsingDomain_);
    virtual void CreateRules();
    virtual void GetReferencedGrammars();
    class DocumentRule;
    class NameRule;
    class EntityValueRule;
    class AttValueRule;
    class SystemLiteralRule;
    class PubidLiteralRule;
    class CharDataRule;
    class CommentRule;
    class PIRule;
    class PITargetRule;
    class CDSectRule;
    class CDataRule;
    class PrologRule;
    class XMLDeclRule;
    class VersionInfoRule;
    class MiscRule;
    class DocTypeDeclRule;
    class DeclSepRule;
    class IntSubsetRule;
    class MarkupDeclRule;
    class ExtSubsetRule;
    class ExtSubsetDeclRule;
    class SDDeclRule;
    class ElementRule;
    class AttributeRule;
    class ETagRule;
    class ContentRule;
    class ElementDeclRule;
    class AttlistDeclRule;
    class AttDefRule;
    class DefaultDeclRule;
    class ConditionalSectRule;
    class IncludeSectRule;
    class CharRefRule;
    class ReferenceRule;
    class EntityRefRule;
    class PEReferenceRule;
    class EntityDeclRule;
    class GEDeclRule;
    class PEDeclRule;
    class EntityDefRule;
    class PEDefRule;
    class TextDeclRule;
    class ExtParsedEntRule;
    class EncodingDeclRule;
    class EncNameRule;
    class NotationDeclRule;
};

} } // namespace cmajor.xml

#endif // XmlGrammar_hpp_12276
