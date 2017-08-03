// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/binder/TypeResolver.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/ast/Visitor.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/ast/Expression.hpp>
#include <cmajor/symbols/ClassTypeSymbol.hpp>
#include <cmajor/symbols/DerivedTypeSymbol.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/TemplateSymbol.hpp>
#include <cmajor/util/Unicode.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::unicode;

NamespaceTypeSymbol::NamespaceTypeSymbol(NamespaceSymbol* ns_) : TypeSymbol(SymbolType::namespaceTypeSymbol, ns_->GetSpan(), ns_->Name()), ns(ns_)
{
}

class TypeResolver : public Visitor
{
public:
    TypeResolver(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_);
    TypeSymbol* GetType() { return type; }
    const TypeDerivationRec& DerivationRec() const { return derivationRec; }
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
    void Visit(ConstNode& constNode) override;
    void Visit(LValueRefNode& lvalueRefNode) override;
    void Visit(RValueRefNode& rvalueRefNode) override;
    void Visit(PointerNode& pointerNode) override;
    void Visit(ArrayNode& arrayNode) override;
    void Visit(IdentifierNode& identifierNode) override;
    void Visit(DotNode& dotNode) override;
private:
    BoundCompileUnit& boundCompileUnit;
    SymbolTable& symbolTable;
    ContainerScope* containerScope;
    TypeSymbol* type;
    TypeDerivationRec derivationRec;
    std::unique_ptr<NamespaceTypeSymbol> nsTypeSymbol;
    void ResolveSymbol(Node& node, Symbol* symbol);
};

TypeResolver::TypeResolver(BoundCompileUnit& boundCompileUnit_, ContainerScope* containerScope_) : 
    boundCompileUnit(boundCompileUnit_), symbolTable(boundCompileUnit.GetSymbolTable()), containerScope(containerScope_), type(nullptr), derivationRec(), nsTypeSymbol()
{
}

void TypeResolver::Visit(BoolNode& boolNode)
{
    type = symbolTable.GetTypeByName(U"bool");
}

void TypeResolver::Visit(SByteNode& sbyteNode)
{
    type = symbolTable.GetTypeByName(U"sbyte");
}

void TypeResolver::Visit(ByteNode& byteNode)
{
    type = symbolTable.GetTypeByName(U"byte");
}

void TypeResolver::Visit(ShortNode& shortNode)
{
    type = symbolTable.GetTypeByName(U"short");
}

void TypeResolver::Visit(UShortNode& ushortNode)
{
    type = symbolTable.GetTypeByName(U"ushort");
}

void TypeResolver::Visit(IntNode& intNode)
{
    type = symbolTable.GetTypeByName(U"int");
}

void TypeResolver::Visit(UIntNode& uintNode)
{
    type = symbolTable.GetTypeByName(U"uint");
}

void TypeResolver::Visit(LongNode& longNode)
{
    type = symbolTable.GetTypeByName(U"long");
}

void TypeResolver::Visit(ULongNode& ulongNode)
{
    type = symbolTable.GetTypeByName(U"ulong");
}

void TypeResolver::Visit(FloatNode& floatNode)
{
    type = symbolTable.GetTypeByName(U"float");
}

void TypeResolver::Visit(DoubleNode& doubleNode)
{
    type = symbolTable.GetTypeByName(U"double");
}

void TypeResolver::Visit(CharNode& charNode)
{
    type = symbolTable.GetTypeByName(U"char");
}

void TypeResolver::Visit(WCharNode& wcharNode)
{
    type = symbolTable.GetTypeByName(U"wchar");
}

void TypeResolver::Visit(UCharNode& ucharNode)
{
    type = symbolTable.GetTypeByName(U"uchar");
}

void TypeResolver::Visit(VoidNode& voidNode)
{
    type = symbolTable.GetTypeByName(U"void");
}

void TypeResolver::Visit(ConstNode& constNode)
{
    derivationRec.derivations.push_back(Derivation::constDerivation);
    constNode.Subject()->Accept(*this);
}

void TypeResolver::Visit(LValueRefNode& lvalueRefNode)
{
    lvalueRefNode.Subject()->Accept(*this);
    if (HasReferenceDerivation(derivationRec.derivations))
    {
        throw Exception("cannot have reference to reference type", lvalueRefNode.GetSpan());
    }
    derivationRec.derivations.push_back(Derivation::lvalueRefDerivation);
}

void TypeResolver::Visit(RValueRefNode& rvalueRefNode)
{
    rvalueRefNode.Subject()->Accept(*this);
    if (HasReferenceDerivation(derivationRec.derivations))
    {
        throw Exception("cannot have reference to reference type", rvalueRefNode.GetSpan());
    }
    derivationRec.derivations.push_back(Derivation::rvalueRefDerivation);
}

void TypeResolver::Visit(PointerNode& pointerNode)
{
    pointerNode.Subject()->Accept(*this);
    if (HasReferenceDerivation(derivationRec.derivations))
    {
        throw Exception("cannot have pointer to reference type", pointerNode.GetSpan());
    }
    derivationRec.derivations.push_back(Derivation::pointerDerivation);
}

void TypeResolver::Visit(ArrayNode& arrayNode)
{
    arrayNode.Subject()->Accept(*this);
    if (HasReferenceDerivation(derivationRec.derivations))
    {
        throw Exception("cannot have array of reference type", arrayNode.GetSpan());
    }
    // todo: evaluate size
    derivationRec.derivations.push_back(Derivation::arrayDerivation);
}

void TypeResolver::ResolveSymbol(Node& node, Symbol* symbol)
{
    if (symbol->IsTypeSymbol())
    {
        type = static_cast<TypeSymbol*>(symbol);
    }
    else if (symbol->IsBoundTemplateParameterSymbol())
    {
        BoundTemplateParameterSymbol* boundTemplateParameterSymbol = static_cast<BoundTemplateParameterSymbol*>(symbol);
        type = boundTemplateParameterSymbol->GetType();
    }
    else if (symbol->GetSymbolType() == SymbolType::namespaceSymbol)
    {
        NamespaceSymbol* ns = static_cast<NamespaceSymbol*>(symbol);
        nsTypeSymbol.reset(new NamespaceTypeSymbol(ns));
        type = nsTypeSymbol.get();
    }
    else
    {
        throw Exception("symbol '" + ToUtf8(symbol->FullName()) + "' does not denote a type", node.GetSpan(), symbol->GetSpan());
    }
}

void TypeResolver::Visit(IdentifierNode& identifierNode)
{
    std::u32string name = identifierNode.Str();
    Symbol* symbol = containerScope->Lookup(name, ScopeLookup::this_and_base_and_parent);
    if (!symbol)
    {
        for (const std::unique_ptr<FileScope>& fileScope : boundCompileUnit.FileScopes())
        {
            symbol = fileScope->Lookup(name);
            if (symbol)
            {
                break;
            }
        }
    }
    if (symbol)
    {
        ResolveSymbol(identifierNode, symbol);
    }
    else
    {
        throw Exception("type symbol '" + ToUtf8(name) + "' not found", identifierNode.GetSpan());
    }
}

void TypeResolver::Visit(DotNode& dotNode)
{
    dotNode.Subject()->Accept(*this);
    Scope* scope = nullptr;
    if (type->GetSymbolType() == SymbolType::namespaceTypeSymbol)
    {
        NamespaceTypeSymbol* nsType = static_cast<NamespaceTypeSymbol*>(type);
        scope = nsType->Ns()->GetContainerScope();
    }
    else if (type->IsClassTypeSymbol())
    {
        scope = type->GetContainerScope();
    }
    else
    {
        throw Exception("symbol '" + ToUtf8(type->FullName()) + "' does not denote a class or a namespace", dotNode.GetSpan(), type->GetSpan());
    }
    std::u32string name = dotNode.MemberId()->Str();
    Symbol* symbol = scope->Lookup(name, ScopeLookup::this_and_base);
    if (symbol)
    {
        ResolveSymbol(dotNode, symbol);
    }
    else
    {
        throw Exception("type symbol '" + ToUtf8(name) + "' not found", dotNode.GetSpan());
    }
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
    TypeDerivationRec derivationRec = UnifyDerivations(typeResolver.DerivationRec(), type->DerivationRec());
    if (!derivationRec.derivations.empty())
    {
        return boundCompileUnit.GetSymbolTable().MakeDerivedType(type->BaseType(), derivationRec, typeExprNode->GetSpan());
    }
    return type;
}

} } // namespace cmajor::binder
