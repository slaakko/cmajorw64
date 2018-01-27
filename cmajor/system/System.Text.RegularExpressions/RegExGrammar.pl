namespace System.Text.RegularExpressions
{
    grammar RegularExpressionGrammar
    {
        RegularExpression : PtrNfa;
        AlternativeExpression : PtrNfa;
        SequenceExpression : PtrNfa;
        PostfixExpression : PtrNfa;
        PrimaryExpression : PtrNfa;
        Char : uchar;
        CharClass(var bool inverse, var ustring s) : CharClass;
        Class : Class;
        CharRange : ustring;
        CharClassChar : uchar;
    }
}
