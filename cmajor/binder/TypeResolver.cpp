// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/TypeResolver.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/ast/Visitor.hpp>
#include <cmajor/symbols/Exception.hpp>

namespace cmajor { namespace binder {

class TypeResolver : public Visitor
{
public:
    TypeResolver(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_);
    TypeSymbol* GetType() { return type; }
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
private:
    BoundCompileUnit& boundCompileUnit;
    ContainerScope* containerScope;
    TypeSymbol* type;
};

TypeResolver::TypeResolver(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_) : boundCompileUnit(boundCompileUnit_), containerScope(containerScope_), type(nullptr)
{
}

void TypeResolver::Visit(BoolNode& boolNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"bool");
}

void TypeResolver::Visit(SByteNode& sbyteNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"sbyte");
}

void TypeResolver::Visit(ByteNode& byteNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"byte");
}

void TypeResolver::Visit(ShortNode& shortNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"short");
}

void TypeResolver::Visit(UShortNode& ushortNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"ushort");
}

void TypeResolver::Visit(IntNode& intNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"int");
}

void TypeResolver::Visit(UIntNode& uintNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"uint");
}

void TypeResolver::Visit(LongNode& longNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"long");
}

void TypeResolver::Visit(ULongNode& ulongNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"ulong");
}

void TypeResolver::Visit(FloatNode& floatNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"float");
}

void TypeResolver::Visit(DoubleNode& doubleNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"double");
}

void TypeResolver::Visit(CharNode& charNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"char");
}

void TypeResolver::Visit(WCharNode& wcharNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"wchar");
}

void TypeResolver::Visit(UCharNode& ucharNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"uchar");
}

void TypeResolver::Visit(VoidNode& voidNode)
{
    type = boundCompileUnit.GetSymbolTable().GetTypeByName(U"void");
}

TypeSymbol* ResolveType(Node* typeExprNode, BoundCompileUnit& boundCompileUnit, ContainerScope* containerScope)
{
    TypeResolver typeResolver(boundCompileUnit, containerScope);
    typeExprNode->Accept(typeResolver);
    TypeSymbol* type = typeResolver.GetType();
    if (type->IsInComplete())
    {
        throw Exception("incomplete type expression", typeExprNode->GetSpan());
    }
    return type;
}

} } // namespace cmajor::binder
