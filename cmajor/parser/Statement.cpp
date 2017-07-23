#include "Statement.hpp"
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
#include <cmajor/parser/Keyword.hpp>
#include <cmajor/parser/Expression.hpp>
#include <cmajor/parser/TypeExpr.hpp>
#include <cmajor/parser/Identifier.hpp>

namespace cmajor { namespace parser {

using namespace cmajor::parsing;
using namespace cmajor::util;
using namespace cmajor::unicode;

StatementGrammar* StatementGrammar::Create()
{
    return Create(new cmajor::parsing::ParsingDomain());
}

StatementGrammar* StatementGrammar::Create(cmajor::parsing::ParsingDomain* parsingDomain)
{
    RegisterParsingDomain(parsingDomain);
    StatementGrammar* grammar(new StatementGrammar(parsingDomain));
    parsingDomain->AddGrammar(grammar);
    grammar->CreateRules();
    grammar->Link();
    return grammar;
}

StatementGrammar::StatementGrammar(cmajor::parsing::ParsingDomain* parsingDomain_): cmajor::parsing::Grammar(ToUtf32("StatementGrammar"), parsingDomain_->GetNamespaceScope(ToUtf32("cmajor.parser")), parsingDomain_)
{
    SetOwner(0);
}

StatementNode* StatementGrammar::Parse(const char32_t* start, const char32_t* end, int fileIndex, const std::string& fileName, ParsingContext* ctx)
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
    StatementNode* result = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(value.get());
    stack.pop();
    return result;
}

class StatementGrammar::StatementRule : public cmajor::parsing::Rule
{
public:
    StatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A1Action));
        cmajor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A2Action));
        cmajor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A3Action));
        cmajor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A4Action));
        cmajor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A5Action));
        cmajor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A6Action));
        cmajor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A7Action));
        cmajor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A8Action));
        cmajor::parsing::ActionParser* a9ActionParser = GetAction(ToUtf32("A9"));
        a9ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A9Action));
        cmajor::parsing::ActionParser* a10ActionParser = GetAction(ToUtf32("A10"));
        a10ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<StatementRule>(this, &StatementRule::A10Action));
        cmajor::parsing::NonterminalParser* labeledStatementNonterminalParser = GetNonterminal(ToUtf32("LabeledStatement"));
        labeledStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreLabeledStatement));
        labeledStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostLabeledStatement));
        cmajor::parsing::NonterminalParser* controlStatementNonterminalParser = GetNonterminal(ToUtf32("ControlStatement"));
        controlStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreControlStatement));
        controlStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostControlStatement));
        cmajor::parsing::NonterminalParser* expressionStatementNonterminalParser = GetNonterminal(ToUtf32("ExpressionStatement"));
        expressionStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreExpressionStatement));
        expressionStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostExpressionStatement));
        cmajor::parsing::NonterminalParser* assignmentStatementNonterminalParser = GetNonterminal(ToUtf32("AssignmentStatement"));
        assignmentStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreAssignmentStatement));
        assignmentStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostAssignmentStatement));
        cmajor::parsing::NonterminalParser* constructionStatementNonterminalParser = GetNonterminal(ToUtf32("ConstructionStatement"));
        constructionStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreConstructionStatement));
        constructionStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostConstructionStatement));
        cmajor::parsing::NonterminalParser* deleteStatementNonterminalParser = GetNonterminal(ToUtf32("DeleteStatement"));
        deleteStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreDeleteStatement));
        deleteStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostDeleteStatement));
        cmajor::parsing::NonterminalParser* destroyStatementNonterminalParser = GetNonterminal(ToUtf32("DestroyStatement"));
        destroyStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreDestroyStatement));
        destroyStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostDestroyStatement));
        cmajor::parsing::NonterminalParser* emptyStatementNonterminalParser = GetNonterminal(ToUtf32("EmptyStatement"));
        emptyStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreEmptyStatement));
        emptyStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostEmptyStatement));
        cmajor::parsing::NonterminalParser* throwStatementNonterminalParser = GetNonterminal(ToUtf32("ThrowStatement"));
        throwStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreThrowStatement));
        throwStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostThrowStatement));
        cmajor::parsing::NonterminalParser* tryStatementNonterminalParser = GetNonterminal(ToUtf32("TryStatement"));
        tryStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreTryStatement));
        tryStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostTryStatement));
        cmajor::parsing::NonterminalParser* assertStatementNonterminalParser = GetNonterminal(ToUtf32("AssertStatement"));
        assertStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<StatementRule>(this, &StatementRule::PreAssertStatement));
        assertStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<StatementRule>(this, &StatementRule::PostAssertStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromLabeledStatement;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromControlStatement;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromExpressionStatement;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromAssignmentStatement;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromConstructionStatement;
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDeleteStatement;
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDestroyStatement;
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEmptyStatement;
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromThrowStatement;
    }
    void A9Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromTryStatement;
    }
    void A10Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromAssertStatement;
    }
    void PreLabeledStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostLabeledStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromLabeledStatement_value = std::move(stack.top());
            context->fromLabeledStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromLabeledStatement_value.get());
            stack.pop();
        }
    }
    void PreControlStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostControlStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromControlStatement_value = std::move(stack.top());
            context->fromControlStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromControlStatement_value.get());
            stack.pop();
        }
    }
    void PreExpressionStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpressionStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpressionStatement_value = std::move(stack.top());
            context->fromExpressionStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromExpressionStatement_value.get());
            stack.pop();
        }
    }
    void PreAssignmentStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostAssignmentStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromAssignmentStatement_value = std::move(stack.top());
            context->fromAssignmentStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromAssignmentStatement_value.get());
            stack.pop();
        }
    }
    void PreConstructionStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostConstructionStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromConstructionStatement_value = std::move(stack.top());
            context->fromConstructionStatement = *static_cast<cmajor::parsing::ValueObject<ConstructionStatementNode*>*>(fromConstructionStatement_value.get());
            stack.pop();
        }
    }
    void PreDeleteStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostDeleteStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromDeleteStatement_value = std::move(stack.top());
            context->fromDeleteStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromDeleteStatement_value.get());
            stack.pop();
        }
    }
    void PreDestroyStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostDestroyStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromDestroyStatement_value = std::move(stack.top());
            context->fromDestroyStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromDestroyStatement_value.get());
            stack.pop();
        }
    }
    void PreEmptyStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostEmptyStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromEmptyStatement_value = std::move(stack.top());
            context->fromEmptyStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromEmptyStatement_value.get());
            stack.pop();
        }
    }
    void PreThrowStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostThrowStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromThrowStatement_value = std::move(stack.top());
            context->fromThrowStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromThrowStatement_value.get());
            stack.pop();
        }
    }
    void PreTryStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostTryStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromTryStatement_value = std::move(stack.top());
            context->fromTryStatement = *static_cast<cmajor::parsing::ValueObject<TryStatementNode*>*>(fromTryStatement_value.get());
            stack.pop();
        }
    }
    void PreAssertStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostAssertStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromAssertStatement_value = std::move(stack.top());
            context->fromAssertStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromAssertStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromLabeledStatement(), fromControlStatement(), fromExpressionStatement(), fromAssignmentStatement(), fromConstructionStatement(), fromDeleteStatement(), fromDestroyStatement(), fromEmptyStatement(), fromThrowStatement(), fromTryStatement(), fromAssertStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromLabeledStatement;
        StatementNode* fromControlStatement;
        StatementNode* fromExpressionStatement;
        StatementNode* fromAssignmentStatement;
        ConstructionStatementNode* fromConstructionStatement;
        StatementNode* fromDeleteStatement;
        StatementNode* fromDestroyStatement;
        StatementNode* fromEmptyStatement;
        StatementNode* fromThrowStatement;
        TryStatementNode* fromTryStatement;
        StatementNode* fromAssertStatement;
    };
};

class StatementGrammar::LabelIdRule : public cmajor::parsing::Rule
{
public:
    LabelIdRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("std::u32string"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<std::u32string>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<LabelIdRule>(this, &LabelIdRule::A0Action));
        cmajor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("identifier"));
        identifierNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<LabelIdRule>(this, &LabelIdRule::Postidentifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromidentifier;
    }
    void Postidentifier(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromidentifier_value = std::move(stack.top());
            context->fromidentifier = *static_cast<cmajor::parsing::ValueObject<std::u32string>*>(fromidentifier_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): value(), fromidentifier() {}
        std::u32string value;
        std::u32string fromidentifier;
    };
};

class StatementGrammar::LabelRule : public cmajor::parsing::Rule
{
public:
    LabelRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        SetValueTypeName(ToUtf32("LabelNode*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::u32string"), ToUtf32("label")));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<LabelNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<LabelRule>(this, &LabelRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<LabelRule>(this, &LabelRule::A1Action));
        cmajor::parsing::NonterminalParser* labelIdNonterminalParser = GetNonterminal(ToUtf32("LabelId"));
        labelIdNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<LabelRule>(this, &LabelRule::PostLabelId));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new LabelNode(span, context->label);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->label = context->fromLabelId;
    }
    void PostLabelId(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromLabelId_value = std::move(stack.top());
            context->fromLabelId = *static_cast<cmajor::parsing::ValueObject<std::u32string>*>(fromLabelId_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): value(), label(), fromLabelId() {}
        LabelNode* value;
        std::u32string label;
        std::u32string fromLabelId;
    };
};

class StatementGrammar::LabeledStatementRule : public cmajor::parsing::Rule
{
public:
    LabeledStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<LabeledStatementRule>(this, &LabeledStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* lblNonterminalParser = GetNonterminal(ToUtf32("lbl"));
        lblNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Postlbl));
        cmajor::parsing::NonterminalParser* stmtNonterminalParser = GetNonterminal(ToUtf32("stmt"));
        stmtNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<LabeledStatementRule>(this, &LabeledStatementRule::Prestmt));
        stmtNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<LabeledStatementRule>(this, &LabeledStatementRule::Poststmt));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->fromstmt->SetLabelNode(context->fromlbl);
        context->value = context->fromstmt;
    }
    void Postlbl(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromlbl_value = std::move(stack.top());
            context->fromlbl = *static_cast<cmajor::parsing::ValueObject<LabelNode*>*>(fromlbl_value.get());
            stack.pop();
        }
    }
    void Prestmt(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void Poststmt(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromstmt_value = std::move(stack.top());
            context->fromstmt = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromstmt_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromlbl(), fromstmt() {}
        ParsingContext* ctx;
        StatementNode* value;
        LabelNode* fromlbl;
        StatementNode* fromstmt;
    };
};

class StatementGrammar::ControlStatementRule : public cmajor::parsing::Rule
{
public:
    ControlStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A1Action));
        cmajor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A2Action));
        cmajor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A3Action));
        cmajor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A4Action));
        cmajor::parsing::ActionParser* a5ActionParser = GetAction(ToUtf32("A5"));
        a5ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A5Action));
        cmajor::parsing::ActionParser* a6ActionParser = GetAction(ToUtf32("A6"));
        a6ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A6Action));
        cmajor::parsing::ActionParser* a7ActionParser = GetAction(ToUtf32("A7"));
        a7ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A7Action));
        cmajor::parsing::ActionParser* a8ActionParser = GetAction(ToUtf32("A8"));
        a8ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A8Action));
        cmajor::parsing::ActionParser* a9ActionParser = GetAction(ToUtf32("A9"));
        a9ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A9Action));
        cmajor::parsing::ActionParser* a10ActionParser = GetAction(ToUtf32("A10"));
        a10ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A10Action));
        cmajor::parsing::ActionParser* a11ActionParser = GetAction(ToUtf32("A11"));
        a11ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A11Action));
        cmajor::parsing::ActionParser* a12ActionParser = GetAction(ToUtf32("A12"));
        a12ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ControlStatementRule>(this, &ControlStatementRule::A12Action));
        cmajor::parsing::NonterminalParser* compoundStatementNonterminalParser = GetNonterminal(ToUtf32("CompoundStatement"));
        compoundStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreCompoundStatement));
        compoundStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostCompoundStatement));
        cmajor::parsing::NonterminalParser* returnStatementNonterminalParser = GetNonterminal(ToUtf32("ReturnStatement"));
        returnStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreReturnStatement));
        returnStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostReturnStatement));
        cmajor::parsing::NonterminalParser* ifStatementNonterminalParser = GetNonterminal(ToUtf32("IfStatement"));
        ifStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreIfStatement));
        ifStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostIfStatement));
        cmajor::parsing::NonterminalParser* whileStatementNonterminalParser = GetNonterminal(ToUtf32("WhileStatement"));
        whileStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreWhileStatement));
        whileStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostWhileStatement));
        cmajor::parsing::NonterminalParser* doStatementNonterminalParser = GetNonterminal(ToUtf32("DoStatement"));
        doStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreDoStatement));
        doStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostDoStatement));
        cmajor::parsing::NonterminalParser* rangeForStatementNonterminalParser = GetNonterminal(ToUtf32("RangeForStatement"));
        rangeForStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreRangeForStatement));
        rangeForStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostRangeForStatement));
        cmajor::parsing::NonterminalParser* forStatementNonterminalParser = GetNonterminal(ToUtf32("ForStatement"));
        forStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreForStatement));
        forStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostForStatement));
        cmajor::parsing::NonterminalParser* breakStatementNonterminalParser = GetNonterminal(ToUtf32("BreakStatement"));
        breakStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreBreakStatement));
        breakStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostBreakStatement));
        cmajor::parsing::NonterminalParser* continueStatementNonterminalParser = GetNonterminal(ToUtf32("ContinueStatement"));
        continueStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreContinueStatement));
        continueStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostContinueStatement));
        cmajor::parsing::NonterminalParser* gotoStatementNonterminalParser = GetNonterminal(ToUtf32("GotoStatement"));
        gotoStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreGotoStatement));
        gotoStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostGotoStatement));
        cmajor::parsing::NonterminalParser* switchStatementNonterminalParser = GetNonterminal(ToUtf32("SwitchStatement"));
        switchStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreSwitchStatement));
        switchStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostSwitchStatement));
        cmajor::parsing::NonterminalParser* gotoCaseStatementNonterminalParser = GetNonterminal(ToUtf32("GotoCaseStatement"));
        gotoCaseStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreGotoCaseStatement));
        gotoCaseStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostGotoCaseStatement));
        cmajor::parsing::NonterminalParser* gotoDefaultStatementNonterminalParser = GetNonterminal(ToUtf32("GotoDefaultStatement"));
        gotoDefaultStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ControlStatementRule>(this, &ControlStatementRule::PreGotoDefaultStatement));
        gotoDefaultStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ControlStatementRule>(this, &ControlStatementRule::PostGotoDefaultStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromCompoundStatement;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromReturnStatement;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromIfStatement;
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromWhileStatement;
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromDoStatement;
    }
    void A5Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromRangeForStatement;
    }
    void A6Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromForStatement;
    }
    void A7Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromBreakStatement;
    }
    void A8Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromContinueStatement;
    }
    void A9Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromGotoStatement;
    }
    void A10Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromSwitchStatement;
    }
    void A11Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromGotoCaseStatement;
    }
    void A12Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromGotoDefaultStatement;
    }
    void PreCompoundStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostCompoundStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromCompoundStatement_value = std::move(stack.top());
            context->fromCompoundStatement = *static_cast<cmajor::parsing::ValueObject<CompoundStatementNode*>*>(fromCompoundStatement_value.get());
            stack.pop();
        }
    }
    void PreReturnStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostReturnStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromReturnStatement_value = std::move(stack.top());
            context->fromReturnStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromReturnStatement_value.get());
            stack.pop();
        }
    }
    void PreIfStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostIfStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromIfStatement_value = std::move(stack.top());
            context->fromIfStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromIfStatement_value.get());
            stack.pop();
        }
    }
    void PreWhileStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostWhileStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromWhileStatement_value = std::move(stack.top());
            context->fromWhileStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromWhileStatement_value.get());
            stack.pop();
        }
    }
    void PreDoStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostDoStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromDoStatement_value = std::move(stack.top());
            context->fromDoStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromDoStatement_value.get());
            stack.pop();
        }
    }
    void PreRangeForStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostRangeForStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromRangeForStatement_value = std::move(stack.top());
            context->fromRangeForStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromRangeForStatement_value.get());
            stack.pop();
        }
    }
    void PreForStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostForStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromForStatement_value = std::move(stack.top());
            context->fromForStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromForStatement_value.get());
            stack.pop();
        }
    }
    void PreBreakStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostBreakStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromBreakStatement_value = std::move(stack.top());
            context->fromBreakStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromBreakStatement_value.get());
            stack.pop();
        }
    }
    void PreContinueStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostContinueStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromContinueStatement_value = std::move(stack.top());
            context->fromContinueStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromContinueStatement_value.get());
            stack.pop();
        }
    }
    void PreGotoStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostGotoStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromGotoStatement_value = std::move(stack.top());
            context->fromGotoStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromGotoStatement_value.get());
            stack.pop();
        }
    }
    void PreSwitchStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostSwitchStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromSwitchStatement_value = std::move(stack.top());
            context->fromSwitchStatement = *static_cast<cmajor::parsing::ValueObject<SwitchStatementNode*>*>(fromSwitchStatement_value.get());
            stack.pop();
        }
    }
    void PreGotoCaseStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostGotoCaseStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromGotoCaseStatement_value = std::move(stack.top());
            context->fromGotoCaseStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromGotoCaseStatement_value.get());
            stack.pop();
        }
    }
    void PreGotoDefaultStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostGotoDefaultStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromGotoDefaultStatement_value = std::move(stack.top());
            context->fromGotoDefaultStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromGotoDefaultStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromCompoundStatement(), fromReturnStatement(), fromIfStatement(), fromWhileStatement(), fromDoStatement(), fromRangeForStatement(), fromForStatement(), fromBreakStatement(), fromContinueStatement(), fromGotoStatement(), fromSwitchStatement(), fromGotoCaseStatement(), fromGotoDefaultStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        CompoundStatementNode* fromCompoundStatement;
        StatementNode* fromReturnStatement;
        StatementNode* fromIfStatement;
        StatementNode* fromWhileStatement;
        StatementNode* fromDoStatement;
        StatementNode* fromRangeForStatement;
        StatementNode* fromForStatement;
        StatementNode* fromBreakStatement;
        StatementNode* fromContinueStatement;
        StatementNode* fromGotoStatement;
        SwitchStatementNode* fromSwitchStatement;
        StatementNode* fromGotoCaseStatement;
        StatementNode* fromGotoDefaultStatement;
    };
};

class StatementGrammar::CompoundStatementRule : public cmajor::parsing::Rule
{
public:
    CompoundStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("CompoundStatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<CompoundStatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A1Action));
        cmajor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A2Action));
        cmajor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<CompoundStatementRule>(this, &CompoundStatementRule::A3Action));
        cmajor::parsing::NonterminalParser* stmtNonterminalParser = GetNonterminal(ToUtf32("stmt"));
        stmtNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<CompoundStatementRule>(this, &CompoundStatementRule::Prestmt));
        stmtNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<CompoundStatementRule>(this, &CompoundStatementRule::Poststmt));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->GetSpan().SetEnd(span.End());
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CompoundStatementNode(span);
        context->value->SetBeginBraceSpan(span);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->AddStatement(context->fromstmt);
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->SetEndBraceSpan(span);
    }
    void Prestmt(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void Poststmt(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromstmt_value = std::move(stack.top());
            context->fromstmt = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromstmt_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromstmt() {}
        ParsingContext* ctx;
        CompoundStatementNode* value;
        StatementNode* fromstmt;
    };
};

class StatementGrammar::ReturnStatementRule : public cmajor::parsing::Rule
{
public:
    ReturnStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ReturnStatementRule>(this, &ReturnStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ReturnStatementRule>(this, &ReturnStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ReturnStatementRule>(this, &ReturnStatementRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ReturnStatementNode(span, context->fromExpression);
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
    };
};

class StatementGrammar::IfStatementRule : public cmajor::parsing::Rule
{
public:
    IfStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<IfStatementRule>(this, &IfStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<IfStatementRule>(this, &IfStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::PostExpression));
        cmajor::parsing::NonterminalParser* thensNonterminalParser = GetNonterminal(ToUtf32("thens"));
        thensNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<IfStatementRule>(this, &IfStatementRule::Prethens));
        thensNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::Postthens));
        cmajor::parsing::NonterminalParser* elsesNonterminalParser = GetNonterminal(ToUtf32("elses"));
        elsesNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<IfStatementRule>(this, &IfStatementRule::Preelses));
        elsesNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<IfStatementRule>(this, &IfStatementRule::Postelses));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new IfStatementNode(span, context->fromExpression, context->fromthens, context->fromelses);
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void Prethens(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void Postthens(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromthens_value = std::move(stack.top());
            context->fromthens = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromthens_value.get());
            stack.pop();
        }
    }
    void Preelses(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void Postelses(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromelses_value = std::move(stack.top());
            context->fromelses = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromelses_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromExpression(), fromthens(), fromelses() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
        StatementNode* fromthens;
        StatementNode* fromelses;
    };
};

class StatementGrammar::WhileStatementRule : public cmajor::parsing::Rule
{
public:
    WhileStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<WhileStatementRule>(this, &WhileStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<WhileStatementRule>(this, &WhileStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<WhileStatementRule>(this, &WhileStatementRule::PostExpression));
        cmajor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal(ToUtf32("Statement"));
        statementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<WhileStatementRule>(this, &WhileStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<WhileStatementRule>(this, &WhileStatementRule::PostStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new WhileStatementNode(span, context->fromExpression, context->fromStatement);
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromExpression(), fromStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
        StatementNode* fromStatement;
    };
};

class StatementGrammar::DoStatementRule : public cmajor::parsing::Rule
{
public:
    DoStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<DoStatementRule>(this, &DoStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal(ToUtf32("Statement"));
        statementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<DoStatementRule>(this, &DoStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<DoStatementRule>(this, &DoStatementRule::PostStatement));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<DoStatementRule>(this, &DoStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<DoStatementRule>(this, &DoStatementRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DoStatementNode(span, context->fromStatement, context->fromExpression);
    }
    void PreStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromStatement(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromStatement;
        Node* fromExpression;
    };
};

class StatementGrammar::ForStatementRule : public cmajor::parsing::Rule
{
public:
    ForStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ForStatementRule>(this, &ForStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* forInitStatementNonterminalParser = GetNonterminal(ToUtf32("ForInitStatement"));
        forInitStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreForInitStatement));
        forInitStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostForInitStatement));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostExpression));
        cmajor::parsing::NonterminalParser* forLoopStatementExprNonterminalParser = GetNonterminal(ToUtf32("ForLoopStatementExpr"));
        forLoopStatementExprNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreForLoopStatementExpr));
        forLoopStatementExprNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostForLoopStatementExpr));
        cmajor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal(ToUtf32("Statement"));
        statementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ForStatementRule>(this, &ForStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ForStatementRule>(this, &ForStatementRule::PostStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ForStatementNode(span, context->fromForInitStatement, context->fromExpression, context->fromForLoopStatementExpr, context->fromStatement);
    }
    void PreForInitStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostForInitStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromForInitStatement_value = std::move(stack.top());
            context->fromForInitStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromForInitStatement_value.get());
            stack.pop();
        }
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreForLoopStatementExpr(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostForLoopStatementExpr(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromForLoopStatementExpr_value = std::move(stack.top());
            context->fromForLoopStatementExpr = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromForLoopStatementExpr_value.get());
            stack.pop();
        }
    }
    void PreStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromForInitStatement(), fromExpression(), fromForLoopStatementExpr(), fromStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromForInitStatement;
        Node* fromExpression;
        StatementNode* fromForLoopStatementExpr;
        StatementNode* fromStatement;
    };
};

class StatementGrammar::ForInitStatementRule : public cmajor::parsing::Rule
{
public:
    ForInitStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A1Action));
        cmajor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ForInitStatementRule>(this, &ForInitStatementRule::A2Action));
        cmajor::parsing::NonterminalParser* assignmentStatementNonterminalParser = GetNonterminal(ToUtf32("AssignmentStatement"));
        assignmentStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ForInitStatementRule>(this, &ForInitStatementRule::PreAssignmentStatement));
        assignmentStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostAssignmentStatement));
        cmajor::parsing::NonterminalParser* constructionStatementNonterminalParser = GetNonterminal(ToUtf32("ConstructionStatement"));
        constructionStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ForInitStatementRule>(this, &ForInitStatementRule::PreConstructionStatement));
        constructionStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostConstructionStatement));
        cmajor::parsing::NonterminalParser* emptyStatementNonterminalParser = GetNonterminal(ToUtf32("EmptyStatement"));
        emptyStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ForInitStatementRule>(this, &ForInitStatementRule::PreEmptyStatement));
        emptyStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ForInitStatementRule>(this, &ForInitStatementRule::PostEmptyStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromAssignmentStatement;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromConstructionStatement;
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromEmptyStatement;
    }
    void PreAssignmentStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostAssignmentStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromAssignmentStatement_value = std::move(stack.top());
            context->fromAssignmentStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromAssignmentStatement_value.get());
            stack.pop();
        }
    }
    void PreConstructionStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostConstructionStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromConstructionStatement_value = std::move(stack.top());
            context->fromConstructionStatement = *static_cast<cmajor::parsing::ValueObject<ConstructionStatementNode*>*>(fromConstructionStatement_value.get());
            stack.pop();
        }
    }
    void PreEmptyStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostEmptyStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromEmptyStatement_value = std::move(stack.top());
            context->fromEmptyStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromEmptyStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromAssignmentStatement(), fromConstructionStatement(), fromEmptyStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromAssignmentStatement;
        ConstructionStatementNode* fromConstructionStatement;
        StatementNode* fromEmptyStatement;
    };
};

class StatementGrammar::ForLoopStatementExprRule : public cmajor::parsing::Rule
{
public:
    ForLoopStatementExprRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ForLoopStatementExprRule>(this, &ForLoopStatementExprRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ForLoopStatementExprRule>(this, &ForLoopStatementExprRule::A1Action));
        cmajor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ForLoopStatementExprRule>(this, &ForLoopStatementExprRule::A2Action));
        cmajor::parsing::NonterminalParser* assignmentStatementExprNonterminalParser = GetNonterminal(ToUtf32("AssignmentStatementExpr"));
        assignmentStatementExprNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ForLoopStatementExprRule>(this, &ForLoopStatementExprRule::PreAssignmentStatementExpr));
        assignmentStatementExprNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ForLoopStatementExprRule>(this, &ForLoopStatementExprRule::PostAssignmentStatementExpr));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ForLoopStatementExprRule>(this, &ForLoopStatementExprRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ForLoopStatementExprRule>(this, &ForLoopStatementExprRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromAssignmentStatementExpr;
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ExpressionStatementNode(span, context->fromExpression);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new EmptyStatementNode(span);
    }
    void PreAssignmentStatementExpr(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostAssignmentStatementExpr(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromAssignmentStatementExpr_value = std::move(stack.top());
            context->fromAssignmentStatementExpr = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromAssignmentStatementExpr_value.get());
            stack.pop();
        }
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromAssignmentStatementExpr(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromAssignmentStatementExpr;
        Node* fromExpression;
    };
};

class StatementGrammar::RangeForStatementRule : public cmajor::parsing::Rule
{
public:
    RangeForStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<RangeForStatementRule>(this, &RangeForStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal(ToUtf32("TypeExpr"));
        typeExprNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<RangeForStatementRule>(this, &RangeForStatementRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<RangeForStatementRule>(this, &RangeForStatementRule::PostTypeExpr));
        cmajor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<RangeForStatementRule>(this, &RangeForStatementRule::PostIdentifier));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<RangeForStatementRule>(this, &RangeForStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<RangeForStatementRule>(this, &RangeForStatementRule::PostExpression));
        cmajor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal(ToUtf32("Statement"));
        statementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<RangeForStatementRule>(this, &RangeForStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<RangeForStatementRule>(this, &RangeForStatementRule::PostStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new RangeForStatementNode(span, context->fromTypeExpr, context->fromIdentifier, context->fromExpression, context->fromStatement);
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
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromTypeExpr(), fromIdentifier(), fromExpression(), fromStatement() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
        Node* fromExpression;
        StatementNode* fromStatement;
    };
};

class StatementGrammar::BreakStatementRule : public cmajor::parsing::Rule
{
public:
    BreakStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<BreakStatementRule>(this, &BreakStatementRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new BreakStatementNode(span);
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        StatementNode* value;
    };
};

class StatementGrammar::ContinueStatementRule : public cmajor::parsing::Rule
{
public:
    ContinueStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ContinueStatementRule>(this, &ContinueStatementRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ContinueStatementNode(span);
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        StatementNode* value;
    };
};

class StatementGrammar::GotoStatementRule : public cmajor::parsing::Rule
{
public:
    GotoStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<GotoStatementRule>(this, &GotoStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<GotoStatementRule>(this, &GotoStatementRule::PostIdentifier));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new GotoStatementNode(span, context->fromIdentifier->Str());
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
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromIdentifier() {}
        ParsingContext* ctx;
        StatementNode* value;
        IdentifierNode* fromIdentifier;
    };
};

class StatementGrammar::SwitchStatementRule : public cmajor::parsing::Rule
{
public:
    SwitchStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("SwitchStatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<SwitchStatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<SwitchStatementRule>(this, &SwitchStatementRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<SwitchStatementRule>(this, &SwitchStatementRule::A1Action));
        cmajor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<SwitchStatementRule>(this, &SwitchStatementRule::A2Action));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<SwitchStatementRule>(this, &SwitchStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostExpression));
        cmajor::parsing::NonterminalParser* caseStatementNonterminalParser = GetNonterminal(ToUtf32("CaseStatement"));
        caseStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<SwitchStatementRule>(this, &SwitchStatementRule::PreCaseStatement));
        caseStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostCaseStatement));
        cmajor::parsing::NonterminalParser* defaultStatementNonterminalParser = GetNonterminal(ToUtf32("DefaultStatement"));
        defaultStatementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<SwitchStatementRule>(this, &SwitchStatementRule::PreDefaultStatement));
        defaultStatementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<SwitchStatementRule>(this, &SwitchStatementRule::PostDefaultStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new SwitchStatementNode(span, context->fromExpression);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->AddCase(context->fromCaseStatement);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->SetDefault(context->fromDefaultStatement);
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreCaseStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostCaseStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromCaseStatement_value = std::move(stack.top());
            context->fromCaseStatement = *static_cast<cmajor::parsing::ValueObject<CaseStatementNode*>*>(fromCaseStatement_value.get());
            stack.pop();
        }
    }
    void PreDefaultStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostDefaultStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromDefaultStatement_value = std::move(stack.top());
            context->fromDefaultStatement = *static_cast<cmajor::parsing::ValueObject<DefaultStatementNode*>*>(fromDefaultStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromExpression(), fromCaseStatement(), fromDefaultStatement() {}
        ParsingContext* ctx;
        SwitchStatementNode* value;
        Node* fromExpression;
        CaseStatementNode* fromCaseStatement;
        DefaultStatementNode* fromDefaultStatement;
    };
};

class StatementGrammar::CaseStatementRule : public cmajor::parsing::Rule
{
public:
    CaseStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("CaseStatementNode*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<CaseStatementNode>"), ToUtf32("caseS")));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<CaseStatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A1Action));
        cmajor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A2Action));
        cmajor::parsing::ActionParser* a3ActionParser = GetAction(ToUtf32("A3"));
        a3ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A3Action));
        cmajor::parsing::ActionParser* a4ActionParser = GetAction(ToUtf32("A4"));
        a4ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<CaseStatementRule>(this, &CaseStatementRule::A4Action));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<CaseStatementRule>(this, &CaseStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<CaseStatementRule>(this, &CaseStatementRule::PostExpression));
        cmajor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal(ToUtf32("Statement"));
        statementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<CaseStatementRule>(this, &CaseStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<CaseStatementRule>(this, &CaseStatementRule::PostStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->caseS.release();
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->caseS->GetSpan().SetEnd(span.End());
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->caseS.reset(new CaseStatementNode(span));
    }
    void A3Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->caseS->AddCaseExpr(context->fromExpression);
    }
    void A4Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->caseS->AddStatement(context->fromStatement);
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), caseS(), fromExpression(), fromStatement() {}
        ParsingContext* ctx;
        CaseStatementNode* value;
        std::unique_ptr<CaseStatementNode> caseS;
        Node* fromExpression;
        StatementNode* fromStatement;
    };
};

class StatementGrammar::DefaultStatementRule : public cmajor::parsing::Rule
{
public:
    DefaultStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("DefaultStatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<DefaultStatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<DefaultStatementRule>(this, &DefaultStatementRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<DefaultStatementRule>(this, &DefaultStatementRule::A1Action));
        cmajor::parsing::NonterminalParser* statementNonterminalParser = GetNonterminal(ToUtf32("Statement"));
        statementNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<DefaultStatementRule>(this, &DefaultStatementRule::PreStatement));
        statementNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<DefaultStatementRule>(this, &DefaultStatementRule::PostStatement));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DefaultStatementNode(span);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->AddStatement(context->fromStatement);
    }
    void PreStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostStatement(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromStatement_value = std::move(stack.top());
            context->fromStatement = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromStatement_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromStatement() {}
        ParsingContext* ctx;
        DefaultStatementNode* value;
        StatementNode* fromStatement;
    };
};

class StatementGrammar::GotoCaseStatementRule : public cmajor::parsing::Rule
{
public:
    GotoCaseStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<GotoCaseStatementRule>(this, &GotoCaseStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<GotoCaseStatementRule>(this, &GotoCaseStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<GotoCaseStatementRule>(this, &GotoCaseStatementRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new GotoCaseStatementNode(span, context->fromExpression);
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
    };
};

class StatementGrammar::GotoDefaultStatementRule : public cmajor::parsing::Rule
{
public:
    GotoDefaultStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<GotoDefaultStatementRule>(this, &GotoDefaultStatementRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new GotoDefaultStatementNode(span);
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        StatementNode* value;
    };
};

class StatementGrammar::AssignmentStatementExprRule : public cmajor::parsing::Rule
{
public:
    AssignmentStatementExprRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<Node>"), ToUtf32("targetExpr")));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<AssignmentStatementExprRule>(this, &AssignmentStatementExprRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<AssignmentStatementExprRule>(this, &AssignmentStatementExprRule::A1Action));
        cmajor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<AssignmentStatementExprRule>(this, &AssignmentStatementExprRule::A2Action));
        a2ActionParser->SetFailureAction(new cmajor::parsing::MemberFailureAction<AssignmentStatementExprRule>(this, &AssignmentStatementExprRule::A2ActionFail));
        cmajor::parsing::NonterminalParser* targetNonterminalParser = GetNonterminal(ToUtf32("target"));
        targetNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<AssignmentStatementExprRule>(this, &AssignmentStatementExprRule::Pretarget));
        targetNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<AssignmentStatementExprRule>(this, &AssignmentStatementExprRule::Posttarget));
        cmajor::parsing::NonterminalParser* sourceNonterminalParser = GetNonterminal(ToUtf32("source"));
        sourceNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<AssignmentStatementExprRule>(this, &AssignmentStatementExprRule::Presource));
        sourceNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<AssignmentStatementExprRule>(this, &AssignmentStatementExprRule::Postsource));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctx->PushParsingLvalue(true);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AssignmentStatementNode(span, context->targetExpr.release(), context->fromsource);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctx->PopParsingLvalue();
        context->targetExpr.reset(context->fromtarget);
    }
    void A2ActionFail(ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctx->PopParsingLvalue();
    }
    void Pretarget(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void Posttarget(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromtarget_value = std::move(stack.top());
            context->fromtarget = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromtarget_value.get());
            stack.pop();
        }
    }
    void Presource(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void Postsource(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromsource_value = std::move(stack.top());
            context->fromsource = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromsource_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), targetExpr(), fromtarget(), fromsource() {}
        ParsingContext* ctx;
        StatementNode* value;
        std::unique_ptr<Node> targetExpr;
        Node* fromtarget;
        Node* fromsource;
    };
};

class StatementGrammar::AssignmentStatementRule : public cmajor::parsing::Rule
{
public:
    AssignmentStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<AssignmentStatementRule>(this, &AssignmentStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* assignmentStatementExprNonterminalParser = GetNonterminal(ToUtf32("AssignmentStatementExpr"));
        assignmentStatementExprNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<AssignmentStatementRule>(this, &AssignmentStatementRule::PreAssignmentStatementExpr));
        assignmentStatementExprNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<AssignmentStatementRule>(this, &AssignmentStatementRule::PostAssignmentStatementExpr));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = context->fromAssignmentStatementExpr;
        context->value->GetSpan().SetEnd(span.End());
    }
    void PreAssignmentStatementExpr(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostAssignmentStatementExpr(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromAssignmentStatementExpr_value = std::move(stack.top());
            context->fromAssignmentStatementExpr = *static_cast<cmajor::parsing::ValueObject<StatementNode*>*>(fromAssignmentStatementExpr_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromAssignmentStatementExpr() {}
        ParsingContext* ctx;
        StatementNode* value;
        StatementNode* fromAssignmentStatementExpr;
    };
};

class StatementGrammar::ConstructionStatementRule : public cmajor::parsing::Rule
{
public:
    ConstructionStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("ConstructionStatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ConstructionStatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ConstructionStatementRule>(this, &ConstructionStatementRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ConstructionStatementRule>(this, &ConstructionStatementRule::A1Action));
        cmajor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ConstructionStatementRule>(this, &ConstructionStatementRule::A2Action));
        cmajor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal(ToUtf32("TypeExpr"));
        typeExprNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PostTypeExpr));
        cmajor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PostIdentifier));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PostExpression));
        cmajor::parsing::NonterminalParser* argumentListNonterminalParser = GetNonterminal(ToUtf32("ArgumentList"));
        argumentListNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ConstructionStatementRule>(this, &ConstructionStatementRule::PreArgumentList));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ConstructionStatementNode(span, context->fromTypeExpr, context->fromIdentifier);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->AddArgument(context->fromExpression);
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->GetSpan().SetEnd(span.End());
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
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
    void PreArgumentList(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<Node*>(context->value)));
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromTypeExpr(), fromIdentifier(), fromExpression() {}
        ParsingContext* ctx;
        ConstructionStatementNode* value;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
        Node* fromExpression;
    };
};

class StatementGrammar::DeleteStatementRule : public cmajor::parsing::Rule
{
public:
    DeleteStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<DeleteStatementRule>(this, &DeleteStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<DeleteStatementRule>(this, &DeleteStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<DeleteStatementRule>(this, &DeleteStatementRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DeleteStatementNode(span, context->fromExpression);
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
    };
};

class StatementGrammar::DestroyStatementRule : public cmajor::parsing::Rule
{
public:
    DestroyStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<DestroyStatementRule>(this, &DestroyStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<DestroyStatementRule>(this, &DestroyStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<DestroyStatementRule>(this, &DestroyStatementRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new DestroyStatementNode(span, context->fromExpression);
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
    };
};

class StatementGrammar::ExpressionStatementRule : public cmajor::parsing::Rule
{
public:
    ExpressionStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
        AddLocalVariable(AttrOrVariable(ToUtf32("std::unique_ptr<Node>"), ToUtf32("expr")));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ExpressionStatementRule>(this, &ExpressionStatementRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ExpressionStatementRule>(this, &ExpressionStatementRule::A1Action));
        a1ActionParser->SetFailureAction(new cmajor::parsing::MemberFailureAction<ExpressionStatementRule>(this, &ExpressionStatementRule::A1ActionFail));
        cmajor::parsing::ActionParser* a2ActionParser = GetAction(ToUtf32("A2"));
        a2ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ExpressionStatementRule>(this, &ExpressionStatementRule::A2Action));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ExpressionStatementRule>(this, &ExpressionStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ExpressionStatementRule>(this, &ExpressionStatementRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctx->PushParsingExpressionStatement(true);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctx->PopParsingExpressionStatement();
        context->value = new ExpressionStatementNode(span, context->expr.release());
    }
    void A1ActionFail(ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->ctx->PopParsingExpressionStatement();
    }
    void A2Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->expr.reset(context->fromExpression);
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), expr(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        std::unique_ptr<Node> expr;
        Node* fromExpression;
    };
};

class StatementGrammar::EmptyStatementRule : public cmajor::parsing::Rule
{
public:
    EmptyStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<EmptyStatementRule>(this, &EmptyStatementRule::A0Action));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new EmptyStatementNode(span);
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value() {}
        ParsingContext* ctx;
        StatementNode* value;
    };
};

class StatementGrammar::ThrowStatementRule : public cmajor::parsing::Rule
{
public:
    ThrowStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<ThrowStatementRule>(this, &ThrowStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<ThrowStatementRule>(this, &ThrowStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<ThrowStatementRule>(this, &ThrowStatementRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new ThrowStatementNode(span, context->fromExpression);
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
    };
};

class StatementGrammar::TryStatementRule : public cmajor::parsing::Rule
{
public:
    TryStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("TryStatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<TryStatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A0Action));
        cmajor::parsing::ActionParser* a1ActionParser = GetAction(ToUtf32("A1"));
        a1ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<TryStatementRule>(this, &TryStatementRule::A1Action));
        cmajor::parsing::NonterminalParser* tryBlockNonterminalParser = GetNonterminal(ToUtf32("tryBlock"));
        tryBlockNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::PretryBlock));
        tryBlockNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::PosttryBlock));
        cmajor::parsing::NonterminalParser* catchNonterminalParser = GetNonterminal(ToUtf32("Catch"));
        catchNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<TryStatementRule>(this, &TryStatementRule::PreCatch));
        catchNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<TryStatementRule>(this, &TryStatementRule::PostCatch));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new TryStatementNode(span, context->fromtryBlock);
    }
    void A1Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value->AddCatch(context->fromCatch);
    }
    void PretryBlock(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PosttryBlock(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromtryBlock_value = std::move(stack.top());
            context->fromtryBlock = *static_cast<cmajor::parsing::ValueObject<CompoundStatementNode*>*>(fromtryBlock_value.get());
            stack.pop();
        }
    }
    void PreCatch(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostCatch(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromCatch_value = std::move(stack.top());
            context->fromCatch = *static_cast<cmajor::parsing::ValueObject<CatchNode*>*>(fromCatch_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromtryBlock(), fromCatch() {}
        ParsingContext* ctx;
        TryStatementNode* value;
        CompoundStatementNode* fromtryBlock;
        CatchNode* fromCatch;
    };
};

class StatementGrammar::CatchRule : public cmajor::parsing::Rule
{
public:
    CatchRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("CatchNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<CatchNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<CatchRule>(this, &CatchRule::A0Action));
        cmajor::parsing::NonterminalParser* typeExprNonterminalParser = GetNonterminal(ToUtf32("TypeExpr"));
        typeExprNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<CatchRule>(this, &CatchRule::PreTypeExpr));
        typeExprNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<CatchRule>(this, &CatchRule::PostTypeExpr));
        cmajor::parsing::NonterminalParser* identifierNonterminalParser = GetNonterminal(ToUtf32("Identifier"));
        identifierNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<CatchRule>(this, &CatchRule::PostIdentifier));
        cmajor::parsing::NonterminalParser* catchBlockNonterminalParser = GetNonterminal(ToUtf32("catchBlock"));
        catchBlockNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<CatchRule>(this, &CatchRule::PrecatchBlock));
        catchBlockNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<CatchRule>(this, &CatchRule::PostcatchBlock));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new CatchNode(span, context->fromTypeExpr, context->fromIdentifier, context->fromcatchBlock);
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
    void PrecatchBlock(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostcatchBlock(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromcatchBlock_value = std::move(stack.top());
            context->fromcatchBlock = *static_cast<cmajor::parsing::ValueObject<CompoundStatementNode*>*>(fromcatchBlock_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromTypeExpr(), fromIdentifier(), fromcatchBlock() {}
        ParsingContext* ctx;
        CatchNode* value;
        Node* fromTypeExpr;
        IdentifierNode* fromIdentifier;
        CompoundStatementNode* fromcatchBlock;
    };
};

class StatementGrammar::AssertStatementRule : public cmajor::parsing::Rule
{
public:
    AssertStatementRule(const std::u32string& name_, Scope* enclosingScope_, int id_, Parser* definition_):
        cmajor::parsing::Rule(name_, enclosingScope_, id_, definition_)
    {
        AddInheritedAttribute(AttrOrVariable(ToUtf32("ParsingContext*"), ToUtf32("ctx")));
        SetValueTypeName(ToUtf32("StatementNode*"));
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
            stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<StatementNode*>(context->value)));
        }
        parsingData->PopContext(Id());
    }
    virtual void Link()
    {
        cmajor::parsing::ActionParser* a0ActionParser = GetAction(ToUtf32("A0"));
        a0ActionParser->SetAction(new cmajor::parsing::MemberParsingAction<AssertStatementRule>(this, &AssertStatementRule::A0Action));
        cmajor::parsing::NonterminalParser* expressionNonterminalParser = GetNonterminal(ToUtf32("Expression"));
        expressionNonterminalParser->SetPreCall(new cmajor::parsing::MemberPreCall<AssertStatementRule>(this, &AssertStatementRule::PreExpression));
        expressionNonterminalParser->SetPostCall(new cmajor::parsing::MemberPostCall<AssertStatementRule>(this, &AssertStatementRule::PostExpression));
    }
    void A0Action(const char32_t* matchBegin, const char32_t* matchEnd, const Span& span, const std::string& fileName, ParsingData* parsingData, bool& pass)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        context->value = new AssertStatementNode(span, context->fromExpression);
    }
    void PreExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        stack.push(std::unique_ptr<cmajor::parsing::Object>(new cmajor::parsing::ValueObject<ParsingContext*>(context->ctx)));
    }
    void PostExpression(cmajor::parsing::ObjectStack& stack, ParsingData* parsingData, bool matched)
    {
        Context* context = static_cast<Context*>(parsingData->GetContext(Id()));
        if (matched)
        {
            std::unique_ptr<cmajor::parsing::Object> fromExpression_value = std::move(stack.top());
            context->fromExpression = *static_cast<cmajor::parsing::ValueObject<Node*>*>(fromExpression_value.get());
            stack.pop();
        }
    }
private:
    struct Context : cmajor::parsing::Context
    {
        Context(): ctx(), value(), fromExpression() {}
        ParsingContext* ctx;
        StatementNode* value;
        Node* fromExpression;
    };
};

void StatementGrammar::GetReferencedGrammars()
{
    cmajor::parsing::ParsingDomain* pd = GetParsingDomain();
    cmajor::parsing::Grammar* grammar0 = pd->GetGrammar(ToUtf32("cmajor.parsing.stdlib"));
    if (!grammar0)
    {
        grammar0 = cmajor::parsing::stdlib::Create(pd);
    }
    AddGrammarReference(grammar0);
    cmajor::parsing::Grammar* grammar1 = pd->GetGrammar(ToUtf32("cmajor.parser.KeywordGrammar"));
    if (!grammar1)
    {
        grammar1 = cmajor::parser::KeywordGrammar::Create(pd);
    }
    AddGrammarReference(grammar1);
    cmajor::parsing::Grammar* grammar2 = pd->GetGrammar(ToUtf32("cmajor.parser.ExpressionGrammar"));
    if (!grammar2)
    {
        grammar2 = cmajor::parser::ExpressionGrammar::Create(pd);
    }
    AddGrammarReference(grammar2);
    cmajor::parsing::Grammar* grammar3 = pd->GetGrammar(ToUtf32("cmajor.parser.TypeExprGrammar"));
    if (!grammar3)
    {
        grammar3 = cmajor::parser::TypeExprGrammar::Create(pd);
    }
    AddGrammarReference(grammar3);
    cmajor::parsing::Grammar* grammar4 = pd->GetGrammar(ToUtf32("cmajor.parser.IdentifierGrammar"));
    if (!grammar4)
    {
        grammar4 = cmajor::parser::IdentifierGrammar::Create(pd);
    }
    AddGrammarReference(grammar4);
}

void StatementGrammar::CreateRules()
{
    AddRuleLink(new cmajor::parsing::RuleLink(ToUtf32("Keyword"), this, ToUtf32("KeywordGrammar.Keyword")));
    AddRuleLink(new cmajor::parsing::RuleLink(ToUtf32("identifier"), this, ToUtf32("cmajor.parsing.stdlib.identifier")));
    AddRuleLink(new cmajor::parsing::RuleLink(ToUtf32("Expression"), this, ToUtf32("ExpressionGrammar.Expression")));
    AddRuleLink(new cmajor::parsing::RuleLink(ToUtf32("TypeExpr"), this, ToUtf32("TypeExprGrammar.TypeExpr")));
    AddRuleLink(new cmajor::parsing::RuleLink(ToUtf32("Identifier"), this, ToUtf32("IdentifierGrammar.Identifier")));
    AddRuleLink(new cmajor::parsing::RuleLink(ToUtf32("ArgumentList"), this, ToUtf32("ExpressionGrammar.ArgumentList")));
    AddRule(new StatementRule(ToUtf32("Statement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::AlternativeParser(
            new cmajor::parsing::AlternativeParser(
                new cmajor::parsing::AlternativeParser(
                    new cmajor::parsing::AlternativeParser(
                        new cmajor::parsing::AlternativeParser(
                            new cmajor::parsing::AlternativeParser(
                                new cmajor::parsing::AlternativeParser(
                                    new cmajor::parsing::AlternativeParser(
                                        new cmajor::parsing::AlternativeParser(
                                            new cmajor::parsing::AlternativeParser(
                                                new cmajor::parsing::ActionParser(ToUtf32("A0"),
                                                    new cmajor::parsing::NonterminalParser(ToUtf32("LabeledStatement"), ToUtf32("LabeledStatement"), 1)),
                                                new cmajor::parsing::ActionParser(ToUtf32("A1"),
                                                    new cmajor::parsing::NonterminalParser(ToUtf32("ControlStatement"), ToUtf32("ControlStatement"), 1))),
                                            new cmajor::parsing::ActionParser(ToUtf32("A2"),
                                                new cmajor::parsing::NonterminalParser(ToUtf32("ExpressionStatement"), ToUtf32("ExpressionStatement"), 1))),
                                        new cmajor::parsing::ActionParser(ToUtf32("A3"),
                                            new cmajor::parsing::NonterminalParser(ToUtf32("AssignmentStatement"), ToUtf32("AssignmentStatement"), 1))),
                                    new cmajor::parsing::ActionParser(ToUtf32("A4"),
                                        new cmajor::parsing::NonterminalParser(ToUtf32("ConstructionStatement"), ToUtf32("ConstructionStatement"), 1))),
                                new cmajor::parsing::ActionParser(ToUtf32("A5"),
                                    new cmajor::parsing::NonterminalParser(ToUtf32("DeleteStatement"), ToUtf32("DeleteStatement"), 1))),
                            new cmajor::parsing::ActionParser(ToUtf32("A6"),
                                new cmajor::parsing::NonterminalParser(ToUtf32("DestroyStatement"), ToUtf32("DestroyStatement"), 1))),
                        new cmajor::parsing::ActionParser(ToUtf32("A7"),
                            new cmajor::parsing::NonterminalParser(ToUtf32("EmptyStatement"), ToUtf32("EmptyStatement"), 1))),
                    new cmajor::parsing::ActionParser(ToUtf32("A8"),
                        new cmajor::parsing::NonterminalParser(ToUtf32("ThrowStatement"), ToUtf32("ThrowStatement"), 1))),
                new cmajor::parsing::ActionParser(ToUtf32("A9"),
                    new cmajor::parsing::NonterminalParser(ToUtf32("TryStatement"), ToUtf32("TryStatement"), 1))),
            new cmajor::parsing::ActionParser(ToUtf32("A10"),
                new cmajor::parsing::NonterminalParser(ToUtf32("AssertStatement"), ToUtf32("AssertStatement"), 1)))));
    AddRule(new LabelIdRule(ToUtf32("LabelId"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::DifferenceParser(
                new cmajor::parsing::NonterminalParser(ToUtf32("identifier"), ToUtf32("identifier"), 0),
                new cmajor::parsing::NonterminalParser(ToUtf32("Keyword"), ToUtf32("Keyword"), 0)))));
    AddRule(new LabelRule(ToUtf32("Label"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::ActionParser(ToUtf32("A1"),
                    new cmajor::parsing::NonterminalParser(ToUtf32("LabelId"), ToUtf32("LabelId"), 0)),
                new cmajor::parsing::CharParser(':')))));
    AddRule(new LabeledStatementRule(ToUtf32("LabeledStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::SequenceParser(
            new cmajor::parsing::NonterminalParser(ToUtf32("lbl"), ToUtf32("Label"), 0),
            new cmajor::parsing::ActionParser(ToUtf32("A0"),
                new cmajor::parsing::NonterminalParser(ToUtf32("stmt"), ToUtf32("Statement"), 1)))));
    AddRule(new ControlStatementRule(ToUtf32("ControlStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::AlternativeParser(
            new cmajor::parsing::AlternativeParser(
                new cmajor::parsing::AlternativeParser(
                    new cmajor::parsing::AlternativeParser(
                        new cmajor::parsing::AlternativeParser(
                            new cmajor::parsing::AlternativeParser(
                                new cmajor::parsing::AlternativeParser(
                                    new cmajor::parsing::AlternativeParser(
                                        new cmajor::parsing::AlternativeParser(
                                            new cmajor::parsing::AlternativeParser(
                                                new cmajor::parsing::AlternativeParser(
                                                    new cmajor::parsing::AlternativeParser(
                                                        new cmajor::parsing::ActionParser(ToUtf32("A0"),
                                                            new cmajor::parsing::NonterminalParser(ToUtf32("CompoundStatement"), ToUtf32("CompoundStatement"), 1)),
                                                        new cmajor::parsing::ActionParser(ToUtf32("A1"),
                                                            new cmajor::parsing::NonterminalParser(ToUtf32("ReturnStatement"), ToUtf32("ReturnStatement"), 1))),
                                                    new cmajor::parsing::ActionParser(ToUtf32("A2"),
                                                        new cmajor::parsing::NonterminalParser(ToUtf32("IfStatement"), ToUtf32("IfStatement"), 1))),
                                                new cmajor::parsing::ActionParser(ToUtf32("A3"),
                                                    new cmajor::parsing::NonterminalParser(ToUtf32("WhileStatement"), ToUtf32("WhileStatement"), 1))),
                                            new cmajor::parsing::ActionParser(ToUtf32("A4"),
                                                new cmajor::parsing::NonterminalParser(ToUtf32("DoStatement"), ToUtf32("DoStatement"), 1))),
                                        new cmajor::parsing::ActionParser(ToUtf32("A5"),
                                            new cmajor::parsing::NonterminalParser(ToUtf32("RangeForStatement"), ToUtf32("RangeForStatement"), 1))),
                                    new cmajor::parsing::ActionParser(ToUtf32("A6"),
                                        new cmajor::parsing::NonterminalParser(ToUtf32("ForStatement"), ToUtf32("ForStatement"), 1))),
                                new cmajor::parsing::ActionParser(ToUtf32("A7"),
                                    new cmajor::parsing::NonterminalParser(ToUtf32("BreakStatement"), ToUtf32("BreakStatement"), 1))),
                            new cmajor::parsing::ActionParser(ToUtf32("A8"),
                                new cmajor::parsing::NonterminalParser(ToUtf32("ContinueStatement"), ToUtf32("ContinueStatement"), 1))),
                        new cmajor::parsing::ActionParser(ToUtf32("A9"),
                            new cmajor::parsing::NonterminalParser(ToUtf32("GotoStatement"), ToUtf32("GotoStatement"), 1))),
                    new cmajor::parsing::ActionParser(ToUtf32("A10"),
                        new cmajor::parsing::NonterminalParser(ToUtf32("SwitchStatement"), ToUtf32("SwitchStatement"), 1))),
                new cmajor::parsing::ActionParser(ToUtf32("A11"),
                    new cmajor::parsing::NonterminalParser(ToUtf32("GotoCaseStatement"), ToUtf32("GotoCaseStatement"), 1))),
            new cmajor::parsing::ActionParser(ToUtf32("A12"),
                new cmajor::parsing::NonterminalParser(ToUtf32("GotoDefaultStatement"), ToUtf32("GotoDefaultStatement"), 1)))));
    AddRule(new CompoundStatementRule(ToUtf32("CompoundStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::ActionParser(ToUtf32("A1"),
                        new cmajor::parsing::CharParser('{')),
                    new cmajor::parsing::KleeneStarParser(
                        new cmajor::parsing::ActionParser(ToUtf32("A2"),
                            new cmajor::parsing::NonterminalParser(ToUtf32("stmt"), ToUtf32("Statement"), 1)))),
                new cmajor::parsing::ActionParser(ToUtf32("A3"),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::CharParser('}')))))));
    AddRule(new ReturnStatementRule(ToUtf32("ReturnStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::KeywordParser(ToUtf32("return")),
                    new cmajor::parsing::OptionalParser(
                        new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new IfStatementRule(ToUtf32("IfStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::SequenceParser(
                            new cmajor::parsing::SequenceParser(
                                new cmajor::parsing::KeywordParser(ToUtf32("if")),
                                new cmajor::parsing::CharParser('(')),
                            new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1)),
                        new cmajor::parsing::CharParser(')')),
                    new cmajor::parsing::NonterminalParser(ToUtf32("thens"), ToUtf32("Statement"), 1)),
                new cmajor::parsing::OptionalParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::KeywordParser(ToUtf32("else")),
                        new cmajor::parsing::NonterminalParser(ToUtf32("elses"), ToUtf32("Statement"), 1)))))));
    AddRule(new WhileStatementRule(ToUtf32("WhileStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::SequenceParser(
                            new cmajor::parsing::KeywordParser(ToUtf32("while")),
                            new cmajor::parsing::ExpectationParser(
                                new cmajor::parsing::CharParser('('))),
                        new cmajor::parsing::ExpectationParser(
                            new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::CharParser(')'))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::NonterminalParser(ToUtf32("Statement"), ToUtf32("Statement"), 1))))));
    AddRule(new DoStatementRule(ToUtf32("DoStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::SequenceParser(
                            new cmajor::parsing::SequenceParser(
                                new cmajor::parsing::SequenceParser(
                                    new cmajor::parsing::KeywordParser(ToUtf32("do")),
                                    new cmajor::parsing::ExpectationParser(
                                        new cmajor::parsing::NonterminalParser(ToUtf32("Statement"), ToUtf32("Statement"), 1))),
                                new cmajor::parsing::ExpectationParser(
                                    new cmajor::parsing::KeywordParser(ToUtf32("while")))),
                            new cmajor::parsing::ExpectationParser(
                                new cmajor::parsing::CharParser('('))),
                        new cmajor::parsing::ExpectationParser(
                            new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::CharParser(')'))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new ForStatementRule(ToUtf32("ForStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::SequenceParser(
                            new cmajor::parsing::SequenceParser(
                                new cmajor::parsing::SequenceParser(
                                    new cmajor::parsing::SequenceParser(
                                        new cmajor::parsing::KeywordParser(ToUtf32("for")),
                                        new cmajor::parsing::CharParser('(')),
                                    new cmajor::parsing::ExpectationParser(
                                        new cmajor::parsing::NonterminalParser(ToUtf32("ForInitStatement"), ToUtf32("ForInitStatement"), 1))),
                                new cmajor::parsing::OptionalParser(
                                    new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                            new cmajor::parsing::ExpectationParser(
                                new cmajor::parsing::CharParser(';'))),
                        new cmajor::parsing::ExpectationParser(
                            new cmajor::parsing::NonterminalParser(ToUtf32("ForLoopStatementExpr"), ToUtf32("ForLoopStatementExpr"), 1))),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::CharParser(')'))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::NonterminalParser(ToUtf32("Statement"), ToUtf32("Statement"), 1))))));
    AddRule(new ForInitStatementRule(ToUtf32("ForInitStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::AlternativeParser(
            new cmajor::parsing::AlternativeParser(
                new cmajor::parsing::ActionParser(ToUtf32("A0"),
                    new cmajor::parsing::NonterminalParser(ToUtf32("AssignmentStatement"), ToUtf32("AssignmentStatement"), 1)),
                new cmajor::parsing::ActionParser(ToUtf32("A1"),
                    new cmajor::parsing::NonterminalParser(ToUtf32("ConstructionStatement"), ToUtf32("ConstructionStatement"), 1))),
            new cmajor::parsing::ActionParser(ToUtf32("A2"),
                new cmajor::parsing::NonterminalParser(ToUtf32("EmptyStatement"), ToUtf32("EmptyStatement"), 1)))));
    AddRule(new ForLoopStatementExprRule(ToUtf32("ForLoopStatementExpr"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::AlternativeParser(
            new cmajor::parsing::AlternativeParser(
                new cmajor::parsing::ActionParser(ToUtf32("A0"),
                    new cmajor::parsing::NonterminalParser(ToUtf32("AssignmentStatementExpr"), ToUtf32("AssignmentStatementExpr"), 1)),
                new cmajor::parsing::ActionParser(ToUtf32("A1"),
                    new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
            new cmajor::parsing::ActionParser(ToUtf32("A2"),
                new cmajor::parsing::EmptyParser()))));
    AddRule(new RangeForStatementRule(ToUtf32("RangeForStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::SequenceParser(
                            new cmajor::parsing::SequenceParser(
                                new cmajor::parsing::SequenceParser(
                                    new cmajor::parsing::SequenceParser(
                                        new cmajor::parsing::KeywordParser(ToUtf32("for")),
                                        new cmajor::parsing::CharParser('(')),
                                    new cmajor::parsing::NonterminalParser(ToUtf32("TypeExpr"), ToUtf32("TypeExpr"), 1)),
                                new cmajor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0)),
                            new cmajor::parsing::CharParser(':')),
                        new cmajor::parsing::ExpectationParser(
                            new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::CharParser(')'))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::NonterminalParser(ToUtf32("Statement"), ToUtf32("Statement"), 1))))));
    AddRule(new BreakStatementRule(ToUtf32("BreakStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::KeywordParser(ToUtf32("break")),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new ContinueStatementRule(ToUtf32("ContinueStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::KeywordParser(ToUtf32("continue")),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new GotoStatementRule(ToUtf32("GotoStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::KeywordParser(ToUtf32("goto")),
                    new cmajor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0)),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new SwitchStatementRule(ToUtf32("SwitchStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::SequenceParser(
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::ActionParser(ToUtf32("A0"),
                        new cmajor::parsing::SequenceParser(
                            new cmajor::parsing::SequenceParser(
                                new cmajor::parsing::SequenceParser(
                                    new cmajor::parsing::KeywordParser(ToUtf32("switch")),
                                    new cmajor::parsing::ExpectationParser(
                                        new cmajor::parsing::CharParser('('))),
                                new cmajor::parsing::ExpectationParser(
                                    new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                            new cmajor::parsing::ExpectationParser(
                                new cmajor::parsing::CharParser(')')))),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::CharParser('{'))),
                new cmajor::parsing::KleeneStarParser(
                    new cmajor::parsing::AlternativeParser(
                        new cmajor::parsing::ActionParser(ToUtf32("A1"),
                            new cmajor::parsing::NonterminalParser(ToUtf32("CaseStatement"), ToUtf32("CaseStatement"), 1)),
                        new cmajor::parsing::ActionParser(ToUtf32("A2"),
                            new cmajor::parsing::NonterminalParser(ToUtf32("DefaultStatement"), ToUtf32("DefaultStatement"), 1))))),
            new cmajor::parsing::ExpectationParser(
                new cmajor::parsing::CharParser('}')))));
    AddRule(new CaseStatementRule(ToUtf32("CaseStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::ActionParser(ToUtf32("A1"),
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::ActionParser(ToUtf32("A2"),
                            new cmajor::parsing::EmptyParser()),
                        new cmajor::parsing::PositiveParser(
                            new cmajor::parsing::SequenceParser(
                                new cmajor::parsing::SequenceParser(
                                    new cmajor::parsing::KeywordParser(ToUtf32("case")),
                                    new cmajor::parsing::ActionParser(ToUtf32("A3"),
                                        new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                                new cmajor::parsing::ExpectationParser(
                                    new cmajor::parsing::CharParser(':')))))),
                new cmajor::parsing::KleeneStarParser(
                    new cmajor::parsing::ActionParser(ToUtf32("A4"),
                        new cmajor::parsing::NonterminalParser(ToUtf32("Statement"), ToUtf32("Statement"), 1)))))));
    AddRule(new DefaultStatementRule(ToUtf32("DefaultStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::SequenceParser(
            new cmajor::parsing::ActionParser(ToUtf32("A0"),
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::KeywordParser(ToUtf32("default")),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::CharParser(':')))),
            new cmajor::parsing::KleeneStarParser(
                new cmajor::parsing::ActionParser(ToUtf32("A1"),
                    new cmajor::parsing::NonterminalParser(ToUtf32("Statement"), ToUtf32("Statement"), 1))))));
    AddRule(new GotoCaseStatementRule(ToUtf32("GotoCaseStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::KeywordParser(ToUtf32("goto")),
                        new cmajor::parsing::KeywordParser(ToUtf32("case"))),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new GotoDefaultStatementRule(ToUtf32("GotoDefaultStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::KeywordParser(ToUtf32("goto")),
                    new cmajor::parsing::KeywordParser(ToUtf32("default"))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new AssignmentStatementExprRule(ToUtf32("AssignmentStatementExpr"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::SequenceParser(
            new cmajor::parsing::ActionParser(ToUtf32("A0"),
                new cmajor::parsing::EmptyParser()),
            new cmajor::parsing::ActionParser(ToUtf32("A1"),
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::ActionParser(ToUtf32("A2"),
                            new cmajor::parsing::NonterminalParser(ToUtf32("target"), ToUtf32("Expression"), 1)),
                        new cmajor::parsing::CharParser('=')),
                    new cmajor::parsing::NonterminalParser(ToUtf32("source"), ToUtf32("Expression"), 1))))));
    AddRule(new AssignmentStatementRule(ToUtf32("AssignmentStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::NonterminalParser(ToUtf32("AssignmentStatementExpr"), ToUtf32("AssignmentStatementExpr"), 1),
                new cmajor::parsing::CharParser(';')))));
    AddRule(new ConstructionStatementRule(ToUtf32("ConstructionStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::SequenceParser(
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::NonterminalParser(ToUtf32("TypeExpr"), ToUtf32("TypeExpr"), 1),
                    new cmajor::parsing::ActionParser(ToUtf32("A0"),
                        new cmajor::parsing::ExpectationParser(
                            new cmajor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0)))),
                new cmajor::parsing::AlternativeParser(
                    new cmajor::parsing::AlternativeParser(
                        new cmajor::parsing::SequenceParser(
                            new cmajor::parsing::CharParser('='),
                            new cmajor::parsing::ActionParser(ToUtf32("A1"),
                                new cmajor::parsing::ExpectationParser(
                                    new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1)))),
                        new cmajor::parsing::SequenceParser(
                            new cmajor::parsing::SequenceParser(
                                new cmajor::parsing::CharParser('('),
                                new cmajor::parsing::ExpectationParser(
                                    new cmajor::parsing::NonterminalParser(ToUtf32("ArgumentList"), ToUtf32("ArgumentList"), 2))),
                            new cmajor::parsing::ExpectationParser(
                                new cmajor::parsing::CharParser(')')))),
                    new cmajor::parsing::EmptyParser())),
            new cmajor::parsing::ActionParser(ToUtf32("A2"),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new DeleteStatementRule(ToUtf32("DeleteStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::KeywordParser(ToUtf32("delete")),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new DestroyStatementRule(ToUtf32("DestroyStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::KeywordParser(ToUtf32("destroy")),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new ExpressionStatementRule(ToUtf32("ExpressionStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::SequenceParser(
            new cmajor::parsing::ActionParser(ToUtf32("A0"),
                new cmajor::parsing::EmptyParser()),
            new cmajor::parsing::ActionParser(ToUtf32("A1"),
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::ActionParser(ToUtf32("A2"),
                        new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1)),
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new EmptyStatementRule(ToUtf32("EmptyStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::CharParser(';'))));
    AddRule(new ThrowStatementRule(ToUtf32("ThrowStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::KeywordParser(ToUtf32("throw")),
                    new cmajor::parsing::OptionalParser(
                        new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
    AddRule(new TryStatementRule(ToUtf32("TryStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::SequenceParser(
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::KeywordParser(ToUtf32("try")),
                new cmajor::parsing::ActionParser(ToUtf32("A0"),
                    new cmajor::parsing::NonterminalParser(ToUtf32("tryBlock"), ToUtf32("CompoundStatement"), 1))),
            new cmajor::parsing::PositiveParser(
                new cmajor::parsing::ActionParser(ToUtf32("A1"),
                    new cmajor::parsing::NonterminalParser(ToUtf32("Catch"), ToUtf32("Catch"), 1))))));
    AddRule(new CatchRule(ToUtf32("Catch"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::SequenceParser(
                            new cmajor::parsing::SequenceParser(
                                new cmajor::parsing::KeywordParser(ToUtf32("catch")),
                                new cmajor::parsing::ExpectationParser(
                                    new cmajor::parsing::CharParser('('))),
                            new cmajor::parsing::ExpectationParser(
                                new cmajor::parsing::NonterminalParser(ToUtf32("TypeExpr"), ToUtf32("TypeExpr"), 1))),
                        new cmajor::parsing::OptionalParser(
                            new cmajor::parsing::NonterminalParser(ToUtf32("Identifier"), ToUtf32("Identifier"), 0))),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::CharParser(')'))),
                new cmajor::parsing::NonterminalParser(ToUtf32("catchBlock"), ToUtf32("CompoundStatement"), 1)))));
    AddRule(new AssertStatementRule(ToUtf32("AssertStatement"), GetScope(), GetParsingDomain()->GetNextRuleId(),
        new cmajor::parsing::ActionParser(ToUtf32("A0"),
            new cmajor::parsing::SequenceParser(
                new cmajor::parsing::SequenceParser(
                    new cmajor::parsing::SequenceParser(
                        new cmajor::parsing::CharParser('#'),
                        new cmajor::parsing::KeywordParser(ToUtf32("assert"))),
                    new cmajor::parsing::ExpectationParser(
                        new cmajor::parsing::NonterminalParser(ToUtf32("Expression"), ToUtf32("Expression"), 1))),
                new cmajor::parsing::ExpectationParser(
                    new cmajor::parsing::CharParser(';'))))));
}

} } // namespace cmajor.parser
