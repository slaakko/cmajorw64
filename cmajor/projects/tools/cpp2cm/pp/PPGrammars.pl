namespace pp
{
    grammar PPFileGrammar
    {
        PPFile(PP* pp, ustring* result);
        Group(PP* pp, bool process, ustring* result);
        IfSection(PP* pp, bool parentProcess, ustring* result, var bool processed);
        IfGroup(PP* pp, bool parentProcess, ustring* result, var bool process) : bool;
        ElifGroups(PP* pp, bool parentProcess, bool* processed, ustring* result);
        ElifGroup(PP* pp, bool parentProcess, bool* processed, ustring* result, var bool process);
        ElseGroup(PP* pp, bool parentProcess, bool* processed, ustring* result);
        EndIfLine(PP* pp, ustring* result);
        ControlLine(PP* pp, bool process, ustring* result);
        IncludeDirective(PP* pp, bool process, ustring* result, var ustring headerNameText);
        DefineFunctionMacro(PP* pp, bool process, ustring* result, var ustring replacementList);
        MacroParams : List<ustring>;
        DefineObjectMacro(PP* pp, bool process, ustring* result, var ustring replacementList);
        UndefineMacro(PP* pp, bool process, ustring* result);
        ConstantExpression(bool process, PP* pp, var ustring constantExpressionText) : bool;
        TextLine(PP* pp, bool process, ustring* result, var ustring replacementText);
        PossiblyEmptyTokenLine(bool process, ustring* result, PP* pp, PPTokenFlags flags, var bool substituted);
        NonemptyTokenLine(bool process, ustring* result, PP* pp, PPTokenFlags flags, var bool substituted);
        PPToken(bool process, ustring* result, PP* pp, PPTokenFlags flags, bool* substituted);
        FunctionMacroInvocation(bool process, PP* pp, PPTokenFlags flags, ustring* result, var FunctionMacro* macro);
        MacroArgumentList(bool process, PP* pp, PPTokenFlags flags) : List<ustring>;
        MacroArgument(bool process, PP* pp, PPTokenFlags flags, var bool substituted) : ustring;
        MacroReplacement(FunctionMacro* macro, List<ustring>* args, PP* pp, PPTokenFlags flags, var bool substituted) : ustring;
        PPNumber : ustring;
        HeaderName : ustring;
        Spaces;
    }
    grammar PPPossiblyEmptyTokenLineGrammar
    {
    }
    grammar PPMacroReplacementGrammar
    {
    }
    grammar PPNonemptyTokenLineGrammar
    {
    }
}
