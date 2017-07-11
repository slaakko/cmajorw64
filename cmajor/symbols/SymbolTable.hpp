// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_SYMBOL_TABLE_INCLUDED
#define CMAJOR_SYMBOLS_SYMBOL_TABLE_INCLUDED
#include <cmajor/symbols/NamespaceSymbol.hpp>
#include <cmajor/ast/Namespace.hpp>
#include <cmajor/ast/Function.hpp>
#include <cmajor/ast/Class.hpp>
#include <cmajor/ast/Interface.hpp>
#include <cmajor/ast/Delegate.hpp>
#include <cmajor/ast/Typedef.hpp>
#include <cmajor/ast/Constant.hpp>
#include <cmajor/ast/Enumeration.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::ast;

class FunctionSymbol;
class TypeSymbol;

struct TypeRequest
{
    TypeRequest(Symbol* symbol_, uint32_t typeId_, int index_) : symbol(symbol_), typeId(typeId_), index(index_) {}
    Symbol* symbol;
    uint32_t typeId;
    int index;
};

class SymbolTable
{
public:
    SymbolTable();
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader);
    const NamespaceSymbol& GlobalNs() const { return globalNs; }
    NamespaceSymbol& GlobalNs() { return globalNs; }
    void BeginContainer(ContainerSymbol* container_);
    void EndContainer();
    void BeginNamespace(NamespaceNode& namespaceNode);
    void BeginNamespace(const std::u32string& namespaceName, const Span& span);
    void EndNamespace();
    void BeginFunction(FunctionNode& functionNode);
    void EndFunction();
    void AddParameter(ParameterNode& parameterNode);
    void BeginClass(ClassNode& classNode);
    void EndClass();
    void AddTemplateParameter(TemplateParameterNode& templateParameterNode);
    void BeginInterface(InterfaceNode& interfaceNode);
    void EndInterface();
    void BeginStaticConstructor(StaticConstructorNode& staticConstructorNode);
    void EndStaticConstructor();
    void BeginConstructor(ConstructorNode& constructorNode);
    void EndConstructor();
    void BeginDestructor(DestructorNode& destructorNode);
    void EndDestructor();
    void BeginMemberFunction(MemberFunctionNode& memberFunctionNode);
    void EndMemberFunction();
    void AddMemberVariable(MemberVariableNode& memberVariableNode);
    void BeginDelegate(DelegateNode& delegateNode);
    void EndDelegate();
    void BeginClassDelegate(ClassDelegateNode& classDelegateNode);
    void EndClassDelegate();
    void BeginDeclarationBlock(Node& node);
    void EndDeclarationBlock();
    void AddLocalVariable(ConstructionStatementNode& constructionStatementNode);
    void AddLocalVariable(IdentifierNode& identifierNode);
    void AddTypedef(TypedefNode& typedefNode);
    void AddConstant(ConstantNode& constantNode);
    void BeginEnumType(EnumTypeNode& enumTypeNode);
    void EndEnumType();
    void AddEnumConstant(EnumConstantNode& enumConstantNode);
    void AddTypeSymbolToGlobalScope(TypeSymbol* typeSymbol);
    void MapNode(Node* node, Symbol* symbol);
    Symbol* GetSymbolNoThrow(Node* node) const;
    Symbol* GetSymbol(Node* node) const;
    Node* GetNodeNoThrow(Symbol* symbol) const;
    Node* GetNode(Symbol* symbol) const;
    uint32_t GetNextTypeId() { return nextTypeId++; }
    void SetTypeIdFor(TypeSymbol* typeSymbol);
    void AddTypeSymbolToTypeMap(TypeSymbol* typeSymbol);
    void EmplaceTypeRequest(Symbol* forSymbol, uint32_t typeId, int index);
    void ProcessTypeRequests();
    TypeSymbol* GetTypeByNameNoThrow(const std::u32string& typeName) const;
    TypeSymbol* GetTypeByName(const std::u32string& typeName) const;
private:
    NamespaceSymbol globalNs;
    ContainerSymbol* container;
    std::stack<ContainerSymbol*> containerStack;
    FunctionSymbol* mainFunctionSymbol;
    int parameterIndex;
    std::unordered_map<Node*, Symbol*> nodeSymbolMap;
    std::unordered_map<Symbol*, Node*> symbolNodeMap;
    uint32_t nextTypeId;
    std::unordered_map<uint32_t, TypeSymbol*> typeIdMap;
    std::unordered_map<std::u32string, TypeSymbol*> typeNameMap;
    std::vector<TypeRequest> typeRequests;
};

void InitSymbolTable(SymbolTable& symbolTable);

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_SYMBOL_TABLE_INCLUDED
