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
        Equivalence(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
        Implication(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
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
        InvokeExpr(ParsingContext* ctx, var std::unique_ptr<Node> expr, var Span s): Node*;
    }
    grammar BasicTypeGrammar
    {
        BasicType: Node*;
    }
    grammar ConstantGrammar
    {
        Constant(ParsingContext* ctx): ConstantNode*;
    }
    grammar ClassGrammar
    {
        Class(ParsingContext* ctx): ClassNode*;
        InheritanceAndInterfaces(ParsingContext* ctx, ClassNode* classNode);
        BaseClassOrInterface(ParsingContext* ctx): Node*;
        ClassContent(ParsingContext* ctx, ClassNode* classNode);
        ClassMember(ParsingContext* ctx, ClassNode* classNode): Node*;
        StaticConstructor(ParsingContext* ctx, ClassNode* classNode, var std::unique_ptr<IdentifierNode> id): StaticConstructorNode*;
        Constructor(ParsingContext* ctx, ClassNode* classNode, var std::unique_ptr<IdentifierNode> id, var std::unique_ptr<ConstructorNode> ctor): Node*;
        Destructor(ParsingContext* ctx, ClassNode* classNode, var std::unique_ptr<IdentifierNode> id, var std::unique_ptr<DestructorNode> dtor): Node*;
        Initializer(ParsingContext* ctx): InitializerNode*;
        MemberFunction(ParsingContext* ctx, var std::unique_ptr<MemberFunctionNode> memFun, var std::unique_ptr<IdentifierNode> qid): Node*;
        ConversionFunction(ParsingContext* ctx, var std::unique_ptr<ConversionFunctionNode> conversionFun): Node*;
        MemberVariable(ParsingContext* ctx): Node*;
    }
    grammar DelegateGrammar
    {
        Delegate(ParsingContext* ctx): DelegateNode*;
        ClassDelegate(ParsingContext* ctx): ClassDelegateNode*;
    }
    grammar CompileUnitGrammar
    {
        CompileUnit(ParsingContext* ctx): CompileUnitNode*;
        NamespaceContent(ParsingContext* ctx, NamespaceNode* ns);
        UsingDirectives(ParsingContext* ctx, NamespaceNode* ns);
        UsingDirective(ParsingContext* ctx, NamespaceNode* ns);
        UsingAliasDirective(var std::unique_ptr<IdentifierNode> id): Node*;
        UsingNamespaceDirective: Node*;
        Definitions(ParsingContext* ctx, NamespaceNode* ns);
        Definition(ParsingContext* ctx, NamespaceNode* ns): Node*;
        NamespaceDefinition(ParsingContext* ctx, NamespaceNode* ns): NamespaceNode*;
        TypedefDeclaration(ParsingContext* ctx): TypedefNode*;
        ConceptDefinition(ParsingContext* ctx): ConceptNode*;
        FunctionDefinition(ParsingContext* ctx): FunctionNode*;
        ClassDefinition(ParsingContext* ctx): ClassNode*;
        InterfaceDefinition(ParsingContext* ctx): InterfaceNode*;
        EnumTypeDefinition(ParsingContext* ctx): EnumTypeNode*;
        ConstantDefinition(ParsingContext* ctx): ConstantNode*;
        DelegateDefinition(ParsingContext* ctx): DelegateNode*;
        ClassDelegateDefinition(ParsingContext* ctx): ClassDelegateNode*;
    }
    grammar ConceptGrammar
    {
        Concept(ParsingContext* ctx): ConceptNode*;
        Refinement: ConceptIdNode*;
        ConceptBody(ParsingContext* ctx, ConceptNode* concept);
        ConceptBodyConstraint(ParsingContext* ctx, ConceptNode* concept);
        TypeNameConstraint(ParsingContext* ctx): ConstraintNode*;
        SignatureConstraint(ParsingContext* ctx, IdentifierNode* firstTypeParameter): ConstraintNode*;
        ConstructorConstraint(ParsingContext* ctx, IdentifierNode* firstTypeParameter, var std::unique_ptr<IdentifierNode> id, var std::unique_ptr<ConstraintNode> ctorConstraint): ConstraintNode*;
        DestructorConstraint(ParsingContext* ctx, IdentifierNode* firstTypeParameter, var std::unique_ptr<IdentifierNode> id): ConstraintNode*;
        MemberFunctionConstraint(ParsingContext* ctx, var std::unique_ptr<Node> returnType, var std::unique_ptr<IdentifierNode> typeParam): ConstraintNode*;
        FunctionConstraint(ParsingContext* ctx): ConstraintNode*;
        EmbeddedConstraint(ParsingContext* ctx): ConstraintNode*;
        WhereConstraint(ParsingContext* ctx): WhereConstraintNode*;
        ConstraintExpr(ParsingContext* ctx): ConstraintNode*;
        DisjunctiveConstraintExpr(ParsingContext* ctx, var Span s): ConstraintNode*;
        ConjunctiveConstraintExpr(ParsingContext* ctx, var Span s): ConstraintNode*;
        PrimaryConstraintExpr(ParsingContext* ctx): ConstraintNode*;
        AtomicConstraintExpr(ParsingContext* ctx): ConstraintNode*;
        PredicateConstraint(ParsingContext* ctx): ConstraintNode*;
        IsConstraint(ParsingContext* ctx, var std::unique_ptr<Node> typeExpr): ConstraintNode*;
        ConceptOrTypeName(ParsingContext* ctx): Node*;
        MultiParamConstraint(ParsingContext* ctx, var std::unique_ptr<MultiParamConstraintNode> constraint): ConstraintNode*;
        Axiom(ParsingContext* ctx, ConceptNode* concept, var std::unique_ptr<AxiomNode> axiom);
        AxiomBody(ParsingContext* ctx, AxiomNode* axiom);
        AxiomStatement(ParsingContext* ctx): AxiomStatementNode*;
    }
    grammar SolutionGrammar
    {
        Solution: Solution*;
        Declaration: SolutionDeclaration*;
        SolutionProjectDeclaration: SolutionDeclaration*;
        ActiveProjectDeclaration: SolutionDeclaration*;
        FilePath: std::string;
    }
    grammar EnumerationGrammar
    {
        EnumType(ParsingContext* ctx): EnumTypeNode*;
        UnderlyingType(ParsingContext* ctx): Node*;
        EnumConstants(ParsingContext* ctx, EnumTypeNode* enumType);
        EnumConstant(ParsingContext* ctx, EnumTypeNode* enumType, var Span s): EnumConstantNode*;
    }
    grammar IdentifierGrammar
    {
        Identifier: IdentifierNode*;
        QualifiedId: IdentifierNode*;
    }
    grammar InterfaceGrammar
    {
        Interface(ParsingContext* ctx): InterfaceNode*;
        InterfaceContent(ParsingContext* ctx, InterfaceNode* intf);
        InterfaceMemFun(ParsingContext* ctx, var std::unique_ptr<MemberFunctionNode> memFun): Node*;
        InterfaceFunctionGroupId(var std::unique_ptr<IdentifierNode> id): std::u32string;
    }
    grammar KeywordGrammar
    {
        Keyword;
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
    grammar ParameterGrammar
    {
        ParameterList(ParsingContext* ctx, Node* owner);
        Parameter(ParsingContext* ctx): ParameterNode*;
    }
    grammar ProjectGrammar
    {
        Project(std::string config): Project*;
        Declaration: ProjectDeclaration*;
        ReferenceDeclaration: ProjectDeclaration*;
        SourceFileDeclaration: ProjectDeclaration*;
        TextFileDeclaration: ProjectDeclaration*;
        TargetDeclaration: ProjectDeclaration*;
        Target: Target;
        FilePath: std::string;
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
        ExpressionStatement(ParsingContext* ctx, var std::unique_ptr<Node> expr): StatementNode*;
        EmptyStatement(ParsingContext* ctx): StatementNode*;
        ThrowStatement(ParsingContext* ctx): StatementNode*;
        TryStatement(ParsingContext* ctx): TryStatementNode*;
        Catch(ParsingContext* ctx): CatchNode*;
        AssertStatement(ParsingContext* ctx): StatementNode*;
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
