namespace cmparser
{
    grammar TypeExprGrammar
    {
        TypeExpr : Node*;
        PrefixTypeExpr : Node*;
        PostfixTypeExpr(var UniquePtr<Node> node) : Node*;
        PrimaryTypeExpr : Node*;
    }
    grammar TemplateGrammar
    {
        TemplateId(var UniquePtr<TemplateIdNode> templateId) : TemplateIdNode*;
    }
    grammar IdentifierGrammar
    {
        Identifier : IdentifierNode*;
        QualifiedId(var UniquePtr<Node> node) : Node*;
    }
    grammar KeywordGrammar
    {
        Keyword;
    }
    grammar BasicTypeGrammar
    {
        BasicType : Node*;
    }
}
