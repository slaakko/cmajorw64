#include "Template.hpp"
#include <cmajor/parsing/Action.hpp>
#include <cmajor/parsing/Rule.hpp>
#include <cmajor/parsing/ParsingDomain.hpp>
#include <cmajor/parsing/Primitive.hpp>
#include <cmajor/parsing/Composite.hpp>
#include <cmajor/parsing/Nonterminal.hpp>
#include <cmajor/parsing/Exception.hpp>
#include <cmajor/parsing/StdLib.hpp>
#include <cmajor/parsing/XmlLog.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/parser/Identifier.hpp>
#include <cmajor/parser/TypeExpr.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::parsing;
using namespace cmajor::util;
using namespace cmajor::unicode;

TemplateGrammar* TemplateGrammar::Create()
{
    return Create(new cmajor::parsing::ParsingDomain());
}

TemplateGrammar* TemplateGrammar::Create(cmajor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    TemplateGrammar* grammar(new TemplateGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

TemplateGrammar::TemplateGrammar(cmajor::parsing::ParsingDomain* parsingDomain_): cmajor::parsing::Grammar(ToUtf32("TemplateGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cmajor.parser")), parsingDomain_)
{
    SetOwner(0);
}

Node* TemplateGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
{
    cmajor::parsing::Scanner scanner(start, end, fileName, fileIndex, SkipRule());
    std::unique_ptr<cmajor::parsing::XmlLog> xmlLog;
    if (Log())
    {
        xmlLog.reset(new cmajor::parsing::XmlLog(*Log(), MaxLogLineLength()));
        scanner.SetLog(xmlLog.get());
        xmlLog->WriteBeginRule("parse");
    }
    cmajor::parsing::ObjectStack stack;
    std::unique_ptr<cmajor::parsing::ParsingData> parsingData(new cmajor::parsing::ParsingData(GetParsingDomain()->GetNumRules()));
    scanner.SetParsingData(parsingData.get());
    stack.push(std::unique_ptr<cmajor::parsing::Object>(new ValueObject<ParsingContext*>(ctx)));
    cmajor::parsing::Match match = cmajor::parsing::Grammar::Parse(scanner, stack, parsingData.get());
    cmajor::parsing::Span stop = scanner.GetSpan();
    if (Log())
    {
        xmlLog->WriteEndRule("parse");
    }
    if (!match.Hit() || stop.Start() != int(end - start))
    {
        if (StartRule())
        {
            throw cmajor::parsing::ExpectationFailure(StartRule()->Info(), fileName, stop, start, end);
        }
        else
        {
            throw cmajor::parsing::ParsingException("grammar '" + ToUtf8(Name()) + "' has no start rule", fileName, scanner.GetSpan(), start, end);
        }
    }
    std::unique_ptr<cmajor::parsing::Object> value = std::move(stack.top());
    Node* result = *static_cast<cmajor::parsing::ValueObject<Node*>*>(value.get());
    stack.pop();
    return result;
}

class TemplateGrammar::TemplateIdRule : public cmajor::parsing::Rule
{
public:
    TemplateIdRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("Node*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<TemplateIdNode>"), ToUtf32("templateId")));
    }
    virtual void Enter(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cmajor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cmajor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<Node*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<TemplateIdRule>(this, &TemplateIdRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<TemplateIdRule>(this, &TemplateIdRule::A1Action));
        a1ActionParser->SetFailureAction(new cmajor::parsing::MemberFailureAction<TemplateIdRule>(this, &TemplateIdRule::A1ActionFail));
        cmajor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<TemplateIdRule>(this, &TemplateIdRule::A2Action));
        cmajor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<TemplateIdRule>(this, &TemplateIdRule::A3Action));
        cmajor::parsing::NonterminalParser* primaryNonterminalParser = GetNonterminal(ToUtf32("primary"));
        primaryNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<TemplateIdRule>(this, &TemplateIdRule::Postprimary));
        cmajor::parsing::NonterminalParser* templateArgNonterminalParser = GetNonterminal(ToUtf32("templateArg"));
        templateArgNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<TemplateIdRule>(this, &TemplateIdRule::PretemplateArg));
        templateArgNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<TemplateIdRule>(this, &TemplateIdRule::PosttemplateArg));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctx->BeginParsingTemplateId();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctx->EndParsingTemplateId();
        context->value = context->templateId.release();
        context->value->GetSpan().SetEnd(span.End());
    }
    void A1ActionFail(ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctx->EndParsingTemplateId();
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->templateId.reset(new TemplateIdNode(span, context->fromprimary));
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->templateId->AddTemplateArgument(context->fromtemplateArg);
    }
    void Postprimary(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromprimary_value = std::move(stack.top());
            context->fromprimary = *static_cast<cmajor::parsing::ValueObject<IdentifierNode*>*>(fromprimary_value.get());
            stack.pop();
        }
    }
    void PretemplateArg(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PosttemplateArg(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromtemplateArg_value = std::move(stack.top());
            context->fromtemplateArg = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromtemplateArg_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), templateId(), fromprimary(), fromtemplateArg() {}
        ParsingContext* ctx;
        Node* value;
        std::unique_ptr<TemplateIdNode> templateId;
        IdentifierNode* fromprimary;
        Node* fromtemplateArg;
    };
};

class TemplateGrammar::TemplateParameterRule : public cmajor::parsing::Rule
{
public:
    TemplateParameterRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("TemplateParameterNode*"));
    }
    virtual void Enter(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cmajor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cmajor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<TemplateParameterNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<TemplateParameterRule>(this, &TemplateParameterRule::A0Action));
        cmajor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<TemplateParameterRule>(this, &TemplateParameterRule::PostIdentifier));
        cmajor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal(ToUtf32("TypeExpr"));
        typeExprNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<TemplateParameterRule>(this, &TemplateParameterRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<TemplateParameterRule>(this, &TemplateParameterRule::PostTypeExpr));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new TemplateParameterNode(span, context->fromIdentifier, context->fromTypeExpr);
    }
    void PostIdentifier(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromIdentifier_value = std::move(stack.top());
            context->fromIdentifier = *static_cast<cmajor::parsing::ValueObject<IdentifierNode*>*>(fromIdentifier_value.get());
            stack.pop();
        }
    }
    void PreTypeExpr(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostTypeExpr(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromTypeExpr_value = std::move(stack.top());
            context->fromTypeExpr = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromTypeExpr_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromIdentifier(), fromTypeExpr() {}
        ParsingContext* ctx;
        TemplateParameterNode* value;
        IdentifierNode* fromIdentifier;
        Node* fromTypeExpr;
    };
};

class TemplateGrammar::TemplateParameterListRule : public cmajor::parsing::Rule
{
public:
    TemplateParameterListRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        AddInheritedAttribute(AttrOrVariable(ToUtf32("Node*"), ToUtf32("owner")));
    }
    virtual void Enter(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        std::unique_ptr<cmajor::parsing::Object> owner_value = std::move(stack.top());
        context->owner = *static_cast<cmajor::parsing::ValueObject<Node*>*>(owner_value.get());
        stack.pop();
        std::unique_ptr<cmajor::parsing::Object> ctx_value = std::move(stack.top());
        context->ctx = *static_cast<cmajor::parsing::ValueObject<ParsingContext*>*>(ctx_value.get());
        stack.pop();
    }
    virtual void Leave(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData, bool matched)
    {
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<TemplateParameterListRule>(this, &TemplateParameterListRule::A0Action));
        cmajor::parsing::NonterminalParser* templateParameterNonterminalParser = GetNonterminal(ToUtf32("TemplateParameter"));
        templateParameterNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<TemplateParameterListRule>(this, &TemplateParameterListRule::PreTemplateParameter));
        templateParameterNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<TemplateParameterListRule>(this, &TemplateParameterListRule::PostTemplateParameter));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->owner->AddTemplateParameter(context->fromTemplateParameter);
    }
    void PreTemplateParameter(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostTemplateParameter(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromTemplateParameter_value = std::move(stack.top());
            context->fromTemplateParameter = *static_cast<cmajor::parsing::ValueObject<TemplateParameterNode*>*>(fromTemplateParameter_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), owner(), fromTemplateParameter() {}
        ParsingContext* ctx;
        Node* owner;
        TemplateParameterNode* fromTemplateParameter;
    };
};

void TemplateGrammar::GetReferencedGrammars()
{
    cmajor::parsing::ParsingDomain* pd = GetParsingDomain();
    cmajor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cmajor.parser.IdentifierGrammar"));
    if (!grammar0)
    {
        grammar0 = cmajor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar0);
    cmajor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cmajor.parser.TypeExprGrammar"));
    if (!grammar1)
    {
        grammar1 = cmajor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
}

void TemplateGrammar::CreateRules()
{
    AddRuleLink(new cmajor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("IdentifierGrammar.Identifier")));
    AddRuleLink(new cmajor::parsing::RuleLink(ToUtf32("QualifiedId"), this, ToUtf32("IdentifierGrammar.QualifiedId")));
    AddRuleLink(new cmajor::parsing::RuleLink(ToUtf32("TypeExpr"), this, ToUtf32("TypeExprGrammar.TypeExpr")));
    AddRule(new TemplateIdRule(ToUtf32("TemplateId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::SequenceParser(
            new cmajor::parsing::ActionParser(ToUtf32("A0"),
                new cmajor::parsing::EmptyParser()),
            new cmajor::parsing::ActionParser(ToUtf32("A1"),
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::SequenceParser(
                            new cmajor::parsing::ActionParser(ToUtf32("A2"),
                                new cmajor::parsing::NonterminalParser(ToUtf32("primary"), ToUtf32("QualifiedId"), 0)),
                            new cmajor::parsing::CharParser('<')),
                        new cmajor::parsing::ListParser(
                            new cmajor::parsing::ActionParser(ToUtf32("A3"),
                                new cmajor::parsing::NonterminalParser(ToUtf32("templateArg"), ToUtf32("TypeExpr"), 1)),
                            new cmajor::parsing::CharParser(','))),
                    new cmajor::parsing::CharParser('>'))))));
    AddRule(new TemplateParameterRule(ToUtf32("TemplateParameter"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0),
                new cmajor::parsing::OptionalParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::CharParser('='),
                        new cmajor::parsing::NonterminalParser(ToUtf32("TypeExpr"), ToUtf32("TypeExpr"), 1)))))));
    AddRule(new TemplateParameterListRule(ToUtf32("TemplateParameterList"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::SequenceParser(
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::CharParser('<'),
                new cmajor::parsing::ListParser(
                    new cmajor::parsing::ActionParser(ToUtf32("A0"),
                        new cmajor::parsing::NonterminalParser(ToUtf32("TemplateParameter"), ToUtf32("TemplateParameter"), 1)),
                    new cmajor::parsing::CharParser(','))),
            new cmajor::parsing::ExpectationParser(
                new cmajor::parsing::CharParser('>')))));
}

} } // namespace cmajor.parser
