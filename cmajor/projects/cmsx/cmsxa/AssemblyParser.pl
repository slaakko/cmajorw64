namespace cmsx.assembly
{
    grammar AssemblyGrammar
    {
        DecimalConstant : DecimalConstant*;
        HexConstant : HexConstant*;
        CharacterConstant : CharacterConstant*;
        StringConstant : StringConstant*;
        Constant : Constant*;
        Symbol : Symbol*;
        LocalOperand(var uchar d) : LocalOperand*;
        LocalLabel(var uchar d) : LocalLabel*;
        At : At*;
        PrimaryExpression : Node*;
        UnaryOperator : Operator;
        Term : Node*;
        StrongOperator : Operator;
        Expression : Node*;
        WeakOperator : Operator;
        Instruction : Instruction*;
        InstructionLine(List<UniquePtr<Instruction>>* instructionList);
        Label : Node*;
        OpCode : OpCode*;
        Operands : OperandList*;
        S;
        SNL(Node* label);
        NL;
        Comment;
        CommentLine;
        EmptyLine;
        ModeLine(List<UniquePtr<Instruction>>* instructionList);
        AssemblyLine(List<UniquePtr<Instruction>>* instructionList);
        AssemblyFile(List<UniquePtr<Instruction>>* instructionList);
    }
}
