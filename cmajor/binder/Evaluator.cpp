// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/Evaluator.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/TypeBinder.hpp>
#include <cmajor/binder/TypeResolver.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/ast/Visitor.hpp>
#include <cmajor/ast/BasicType.hpp>
#include <cmajor/ast/Literal.hpp>
#include <cmajor/ast/Expression.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::unicode;

void ThrowCannotEvaluateStatically(const Span& span)
{
    throw Exception("cannot evaluate statically", span);
}

typedef Value* (*BinaryOperatorFun)(Value* left, Value* right, const Span& span, bool dontThrow);
typedef Value* (*UnaryOperatorFun)(Value* operand, const Span& span, bool dontThrow);

class ScopedValue : public Value
{
public:
    ScopedValue(const Span& span_, ContainerSymbol* containerSymbol_);
    bool IsComplete() const override { return false; }
    bool IsScopedValue() const { return true; }
    const ContainerSymbol* GetContainerSymbol() const { return containerSymbol; }
    ContainerSymbol* GetContainerSymbol() { return containerSymbol; }
    Value* Clone() override { Assert(false, "scoped value cannot be cloned"); return nullptr; }
    Value* As(ValueType targetType, bool cast, const Span& span, bool dontThrow) const override { Assert(false, "scoped value cannot be converted"); return nullptr; }
    llvm::Value* IrValue(Emitter& emitter) override { Assert(false, "scoped value does not have ir value"); return nullptr; }
private:
    ContainerSymbol* containerSymbol;
};

ScopedValue::ScopedValue(const Span& span_, ContainerSymbol* containerSymbol_) : Value(span_, ValueType::none), containerSymbol(containerSymbol_)
{
}

template <typename ValueT, typename Op>
Value* BinaryEvaluate(Value* left, Value* right, Op op, const Span& span)
{
    ValueT* leftCasted = static_cast<ValueT*>(left);
    ValueT* rightCasted = static_cast<ValueT*>(right);
    return new ValueT(span, op(leftCasted->GetValue(), rightCasted->GetValue()));
}

template <typename ValueT, typename Op>
Value* BinaryPredEvaluate(Value* left, Value* right, Op op, const Span& span)
{
    ValueT* leftCasted = static_cast<ValueT*>(left);
    ValueT* rightCasted = static_cast<ValueT*>(right);
    return new BoolValue(span, op(leftCasted->GetValue(), rightCasted->GetValue()));
}

template<typename ValueT, typename Op>
Value* UnaryEvaluate(Value* subject, Op op, const Span& span)
{
    ValueT* subjectCasted = static_cast<ValueT*>(subject);
    return new ValueT(span, op(subjectCasted->GetValue()));
}

Value* NotSupported(Value* subject, const Span& span, bool dontThrow)
{
    if (dontThrow)
    {
        return nullptr;
    }
    throw Exception("operation not supported for type " + ValueTypeStr(subject->GetValueType()), span);
}

Value* NotSupported(Value* left, Value* right, const Span& span, bool dontThrow)
{
    if (dontThrow)
    {
        return nullptr;
    }
    throw Exception("operation not supported for types " + ValueTypeStr(left->GetValueType()) + " and " + ValueTypeStr(right->GetValueType()), span);
}

template<typename ValueT>
Value* Disjunction(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, std::logical_or<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun disjunction[uint8_t(ValueType::maxValue)] =
{
    NotSupported, Disjunction<BoolValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, 
    NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* Conjunction(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, std::logical_and<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun conjunction[uint8_t(ValueType::maxValue)] =
{
    NotSupported, Conjunction<BoolValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported,
    NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* BitOr(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, std::bit_or<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun bitOr[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, BitOr<SByteValue>, BitOr<ByteValue>, BitOr<ShortValue>, BitOr<UShortValue>, BitOr<IntValue>, BitOr<UIntValue>,
    BitOr<LongValue>, BitOr<ULongValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* BitXor(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, std::bit_xor<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun bitXor[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, BitXor<SByteValue>, BitXor<ByteValue>, BitXor<ShortValue>, BitXor<UShortValue>, BitXor<IntValue>, BitXor<UIntValue>,
    BitXor<LongValue>, BitXor<ULongValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* BitAnd(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, std::bit_and<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun bitAnd[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, BitAnd<SByteValue>, BitAnd<ByteValue>, BitAnd<ShortValue>, BitAnd<UShortValue>, BitAnd<IntValue>, BitAnd<UIntValue>,
    BitAnd<LongValue>, BitAnd<ULongValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* Equal(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::equal_to<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun equal[uint8_t(ValueType::maxValue)] =
{
    NotSupported, Equal<BoolValue>, Equal<SByteValue>, Equal<ByteValue>, Equal<ShortValue>, Equal<UShortValue>, Equal<IntValue>, Equal<UIntValue>,
    Equal<LongValue>, Equal<ULongValue>, Equal<FloatValue>, Equal<DoubleValue>, Equal<CharValue>, Equal<WCharValue>, Equal<UCharValue>, NotSupported, NotSupported
};

template<typename ValueT>
Value* NotEqual(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::not_equal_to<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun notEqual[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotEqual<BoolValue>, NotEqual<SByteValue>, NotEqual<ByteValue>, NotEqual<ShortValue>, NotEqual<UShortValue>, NotEqual<IntValue>, NotEqual<UIntValue>,
    NotEqual<LongValue>, NotEqual<ULongValue>, NotEqual<FloatValue>, NotEqual<DoubleValue>, NotEqual<CharValue>, NotEqual<WCharValue>, NotEqual<UCharValue>, NotSupported, NotSupported
};

template<typename ValueT>
Value* Less(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::less<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun less[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, Less<SByteValue>, Less<ByteValue>, Less<ShortValue>, Less<UShortValue>, Less<IntValue>, Less<UIntValue>,
    Less<LongValue>, Less<ULongValue>, Less<FloatValue>, Less<DoubleValue>, Less<CharValue>, Less<WCharValue>, Less<UCharValue>, NotSupported, NotSupported
};

template<typename ValueT>
Value* Greater(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::greater<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun greater[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, Greater<SByteValue>, Greater<ByteValue>, Greater<ShortValue>, Greater<UShortValue>, Greater<IntValue>, Greater<UIntValue>,
    Greater<LongValue>, Greater<ULongValue>, Greater<FloatValue>, Greater<DoubleValue>, Greater<CharValue>, Greater<WCharValue>, Greater<UCharValue>, NotSupported, NotSupported
};

template<typename ValueT>
Value* LessEqual(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::less_equal<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun lessEqual[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, LessEqual<SByteValue>, LessEqual<ByteValue>, LessEqual<ShortValue>, LessEqual<UShortValue>, LessEqual<IntValue>, LessEqual<UIntValue>,
    LessEqual<LongValue>, LessEqual<ULongValue>, LessEqual<FloatValue>, LessEqual<DoubleValue>, LessEqual<CharValue>, LessEqual<WCharValue>, LessEqual<UCharValue>, NotSupported, NotSupported
};

template<typename ValueT>
Value* GreaterEqual(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryPredEvaluate<ValueT>(left, right, std::greater_equal<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun greaterEqual[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, GreaterEqual<SByteValue>, GreaterEqual<ByteValue>, GreaterEqual<ShortValue>, GreaterEqual<UShortValue>, GreaterEqual<IntValue>, GreaterEqual<UIntValue>,
    GreaterEqual<LongValue>, GreaterEqual<ULongValue>, GreaterEqual<FloatValue>, GreaterEqual<DoubleValue>, GreaterEqual<CharValue>, GreaterEqual<WCharValue>, GreaterEqual<UCharValue>, NotSupported, NotSupported
};

template<typename T>
struct shiftLeftFun : std::binary_function<T, T, T>
{
    T operator()(const T& left, const T& right) const
    {
        return left << right;
    }
};

template<typename ValueT>
Value* ShiftLeft(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, shiftLeftFun<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun shiftLeft[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, ShiftLeft<SByteValue>, ShiftLeft<ByteValue>, ShiftLeft<ShortValue>, ShiftLeft<UShortValue>, ShiftLeft<IntValue>, ShiftLeft<UIntValue>,
    ShiftLeft<LongValue>, ShiftLeft<ULongValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename T>
struct shiftRightFun : std::binary_function<T, T, T>
{
    T operator()(const T& left, const T& right) const
    {
        return left >> right;
    }
};

template<typename ValueT>
Value* ShiftRight(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, shiftRightFun<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun shiftRight[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, ShiftRight<SByteValue>, ShiftRight<ByteValue>, ShiftRight<ShortValue>, ShiftRight<UShortValue>, ShiftRight<IntValue>, ShiftRight<UIntValue>,
    ShiftRight<LongValue>, ShiftRight<ULongValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* Add(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, std::plus<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun add[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, Add<SByteValue>, Add<ByteValue>, Add<ShortValue>, Add<UShortValue>, Add<IntValue>, Add<UIntValue>,
    Add<LongValue>, Add<ULongValue>, Add<FloatValue>, Add<DoubleValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* Sub(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, std::minus<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun sub[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, Sub<SByteValue>, Sub<ByteValue>, Sub<ShortValue>, Sub<UShortValue>, Sub<IntValue>, Sub<UIntValue>,
    Sub<LongValue>, Sub<ULongValue>, Sub<FloatValue>, Sub<DoubleValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* Mul(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, std::multiplies<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun mul[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, Mul<SByteValue>, Mul<ByteValue>, Mul<ShortValue>, Mul<UShortValue>, Mul<IntValue>, Mul<UIntValue>,
    Mul<LongValue>, Mul<ULongValue>, Mul<FloatValue>, Mul<DoubleValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* Div(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, std::divides<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun div[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, Div<SByteValue>, Div<ByteValue>, Div<ShortValue>, Div<UShortValue>, Div<IntValue>, Div<UIntValue>,
    Div<LongValue>, Div<ULongValue>, Div<FloatValue>, Div<DoubleValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* Rem(Value* left, Value* right, const Span& span, bool dontThrow)
{
    return BinaryEvaluate<ValueT>(left, right, std::modulus<typename ValueT::OperandType>(), span);
}

BinaryOperatorFun rem[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, Rem<SByteValue>, Rem<ByteValue>, Rem<ShortValue>, Rem<UShortValue>, Rem<IntValue>, Rem<UIntValue>,
    Rem<LongValue>, Rem<ULongValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* Not(Value* subject, const Span& span, bool dontThrow)
{
    return UnaryEvaluate<ValueT>(subject, std::logical_not<typename ValueT::OperandType>(), span);
}

UnaryOperatorFun logicalNot[uint8_t(ValueType::maxValue)] =
{
    NotSupported, Not<BoolValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported,
    NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* UnaryPlus(Value* subject, const Span& span, bool dontThrow)
{
    return UnaryEvaluate<ValueT>(subject, std::identity<typename ValueT::OperandType>(), span);
}

UnaryOperatorFun unaryPlus[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, UnaryPlus<SByteValue>, UnaryPlus<ByteValue>, UnaryPlus<ShortValue>, UnaryPlus<UShortValue>, UnaryPlus<IntValue>, UnaryPlus<UIntValue>,
    UnaryPlus<LongValue>, UnaryPlus<ULongValue>, UnaryPlus<FloatValue>, UnaryPlus<DoubleValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* UnaryMinus(Value* subject, const Span& span, bool dontThrow)
{
    return UnaryEvaluate<ValueT>(subject, std::negate<typename ValueT::OperandType>(), span);
}

UnaryOperatorFun unaryMinus[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, UnaryMinus<SByteValue>, UnaryMinus<ByteValue>, UnaryMinus<ShortValue>, UnaryMinus<UShortValue>, UnaryMinus<IntValue>, UnaryMinus<UIntValue>,
    UnaryMinus<LongValue>, UnaryMinus<ULongValue>, UnaryMinus<FloatValue>, UnaryMinus<DoubleValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

template<typename ValueT>
Value* Complement(Value* subject, const Span& span, bool dontThrow)
{
    return UnaryEvaluate<ValueT>(subject, std::bit_not<typename ValueT::OperandType>(), span);
}

UnaryOperatorFun complement[uint8_t(ValueType::maxValue)] =
{
    NotSupported, NotSupported, Complement<SByteValue>, Complement<ByteValue>, Complement<ShortValue>, Complement<UShortValue>, Complement<IntValue>, Complement<UIntValue>,
    Complement<LongValue>, Complement<ULongValue>, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported, NotSupported
};

class Evaluator : public Visitor
{
public:
    Evaluator(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_, ValueType targetType_, bool dontThrow_);
    bool Error() const { return error; }
    std::unique_ptr<Value> GetValue();
    void Visit(BoolNode& boolNode) override;
    void Visit(SByteNode& sbyteNode) override;
    void Visit(ByteNode& byteNode) override;
    void Visit(ShortNode& shortNode) override;
    void Visit(UShortNode& ushortNode) override;
    void Visit(IntNode& intNode) override;
    void Visit(UIntNode& uintNode) override;
    void Visit(LongNode& longNode) override;
    void Visit(ULongNode& ulongNode) override;
    void Visit(FloatNode& floatNode) override;
    void Visit(DoubleNode& doubleNode) override;
    void Visit(CharNode& charNode) override;
    void Visit(WCharNode& wcharNode) override;
    void Visit(UCharNode& ucharNode) override;
    void Visit(VoidNode& voidNode) override;

    void Visit(BooleanLiteralNode& booleanLiteralNode) override;
    void Visit(SByteLiteralNode& sbyteLiteralBode) override;
    void Visit(ByteLiteralNode& byteLiteralNode) override;
    void Visit(ShortLiteralNode& shortLiteralNode) override;
    void Visit(UShortLiteralNode& ushortLiteralNode) override;
    void Visit(IntLiteralNode& intLiteralNode) override;
    void Visit(UIntLiteralNode& uintLiteralNode) override;
    void Visit(LongLiteralNode& longLiteralNode) override;
    void Visit(ULongLiteralNode& ulongLiteralNode) override;
    void Visit(FloatLiteralNode& floatLiteralNode) override;
    void Visit(DoubleLiteralNode& doubleLiteralNode) override;
    void Visit(CharLiteralNode& charLiteralNode) override;
    void Visit(WCharLiteralNode& wcharLiteralNode) override;
    void Visit(UCharLiteralNode& ucharLiteralNode) override;
    void Visit(StringLiteralNode& stringLiteralNode) override;
    void Visit(WStringLiteralNode& wstringLiteralNode) override;
    void Visit(UStringLiteralNode& ustringLiteralNode) override;
    void Visit(NullLiteralNode& nullLiteralNode) override;

    void Visit(IdentifierNode& identifierNode) override;
    void Visit(TemplateIdNode& templateIdNode) override;

    void Visit(DotNode& dotNode) override;
    void Visit(ArrowNode& arrowNode) override;
    void Visit(EquivalenceNode& equivalenceNode) override;
    void Visit(ImplicationNode& implicationNode) override;
    void Visit(DisjunctionNode& disjunctionNode) override;
    void Visit(ConjunctionNode& conjunctionNode) override;
    void Visit(BitOrNode& bitOrNode) override;
    void Visit(BitXorNode& bitXorNode) override;
    void Visit(BitAndNode& bitAndNode) override;
    void Visit(EqualNode& equalNode) override;
    void Visit(NotEqualNode& notEqualNode) override;
    void Visit(LessNode& lessNode) override;
    void Visit(GreaterNode& greaterNode) override;
    void Visit(LessOrEqualNode& lessOrEqualNode) override;
    void Visit(GreaterOrEqualNode& greaterOrEqualNode) override;
    void Visit(ShiftLeftNode& shiftLeftNode) override;
    void Visit(ShiftRightNode& shiftRightNode) override;
    void Visit(AddNode& addNode) override;
    void Visit(SubNode& subNode) override;
    void Visit(MulNode& mulNode) override;
    void Visit(DivNode& divNode) override;
    void Visit(RemNode& remNode) override;
    void Visit(NotNode& notNode) override;
    void Visit(UnaryPlusNode& unaryPlusNode) override;
    void Visit(UnaryMinusNode& unaryMinusNode) override;
    void Visit(PrefixIncrementNode& prefixIncrementNode) override;
    void Visit(PrefixDecrementNode& prefixDecrementNode) override;
    void Visit(DerefNode& derefNode) override;
    void Visit(AddrOfNode& addrOfNode) override;
    void Visit(ComplementNode& complementNode) override;
    void Visit(IsNode& isNode) override;
    void Visit(AsNode& asNode) override;
    void Visit(IndexingNode& indexingNode) override;
    void Visit(InvokeNode& invokeNode) override;
    void Visit(PostfixIncrementNode& postfixIncrementNode) override;
    void Visit(PostfixDecrementNode& postfixDecrementNode) override;
    void Visit(SizeOfNode& sizeOfNode) override;
    void Visit(TypeNameNode& typeNameNode) override;
    void Visit(CastNode& castNode) override;
    void Visit(ConstructNode& constructNode) override;
    void Visit(NewNode& newNode) override;
    void Visit(ThisNode& thisNode) override;
    void Visit(BaseNode& baseNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    ContainerScope* containerScope;
    bool dontThrow;
    bool error;
    std::unique_ptr<Value> value;
    ValueType targetType;
    void EvaluateBinOp(BinaryNode& node, BinaryOperatorFun* fun);
    void EvaluateUnaryOp(UnaryNode& node, UnaryOperatorFun* fun);
    void EvaluateSymbol(Symbol* symbol, const Span& span);
    void EvaluateConstantSymbol(ConstantSymbol* constantSymbol, const Span& span);
    void EvaluateEnumConstantSymbol(EnumConstantSymbol* enumConstantSymbol, const Span& span);
};

Evaluator::Evaluator(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_, ValueType targetType_, bool dontThrow_) :
    boundCompileUnit(boundCompileUnit_), containerScope(containerScope_), dontThrow(dontThrow_), error(false), value(), targetType(targetType_)
{
}

void Evaluator::EvaluateBinOp(BinaryNode& node, BinaryOperatorFun* fun)
{
    node.Left()->Accept(*this);
    if (error)
    {
        return;
    }
    std::unique_ptr<Value> left(value.release());
    node.Right()->Accept(*this);
    if (error)
    {
        return;
    }
    std::unique_ptr<Value> right(value.release());
    ValueType leftType = left->GetValueType();
    ValueType rightType = right->GetValueType();
    ValueType commonType = CommonType(leftType, rightType);
    ValueType operationType = commonType;
    if (targetType > operationType)
    {
        operationType = targetType;
    }
    std::unique_ptr<Value> leftConverted(left->As(operationType, false, node.GetSpan(), dontThrow));
    std::unique_ptr<Value> rightConverted(right->As(operationType, false, node.GetSpan(), dontThrow));
    if (dontThrow)
    {
        if (!leftConverted || !rightConverted)
        {
            error = true;
            return;
        }
    }
    BinaryOperatorFun operation = fun[uint8_t(operationType)];
    value.reset(operation(leftConverted.get(), rightConverted.get(), node.GetSpan(), dontThrow));
}

void Evaluator::EvaluateUnaryOp(UnaryNode& node, UnaryOperatorFun* fun)
{
    node.Subject()->Accept(*this);
    if (error)
    {
        return;
    }
    std::unique_ptr<Value> subject(value.release());
    ValueType subjectType = subject->GetValueType();
    ValueType operationType = subjectType;
    if (targetType > operationType)
    {
        operationType = targetType;
    }
    std::unique_ptr<Value> subjectConverted(subject->As(operationType, false, node.GetSpan(), dontThrow));
    if (dontThrow)
    {
        if (!subjectConverted)
        {
            error = true;
            return;
        }
    }
    UnaryOperatorFun operation = fun[uint8_t(operationType)];
    value.reset(operation(subjectConverted.get(), node.GetSpan(), dontThrow));
}

std::unique_ptr<Value> Evaluator::GetValue()
{
    if (error)
    {
        return std::unique_ptr<Value>();
    }
    return std::move(value);
}

void Evaluator::Visit(BoolNode& boolNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(boolNode.GetSpan());
    }
}

void Evaluator::Visit(SByteNode& sbyteNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(sbyteNode.GetSpan());
    }
}

void Evaluator::Visit(ByteNode& byteNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(byteNode.GetSpan());
    }
}

void Evaluator::Visit(ShortNode& shortNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(shortNode.GetSpan());
    }
}

void Evaluator::Visit(UShortNode& ushortNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(ushortNode.GetSpan());
    }
}

void Evaluator::Visit(IntNode& intNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(intNode.GetSpan());
    }
}

void Evaluator::Visit(UIntNode& uintNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(uintNode.GetSpan());
    }
}

void Evaluator::Visit(LongNode& longNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(longNode.GetSpan());
    }
}

void Evaluator::Visit(ULongNode& ulongNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(ulongNode.GetSpan());
    }
}

void Evaluator::Visit(FloatNode& floatNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(floatNode.GetSpan());
    }
}

void Evaluator::Visit(DoubleNode& doubleNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(doubleNode.GetSpan());
    }
}

void Evaluator::Visit(CharNode& charNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(charNode.GetSpan());
    }
}

void Evaluator::Visit(WCharNode& wcharNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(wcharNode.GetSpan());
    }
}

void Evaluator::Visit(UCharNode& ucharNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(ucharNode.GetSpan());
    }
}

void Evaluator::Visit(VoidNode& voidNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(voidNode.GetSpan());
    }
}

void Evaluator::Visit(BooleanLiteralNode& booleanLiteralNode)
{
    value.reset(new BoolValue(booleanLiteralNode.GetSpan(), booleanLiteralNode.Value()));
}

void Evaluator::Visit(SByteLiteralNode& sbyteLiteralBode)
{
    value.reset(new SByteValue(sbyteLiteralBode.GetSpan(), sbyteLiteralBode.Value()));
}

void Evaluator::Visit(ByteLiteralNode& byteLiteralNode)
{
    value.reset(new ByteValue(byteLiteralNode.GetSpan(), byteLiteralNode.Value()));
}

void Evaluator::Visit(ShortLiteralNode& shortLiteralNode)
{
    value.reset(new ShortValue(shortLiteralNode.GetSpan(), shortLiteralNode.Value()));
}

void Evaluator::Visit(UShortLiteralNode& ushortLiteralNode)
{
    value.reset(new UShortValue(ushortLiteralNode.GetSpan(), ushortLiteralNode.Value()));
}

void Evaluator::Visit(IntLiteralNode& intLiteralNode)
{
    value.reset(new IntValue(intLiteralNode.GetSpan(), intLiteralNode.Value()));
}

void Evaluator::Visit(UIntLiteralNode& uintLiteralNode)
{
    value.reset(new UIntValue(uintLiteralNode.GetSpan(), uintLiteralNode.Value()));
}

void Evaluator::Visit(LongLiteralNode& longLiteralNode)
{
    value.reset(new LongValue(longLiteralNode.GetSpan(), longLiteralNode.Value()));
}

void Evaluator::Visit(ULongLiteralNode& ulongLiteralNode)
{
    value.reset(new ULongValue(ulongLiteralNode.GetSpan(), ulongLiteralNode.Value()));
}

void Evaluator::Visit(FloatLiteralNode& floatLiteralNode)
{
    value.reset(new FloatValue(floatLiteralNode.GetSpan(), floatLiteralNode.Value()));
}

void Evaluator::Visit(DoubleLiteralNode& doubleLiteralNode)
{
    value.reset(new DoubleValue(doubleLiteralNode.GetSpan(), doubleLiteralNode.Value()));
}

void Evaluator::Visit(CharLiteralNode& charLiteralNode)
{
    value.reset(new CharValue(charLiteralNode.GetSpan(), charLiteralNode.Value()));
}

void Evaluator::Visit(WCharLiteralNode& wcharLiteralNode)
{
    value.reset(new WCharValue(wcharLiteralNode.GetSpan(), wcharLiteralNode.Value()));
}

void Evaluator::Visit(UCharLiteralNode& ucharLiteralNode)
{
    value.reset(new UCharValue(ucharLiteralNode.GetSpan(), ucharLiteralNode.Value()));
}

void Evaluator::Visit(StringLiteralNode& stringLiteralNode)
{
    value.reset(new StringValue(stringLiteralNode.GetSpan(), boundCompileUnit.Install(stringLiteralNode.Value())));
}

void Evaluator::Visit(WStringLiteralNode& wstringLiteralNode)
{
    value.reset(new StringValue(wstringLiteralNode.GetSpan(), boundCompileUnit.Install(ToUtf8(wstringLiteralNode.Value()))));
}

void Evaluator::Visit(UStringLiteralNode& ustringLiteralNode)
{
    value.reset(new StringValue(ustringLiteralNode.GetSpan(), boundCompileUnit.Install(ToUtf8(ustringLiteralNode.Value()))));
}

void Evaluator::Visit(NullLiteralNode& nullLiteralNode)
{
    value.reset(new NullValue(nullLiteralNode.GetSpan(), boundCompileUnit.GetSymbolTable().GetTypeByName(U"@nullptr_type")));
}

void Evaluator::Visit(IdentifierNode& identifierNode)
{
    std::u32string name = identifierNode.Str();
    Symbol* symbol = containerScope->Lookup(name, ScopeLookup::this_and_base_and_parent);
    if (!symbol)
    {
        for (const std::unique_ptr<FileScope>& fileScope : boundCompileUnit.FileScopes())
        {
            symbol = fileScope->Lookup(name); 
            if (symbol) break;
        }
    }
    if (symbol)
    {
        EvaluateSymbol(symbol, identifierNode.GetSpan());
        if (error)
        {
            return;
        }
    }
    else
    {
        if (dontThrow)
        {
            error = true;
            return;
        }
        else
        {
            throw Exception("symbol '" + ToUtf8(name) + "' not found", identifierNode.GetSpan());
        }
    }
}

void Evaluator::Visit(TemplateIdNode& templateIdNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(templateIdNode.GetSpan());
    }
}

void Evaluator::EvaluateSymbol(Symbol* symbol, const Span& span)
{
    if (symbol->GetSymbolType() == SymbolType::constantSymbol)
    {
        ConstantSymbol* constantSymbol = static_cast<ConstantSymbol*>(symbol);
        EvaluateConstantSymbol(constantSymbol, span);
    }
    else if (symbol->GetSymbolType() == SymbolType::enumConstantSymbol)
    {
        EnumConstantSymbol* enumConstantSymbol = static_cast<EnumConstantSymbol*>(symbol);
        EvaluateEnumConstantSymbol(enumConstantSymbol, span);
    }
    else if (symbol->IsContainerSymbol())
    {
        ContainerSymbol* containerSymbol = static_cast<ContainerSymbol*>(symbol);
        value.reset(new ScopedValue(span, containerSymbol));
    }
    else
    {
        if (dontThrow)
        {
            error = true;
        }
        else
        {
            ThrowCannotEvaluateStatically(span);
        }
    }
}

void Evaluator::EvaluateConstantSymbol(ConstantSymbol* constantSymbol, const Span& span)
{
    if (constantSymbol->Evaluating())
    {
        if (dontThrow)
        {
            error = true;
            return;
        }
        throw Exception("cyclic depenency detected", span);
    }
    Value* constantValue = constantSymbol->GetValue();
    if (constantValue)
    {
        value.reset(constantValue->Clone());
    }
    else
    {
        Node* node = boundCompileUnit.GetSymbolTable().GetNode(constantSymbol);
        Assert(node->GetNodeType() == NodeType::constantNode, "constant node expected");
        ConstantNode* constantNode = static_cast<ConstantNode*>(node);
        constantSymbol->SetEvaluating();
        TypeBinder typeBinder(boundCompileUnit);
        typeBinder.SetContainerScope(containerScope);
        constantNode->Accept(typeBinder);
        constantSymbol->ResetEvaluating();
        Value* constantValue = constantSymbol->GetValue();
        Assert(constantValue, "constant value expected");
        value.reset(constantValue->Clone());
    }
}

void Evaluator::EvaluateEnumConstantSymbol(EnumConstantSymbol* enumConstantSymbol, const Span& span)
{
    if (enumConstantSymbol->Evaluating())
    {
        if (dontThrow)
        {
            error = true;
            return;
        }
        throw Exception("cyclic depenency detected", span);
    }
    Value* enumConstantValue = enumConstantSymbol->GetValue();
    if (enumConstantValue)
    {
        value.reset(enumConstantValue->Clone());
    }
    else
    {
        Symbol* symbol = enumConstantSymbol->Parent();
        Assert(symbol->GetSymbolType() == SymbolType::enumTypeSymbol, "enum type symbol expected");
        EnumTypeSymbol* enumTypeSymbol = static_cast<EnumTypeSymbol*>(symbol);
        Node* node = boundCompileUnit.GetSymbolTable().GetNode(enumTypeSymbol);
        Assert(node->GetNodeType() == NodeType::enumTypeNode, "enum type node expected");
        EnumTypeNode* enumTypeNode = static_cast<EnumTypeNode*>(node);
        TypeBinder typeBinder(boundCompileUnit);
        typeBinder.SetContainerScope(containerScope);
        enumTypeNode->Accept(typeBinder);
        enumConstantSymbol->ResetEvaluating();
        Value* enumConstantValue = enumConstantSymbol->GetValue();
        Assert(enumConstantValue, "enum constant value expected");
        value.reset(enumConstantValue->Clone());
    }
}

void Evaluator::Visit(DotNode& dotNode)
{
    dotNode.Subject()->Accept(*this);
    if (error)
    {
        return;
    }
    if (value && value->IsScopedValue())
    {
        ScopedValue* scopedValue = static_cast<ScopedValue*>(value.get());
        ContainerSymbol* containerSymbol = scopedValue->GetContainerSymbol();
        ContainerScope* scope = containerSymbol->GetContainerScope();
        std::u32string memberName = dotNode.MemberId()->Str();
        Symbol* symbol = scope->Lookup(memberName);
        if (symbol)
        {
            EvaluateSymbol(symbol, dotNode.GetSpan());
        }
        else
        {
            if (dontThrow)
            {
                error = true;
                return;
            }
            else
            {
                throw Exception("symbol '" + ToUtf8(containerSymbol->FullName()) + "' does not have member '" + ToUtf8(memberName) + "'", dotNode.GetSpan());
            }
        }
    }
    else
    {
        if (dontThrow)
        {
            error = true;
            return;
        }
        else
        {
            throw Exception("expression '" + dotNode.Subject()->ToString() + "' must denote a namespace, class type or enumerated type", dotNode.Subject()->GetSpan());
        }
    }
}

void Evaluator::Visit(ArrowNode& arrowNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(arrowNode.GetSpan());
    }
}

void Evaluator::Visit(EquivalenceNode& equivalenceNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(equivalenceNode.GetSpan());
    }
}

void Evaluator::Visit(ImplicationNode& implicationNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(implicationNode.GetSpan());
    }
}

void Evaluator::Visit(DisjunctionNode& disjunctionNode)
{
    EvaluateBinOp(disjunctionNode, disjunction);
}

void Evaluator::Visit(ConjunctionNode& conjunctionNode)
{
    EvaluateBinOp(conjunctionNode, conjunction);
}

void Evaluator::Visit(BitOrNode& bitOrNode)
{
    EvaluateBinOp(bitOrNode, bitOr);
}

void Evaluator::Visit(BitXorNode& bitXorNode)
{
    EvaluateBinOp(bitXorNode, bitXor);
}

void Evaluator::Visit(BitAndNode& bitAndNode) 
{
    EvaluateBinOp(bitAndNode, bitAnd);
}

void Evaluator::Visit(EqualNode& equalNode)
{
    EvaluateBinOp(equalNode, equal);
}

void Evaluator::Visit(NotEqualNode& notEqualNode)
{
    EvaluateBinOp(notEqualNode, notEqual);
}

void Evaluator::Visit(LessNode& lessNode)
{
    EvaluateBinOp(lessNode, less);
}

void Evaluator::Visit(GreaterNode& greaterNode)
{
    EvaluateBinOp(greaterNode, greater);
}

void Evaluator::Visit(LessOrEqualNode& lessOrEqualNode)
{
    EvaluateBinOp(lessOrEqualNode, lessEqual);
}

void Evaluator::Visit(GreaterOrEqualNode& greaterOrEqualNode)
{
    EvaluateBinOp(greaterOrEqualNode, greaterEqual);
}

void Evaluator::Visit(ShiftLeftNode& shiftLeftNode)
{
    EvaluateBinOp(shiftLeftNode, shiftLeft);
}

void Evaluator::Visit(ShiftRightNode& shiftRightNode)
{
    EvaluateBinOp(shiftRightNode, shiftRight);
}

void Evaluator::Visit(AddNode& addNode)
{
    EvaluateBinOp(addNode, add);
}

void Evaluator::Visit(SubNode& subNode)
{
    EvaluateBinOp(subNode, sub);
}

void Evaluator::Visit(MulNode& mulNode)
{
    EvaluateBinOp(mulNode, mul);
}

void Evaluator::Visit(DivNode& divNode)
{
    EvaluateBinOp(divNode, div);
}

void Evaluator::Visit(RemNode& remNode)
{
    EvaluateBinOp(remNode, rem);
}

void Evaluator::Visit(NotNode& notNode)
{
    EvaluateUnaryOp(notNode, logicalNot);
}

void Evaluator::Visit(UnaryPlusNode& unaryPlusNode)
{
    EvaluateUnaryOp(unaryPlusNode, unaryPlus);
}

void Evaluator::Visit(UnaryMinusNode& unaryMinusNode)
{
    EvaluateUnaryOp(unaryMinusNode, unaryMinus);
}

void Evaluator::Visit(PrefixIncrementNode& prefixIncrementNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(prefixIncrementNode.GetSpan());
    }
}

void Evaluator::Visit(PrefixDecrementNode& prefixDecrementNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(prefixDecrementNode.GetSpan());
    }
}

void Evaluator::Visit(DerefNode& derefNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(derefNode.GetSpan());
    }
}

void Evaluator::Visit(AddrOfNode& addrOfNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(addrOfNode.GetSpan());
    }
}

void Evaluator::Visit(ComplementNode& complementNode)
{
    EvaluateUnaryOp(complementNode, complement);
}

void Evaluator::Visit(IsNode& isNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(isNode.GetSpan());
    }
}

void Evaluator::Visit(AsNode& asNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(asNode.GetSpan());
    }
}

void Evaluator::Visit(IndexingNode& indexingNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(indexingNode.GetSpan());
    }
}

void Evaluator::Visit(InvokeNode& invokeNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(invokeNode.GetSpan());
    }
}

void Evaluator::Visit(PostfixIncrementNode& postfixIncrementNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(postfixIncrementNode.GetSpan());
    }
}

void Evaluator::Visit(PostfixDecrementNode& postfixDecrementNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(postfixDecrementNode.GetSpan());
    }
}

void Evaluator::Visit(SizeOfNode& sizeOfNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(sizeOfNode.GetSpan());
    }
}

void Evaluator::Visit(TypeNameNode& typeNameNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(typeNameNode.GetSpan());
    }
}

void Evaluator::Visit(CastNode& castNode)
{
    TypeSymbol* type = ResolveType(castNode.TargetTypeExpr(), boundCompileUnit, containerScope, false);
    SymbolType symbolType = type->GetSymbolType();
    ValueType valueType = GetValueTypeFor(symbolType);
    castNode.SourceExpr()->Accept(*this);
    value.reset(value->As(valueType, true, castNode.GetSpan(), dontThrow));
}

void Evaluator::Visit(ConstructNode& constructNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(constructNode.GetSpan());
    }
}

void Evaluator::Visit(NewNode& newNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(newNode.GetSpan());
    }
}

void Evaluator::Visit(ThisNode& thisNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(thisNode.GetSpan());
    }
}

void Evaluator::Visit(BaseNode& baseNode)
{
    if (dontThrow)
    {
        error = true;
    }
    else
    {
        ThrowCannotEvaluateStatically(baseNode.GetSpan());
    }
}

std::unique_ptr<Value> Evaluate(Node* node, ValueType targetType, ContainerScope* containerScope, BoundCompileUnit& boundCompileUnit, bool dontThrow)
{
    Evaluator evaluator(boundCompileUnit, containerScope, targetType, dontThrow);
    node->Accept(evaluator);
    if (evaluator.Error())
    {
        return std::unique_ptr<Value>();
    }
    else
    {
        std::unique_ptr<Value> value = evaluator.GetValue();
        if (value && value->IsComplete())
        {
            value.reset(value->As(targetType, false, node->GetSpan(), dontThrow));
            return std::move(value);
        }
        else
        {
            if (dontThrow)
            {
                return std::unique_ptr<Value>();
            }
            else
            {
                throw Exception("value not complete", node->GetSpan());
            }
        }
    }
}

} } // namespace cmajor::binder
