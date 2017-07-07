#include "Solution.hpp"
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

namespace cmajor { namespace parser {

using namespace cmajor::parsing;
using namespace cmajor::util;
using namespace cmajor::unicode;

SolutionGrammar* SolutionGrammar::Create()
{
    return Create(new cmajor::parsing::ParsingDomain());
}

SolutionGrammar* SolutionGrammar::Create(cmajor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    SolutionGrammar* grammar(new SolutionGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

SolutionGrammar::SolutionGrammar(cmajor::parsing::ParsingDomain* parsingDomain_): cmajor::parsing::Grammar(ToUtf32("SolutionGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cmajor.parser")), parsingDomain_)
{
    SetOwner(0);
}

Solution* SolutionGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName)
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
    Solution* result = *static_cast<cmajor::parsing::ValueObject<Solution*>*>(value.get());
    stack.pop();
    return result;
}

class SolutionGrammar::SolutionRule : public cmajor::parsing::Rule
{
public:
    SolutionRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("Solution*"));
    }
    virtual void Enter(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<Solution*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<SolutionRule>(this, &SolutionRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<SolutionRule>(this, &SolutionRule::A1Action));
        cmajor::parsing::NonterminalParser* qualified_idNonterminalParser = GetNonterminal(ToUtf32("qualified_id"));
        qualified_idNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<SolutionRule>(this, &SolutionRule::Postqualified_id));
        cmajor::parsing::NonterminalParser* declarationNonterminalParser = GetNonterminal(ToUtf32("Declaration"));
        declarationNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<SolutionRule>(this, &SolutionRule::PostDeclaration));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new Solution(context->fromqualified_id, fileName);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->AddDeclaration(context->fromDeclaration);
    }
    void Postqualified_id(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromqualified_id_value = std::move(stack.top());
            context->fromqualified_id = *static_cast<cmajor::parsing::ValueObject<std::u32string>*>(fromqualified_id_value.get());
            stack.pop();
        }
    }
    void PostDeclaration(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromDeclaration_value = std::move(stack.top());
            context->fromDeclaration = *static_cast<cmajor::parsing::ValueObject<SolutionDeclaration*>*>(fromDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): value(), fromqualified_id(), fromDeclaration() {}
        Solution* value;
        std::u32string fromqualified_id;
        SolutionDeclaration* fromDeclaration;
    };
};

class SolutionGrammar::DeclarationRule : public cmajor::parsing::Rule
{
public:
    DeclarationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("SolutionDeclaration*"));
    }
    virtual void Enter(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<SolutionDeclaration*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<DeclarationRule>(this, &DeclarationRule::A0Action));
        cmajor::parsing::NonterminalParser* solutionProjectDeclarationNonterminalParser = GetNonterminal(ToUtf32("SolutionProjectDeclaration"));
        solutionProjectDeclarationNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<DeclarationRule>(this, &DeclarationRule::PostSolutionProjectDeclaration));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSolutionProjectDeclaration;
    }
    void PostSolutionProjectDeclaration(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromSolutionProjectDeclaration_value = std::move(stack.top());
            context->fromSolutionProjectDeclaration = *static_cast<cmajor::parsing::ValueObject<SolutionDeclaration*>*>(fromSolutionProjectDeclaration_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): value(), fromSolutionProjectDeclaration() {}
        SolutionDeclaration* value;
        SolutionDeclaration* fromSolutionProjectDeclaration;
    };
};

class SolutionGrammar::SolutionProjectDeclarationRule : public cmajor::parsing::Rule
{
public:
    SolutionProjectDeclarationRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("SolutionDeclaration*"));
    }
    virtual void Enter(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<SolutionDeclaration*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<SolutionProjectDeclarationRule>(this, &SolutionProjectDeclarationRule::A0Action));
        cmajor::parsing::NonterminalParser* filePathNonterminalParser = GetNonterminal(ToUtf32("FilePath"));
        filePathNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<SolutionProjectDeclarationRule>(this, &SolutionProjectDeclarationRule::PostFilePath));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SolutionProjectDeclaration(context->fromFilePath);
    }
    void PostFilePath(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromFilePath_value = std::move(stack.top());
            context->fromFilePath = *static_cast<cmajor::parsing::ValueObject<std::string>*>(fromFilePath_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): value(), fromFilePath() {}
        SolutionDeclaration* value;
        std::string fromFilePath;
    };
};

class SolutionGrammar::FilePathRule : public cmajor::parsing::Rule
{
public:
    FilePathRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::string"));
    }
    virtual void Enter(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData)
    {
        parsingData->PushContext(Id(), new Context());
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
    }
    virtual void Leave(cmajor::parsing::ObjectStack& stack, cmajor::parsing::ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<std::string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<FilePathRule>(this, &FilePathRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = std::string(matchBegin, matchEnd);
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): value() {}
        std::string value;
    };
};

void SolutionGrammar::GetReferencedGrammars()
{
    cmajor::parsing::ParsingDomain* pd = GetParsingDomain();
    cmajor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cmajor.parsing.stdlib"));
    if (!grammar0)
    {
        grammar0 = cmajor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
}

void SolutionGrammar::CreateRules()
{
    AddRuleLink(new cmajor::parsing::RuleLink(ToUtf32("qualified_id"), this, ToUtf32("cmajor.parsing.stdlib.qualified_id")));
    AddRuleLink(new cmajor::parsing::RuleLink(ToUtf32("spaces_and_comments"), this, ToUtf32("cmajor.parsing.stdlib.spaces_and_comments")));
    AddRule(new SolutionRule(ToUtf32("Solution"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::SequenceParser(
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::KeywordParser(ToUtf32("solution")),
                    new cmajor::parsing::NonterminalParser(ToUtf32("qualified_id"), ToUtf32("qualified_id"), 0)),
                new cmajor::parsing::ActionParser(ToUtf32("A0"),
                    new cmajor::parsing::CharParser(';'))),
            new cmajor::parsing::KleeneStarParser(
                new cmajor::parsing::ActionParser(ToUtf32("A1"),
                    new cmajor::parsing::NonterminalParser(ToUtf32("Declaration"), ToUtf32("Declaration"), 0))))));
    AddRule(new DeclarationRule(ToUtf32("Declaration"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::NonterminalParser(ToUtf32("SolutionProjectDeclaration"), ToUtf32("SolutionProjectDeclaration"), 0))));
    AddRule(new SolutionProjectDeclarationRule(ToUtf32("SolutionProjectDeclaration"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::KeywordParser(ToUtf32("project")),
                    new cmajor::parsing::NonterminalParser(ToUtf32("FilePath"), ToUtf32("FilePath"), 0)),
                new cmajor::parsing::CharParser(';')))));
    AddRule(new FilePathRule(ToUtf32("FilePath"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::TokenParser(
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::CharParser('<'),
                    new cmajor::parsing::ActionParser(ToUtf32("A0"),
                        new cmajor::parsing::PositiveParser(
                            new cmajor::parsing::CharSetParser(ToUtf32(">"), true)))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser('>'))))));
    SetSkipRuleName(ToUtf32("spaces_and_comments"));
}

} } // namespace cmajor.parser