namespace cppparser
{
    grammar DeclarationGrammar
    {
        Declarations(ParsingContext* ctx);
        Declaration(ParsingContext* ctx);
        NamespaceDefinition(ParsingContext* ctx);
        NamedNamespaceDefinition(ParsingContext* ctx);
        UnnamedNamespaceDefinition(ParsingContext* ctx);
        BlockDeclaration(ParsingContext* ctx);
        SimpleDeclaration(ParsingContext* ctx);
        DeclSpecifiers;
        DeclSpecifier;
        StorageClassSpecifier;
        FunctionSpecifier;
        AliasDeclaration(ParsingContext* ctx);
        UsingDirective(ParsingContext* ctx);
        UsingDeclaration(ParsingContext* ctx);
        TypedefDeclaration(ParsingContext* ctx);
        LinkageSpecification(ParsingContext* ctx);
    }
    grammar SimpleTypeGrammar
    {
        SimpleType;
        SimpleTypeSpecifier;
    }
    grammar EnumerationGrammar
    {
        EnumDeclaration(ParsingContext* ctx);
        OpaqueEnumDeclaration(ParsingContext* ctx);
        EnumSpecifier(ParsingContext* ctx);
        EnumHead(ParsingContext* ctx);
        EnumKey;
        EnumName(ParsingContext* ctx);
        EnumBase(ParsingContext* ctx);
        Enumerators(ParsingContext* ctx);
        EnumeratorDefinition(ParsingContext* ctx);
        Enumerator;
    }
    grammar FunctionGrammar
    {
        FunctionDefinition(ParsingContext* ctx);
        ParameterList(ParsingContext* ctx);
        ParameterDeclaration(ParsingContext* ctx);
        FunctionBody(ParsingContext* ctx);
    }
    grammar SourceFileGrammar
    {
        SourceFile(var ParsingContext ctx);
        CommentsAndSpacesAndPPLines;
        PPLine;
        S;
    }
    grammar IdentifierGrammar
    {
        Identifier;
        Id(ParsingContext* ctx);
        NestedNameSpecifier(ParsingContext* ctx);
        QualifiedId(ParsingContext* ctx);
    }
    grammar TypeExprGrammar
    {
        TypeExpr(ParsingContext* ctx);
        PrefixTypeExpr(ParsingContext* ctx);
        CVSpecifierSequence;
        CVSpecifier;
        PostfixTypeExpr(ParsingContext* ctx);
        PrimaryTypeExpr(ParsingContext* ctx);
    }
    grammar TemplateGrammar
    {
        TemplateDeclaration(ParsingContext* ctx);
        TemplateParameterList(ParsingContext* ctx);
        TemplateParameter(ParsingContext* ctx);
        TypeParameter(ParsingContext* ctx);
        SimpleTemplateId(ParsingContext* ctx);
        TemplateId(ParsingContext* ctx);
        TemplateName;
        TemplateArgumentList(ParsingContext* ctx);
        TemplateArgument(ParsingContext* ctx);
        ExplicitInstantiation(ParsingContext* ctx);
        ExplicitSpecialization(ParsingContext* ctx);
    }
    grammar DeclaratorGrammar
    {
        InitDeclarator(ParsingContext* ctx);
        Declarator(ParsingContext* ctx) : bool;
        Initializer(ParsingContext* ctx);
        BraceOrEqualInitializer(ParsingContext* ctx);
        BracedInitializerList(ParsingContext* ctx);
        InitializerClause(ParsingContext* ctx);
        InitializerList(ParsingContext* ctx);
    }
    grammar ClassGrammar
    {
        ClassDeclaration(ParsingContext* ctx);
        ForwardClassDeclaration(ParsingContext* ctx);
        ClassSpecifier(ParsingContext* ctx);
        MemberSpecifications(ParsingContext* ctx);
        MemberSpecification(ParsingContext* ctx);
        MemberDeclaration(ParsingContext* ctx);
        SpecialMemberFunctionDeclaration(ParsingContext* ctx);
        ClassHead(ParsingContext* ctx);
        ClassKey;
        ClassName(ParsingContext* ctx);
        ClassVirtSpecifiers;
        ClassVirtSpecifier;
        BaseClause(ParsingContext* ctx);
        BaseClassSpecifierList(ParsingContext* ctx);
        BaseClassSpecifier(ParsingContext* ctx);
        BaseSpecifiers;
        BaseSpecifier;
        AccessSpecifier;
        VirtPureSpecifiers(bool functionMember) : bool;
        VirtSpecifier(bool functionMember);
        PureSpecifier(bool functionMember);
        SpecialMemberFunctionDefinition(ParsingContext* ctx);
        CtorInitializer(ParsingContext* ctx);
        MemberInitializerList(ParsingContext* ctx);
        MemberInitializer(ParsingContext* ctx);
        MemberInitializerId(ParsingContext* ctx);
    }
    grammar LiteralGrammar
    {
        Literal;
        FloatingLiteral;
        FractionalConstant;
        ExponentPart;
        FloatingSuffix;
        IntegerLiteral;
        DecimalLiteral;
        OctalLiteral;
        HexadecimalLiteral;
        IntegerSuffix;
        CharacterLiteral;
        CChar;
        HexDigit4;
        HexDigit8;
        StringLiteral;
        EncodingPrefix;
        SChar;
        BooleanLiteral;
        PointerLiteral;
    }
    grammar ExpressionGrammar
    {
        ExpressionList(ParsingContext* ctx);
        PossiblyEmptyArgumentList(ParsingContext* ctx);
        Expression(ParsingContext* ctx);
        ConstantExpression(ParsingContext* ctx);
        AssignmentExpression(ParsingContext* ctx);
        ConcreteAssignmentExpression(ParsingContext* ctx);
        AssignmentOperator;
        ConditionalExpression(ParsingContext* ctx);
        ThrowExpression(ParsingContext* ctx);
        LogicalOrExpression(ParsingContext* ctx);
        LogicalAndExpression(ParsingContext* ctx);
        InclusiveOrExpression(ParsingContext* ctx);
        ExclusiveOrExpression(ParsingContext* ctx);
        AndExpression(ParsingContext* ctx);
        EqualityExpression(ParsingContext* ctx);
        RelationalExpression(ParsingContext* ctx);
        ShiftExpression(ParsingContext* ctx);
        AdditiveExpression(ParsingContext* ctx);
        MultiplicativeExpression(ParsingContext* ctx);
        PMExpression(ParsingContext* ctx);
        CastExpression(ParsingContext* ctx);
        UnaryExpression(ParsingContext* ctx);
        UnaryOperator;
        NewExpression(ParsingContext* ctx);
        NewPlacement(ParsingContext* ctx);
        NewInitializer(ParsingContext* ctx);
        DeleteExpression(ParsingContext* ctx);
        PostfixExpression(ParsingContext* ctx);
        PrimaryExpression(ParsingContext* ctx);
        CppCastExpression(ParsingContext* ctx);
        TypeIdExpression(ParsingContext* ctx);
        IdExpression(ParsingContext* ctx);
        UnqualifiedIdExpr(ParsingContext* ctx);
        QualifiedIdExpr(ParsingContext* ctx);
        OperatorFunctionId;
        Operator;
        ConversionFunctionId(ParsingContext* ctx);
    }
    grammar KeywordGrammar
    {
        Keyword;
    }
    grammar StatementGrammar
    {
        Statement(ParsingContext* ctx);
        LabeledStatement(ParsingContext* ctx);
        ExpressionStatement(ParsingContext* ctx);
        CompoundStatement(ParsingContext* ctx);
        SelectionStatement(ParsingContext* ctx);
        Condition(ParsingContext* ctx);
        IterationStatement(ParsingContext* ctx);
        ForInitStatement(ParsingContext* ctx);
        ForRangeDeclaration(ParsingContext* ctx);
        ForRangeInitializer(ParsingContext* ctx);
        JumpStatement(ParsingContext* ctx);
        DeclarationStatement(ParsingContext* ctx);
        TryBlock(ParsingContext* ctx);
        HandlerSequence(ParsingContext* ctx);
        Handler(ParsingContext* ctx);
        ExceptionDeclaration(ParsingContext* ctx);
    }
}
