namespace cmajor.parser
{
    grammar FunctionGrammar
    {
        Function(ParsingContext* ctx, var std::unique_ptr<FunctionNode> fun, var Span s): FunctionNode*;
        FunctionGroupId(ParsingContext* ctx, var std::unique_ptr<IdentifierNode> id): std::u32string;
        OperatorFunctionGroupId(ParsingContext* ctx, var std::unique_ptr<Node> typeExpr): std::u32string;
    }
    grammar ExpressionGrammar
    {
        Expression(ParsingContext* ctx): Node*;
        Disjunction(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        Conjunction(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        BitOr(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        BitXor(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        BitAnd(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        Equality(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s, var Operator op): Node*;
        Relational(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s, var Operator op): Node*;
        Shift(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s, var Operator op): Node*;
        Additive(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s, var Operator op): Node*;
        Multiplicative(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s, var Operator op): Node*;
        Prefix(ParsingContext* ctx, var Span s, var Operator op): Node*;
        Postfix(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        Primary(ParsingContext* ctx): Node*;
        SizeOfExpr(ParsingContext* ctx): Node*;
        TypeNameExpr(ParsingContext* ctx): Node*;
        CastExpr(ParsingContext* ctx): Node*;
        ConstructExpr(ParsingContext* ctx): Node*;
        NewExpr(ParsingContext* ctx): Node*;
        ArgumentList(ParsingContext* ctx, Node* node);
        ExpressionList(ParsingContext* ctx, Node* node);
    }
    grammar BasicTypeGrammar
    {
        BasicType: Node*;
    }
    grammar ConstantGrammar
    {
        Constant(ParsingContext* ctx): Node*;
    }
    grammar DelegateGrammar
    {
        Delegate(ParsingContext* ctx): DelegateNode*;
        ClassDelegate(ParsingContext* ctx): ClassDelegateNode*;
    }
    grammar KeywordGrammar
    {
        Keyword;
    }
    grammar EnumerationGrammar
    {
        EnumType(ParsingContext* ctx): EnumTypeNode*;
        UnderlyingType(ParsingContext* ctx): Node*;
        EnumConstants(ParsingContext* ctx, EnumTypeNode* enumType);
        EnumConstant(ParsingContext* ctx, EnumTypeNode* enumType, var Span s): EnumConstantNode*;
    }
    grammar LiteralGrammar
    {
        Literal: Node*;
        BooleanLiteral: Node*;
        FloatingLiteral(var Span s): Node*;
        FloatingLiteralValue: double;
        FractionalFloatingLiteral;
        ExponentFloatingLiteral;
        ExponentPart;
        IntegerLiteral(var Span s): Node*;
        IntegerLiteralValue: uint64_t;
        HexIntegerLiteral: uint64_t;
        DecIntegerLiteral: uint64_t;
        CharLiteral(var char32_t litValue): Node*;
        StringLiteral(var std::u32string s): Node*;
        NullLiteral: Node*;
        CharEscape: char32_t;
        DecDigitSequence: uint64_t;
        HexDigitSequence: uint64_t;
        HexDigit4: uint16_t;
        HexDigit8: uint32_t;
        OctalDigitSequence: uint64_t;
        Sign;
    }
    grammar IdentifierGrammar
    {
        Identifier: IdentifierNode*;
        QualifiedId: IdentifierNode*;
    }
    grammar ParameterGrammar
    {
        ParameterList(ParsingContext* ctx, Node* owner);
        Parameter(ParsingContext* ctx): ParameterNode*;
    }
    grammar SpecifierGrammar
    {
        Specifiers: Specifiers;
        Specifier: Specifiers;
    }
    grammar StatementGrammar
    {
        Statement(ParsingContext* ctx): StatementNode*;
        LabelId: std::u32string;
        Label(var std::u32string label): LabelNode*;
        LabeledStatement(ParsingContext* ctx): StatementNode*;
        ControlStatement(ParsingContext* ctx): StatementNode*;
        CompoundStatement(ParsingContext* ctx): CompoundStatementNode*;
        ReturnStatement(ParsingContext* ctx): StatementNode*;
        IfStatement(ParsingContext* ctx): StatementNode*;
        WhileStatement(ParsingContext* ctx): StatementNode*;
        DoStatement(ParsingContext* ctx): StatementNode*;
        ForStatement(ParsingContext* ctx): StatementNode*;
        ForInitStatement(ParsingContext* ctx): StatementNode*;
        ForLoopStatementExpr(ParsingContext* ctx): StatementNode*;
        RangeForStatement(ParsingContext* ctx): StatementNode*;
        BreakStatement(ParsingContext* ctx): StatementNode*;
        ContinueStatement(ParsingContext* ctx): StatementNode*;
        GotoStatement(ParsingContext* ctx): StatementNode*;
        SwitchStatement(ParsingContext* ctx): SwitchStatementNode*;
        CaseStatement(ParsingContext* ctx, var std::unique_ptr<CaseStatementNode> caseS): CaseStatementNode*;
        DefaultStatement(ParsingContext* ctx): DefaultStatementNode*;
        GotoCaseStatement(ParsingContext* ctx): StatementNode*;
        GotoDefaultStatement(ParsingContext* ctx): StatementNode*;
        AssignmentStatementExpr(ParsingContext* ctx, var std::unique_ptr<Node> targetExpr): StatementNode*;
        AssignmentStatement(ParsingContext* ctx): StatementNode*;
        ConstructionStatement(ParsingContext* ctx): ConstructionStatementNode*;
        DeleteStatement(ParsingContext* ctx): StatementNode*;
        DestroyStatement(ParsingContext* ctx): StatementNode*;
        IncrementStatementExpr(ParsingContext* ctx, var std::unique_ptr<Node> expr): StatementNode*;
        IncrementStatement(ParsingContext* ctx): StatementNode*;
        DecrementStatementExpr(ParsingContext* ctx, var std::unique_ptr<Node> expr): StatementNode*;
        DecrementStatement(ParsingContext* ctx): StatementNode*;
        ExpressionStatement(ParsingContext* ctx, var std::unique_ptr<Node> expr): StatementNode*;
        EmptyStatement(ParsingContext* ctx): StatementNode*;
        ThrowStatement(ParsingContext* ctx): StatementNode*;
        TryStatement(ParsingContext* ctx): TryStatementNode*;
        Catch(ParsingContext* ctx): CatchNode*;
    }
    grammar TemplateGrammar
    {
        TemplateId(ParsingContext* ctx, var std::unique_ptr<TemplateIdNode> templateId): Node*;
        TemplateParameter(ParsingContext* ctx): TemplateParameterNode*;
        TemplateParameterList(ParsingContext* ctx, Node* owner);
    }
    grammar TypedefGrammar
    {
        Typedef(ParsingContext* ctx): TypedefNode*;
    }
    grammar TypeExprGrammar
    {
        TypeExpr(ParsingContext* ctx): Node*;
        PrefixTypeExpr(ParsingContext* ctx): Node*;
        PostfixTypeExpr(ParsingContext* ctx, var std::unique_ptr<Node> typeExpr, var Span s): Node*;
        PrimaryTypeExpr(ParsingContext* ctx): Node*;
    }
}
