// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_SYMBOL_TABLE_INCLUDED
#define CMAJOR_SYMBOLS_SYMBOL_TABLE_INCLUDED
#include <cmajor/symbols/NamespaceSymbol.hpp>
#include <cmajor/symbols/DerivedTypeSymbol.hpp>
#include <cmajor/symbols/ConversionTable.hpp>
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

class TypeIdCounter
{
public:
    static void Init();
    static void Done();
    static TypeIdCounter& Instance() { Assert(instance, "type id counter not initialized"); return *instance; }
    int GetNextTypeId() { return nextTypeId++;  }
    void SetNextTypeId(int nextTypeId_) { nextTypeId = nextTypeId_; }
private:
    static std::unique_ptr<TypeIdCounter> instance;
    TypeIdCounter();
    int nextTypeId;
};

class SymbolTable
{
public:
    SymbolTable();
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader);
    void Import(SymbolTable& symbolTable);
    void Clear();
    const NamespaceSymbol& GlobalNs() const { return globalNs; }
    NamespaceSymbol& GlobalNs() { return globalNs; }
    const ContainerSymbol* Container() const { return container; }
    ContainerSymbol* Container() { return container; }
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
    void AddFunctionSymbolToGlobalScope(FunctionSymbol* functionSymbol);
    void MapNode(Node* node, Symbol* symbol);
    Symbol* GetSymbolNoThrow(Node* node) const;
    Symbol* GetSymbol(Node* node) const;
    Node* GetNodeNoThrow(Symbol* symbol) const;
    Node* GetNode(Symbol* symbol) const;
    void SetTypeIdFor(TypeSymbol* typeSymbol);
    void AddTypeSymbolToTypeIdMap(TypeSymbol* typeSymbol);
    void EmplaceTypeRequest(Symbol* forSymbol, uint32_t typeId, int index);
    void ProcessTypeRequests();
    TypeSymbol* GetTypeByNameNoThrow(const std::u32string& typeName) const;
    TypeSymbol* GetTypeByName(const std::u32string& typeName) const;
    TypeSymbol* MakeDerivedType(TypeSymbol* baseType, const TypeDerivationRec& derivationRec, const Span& span);
    const FunctionSymbol* MainFunctionSymbol() const { return mainFunctionSymbol; }
    void AddConversion(FunctionSymbol* conversion);
    FunctionSymbol* GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType, const Span& span) const;
private:
    NamespaceSymbol globalNs;
    ContainerSymbol* container;
    std::stack<ContainerSymbol*> containerStack;
    FunctionSymbol* mainFunctionSymbol;
    int parameterIndex;
    int declarationBlockIndex;
    std::unordered_map<Node*, Symbol*> nodeSymbolMap;
    std::unordered_map<Symbol*, Node*> symbolNodeMap;
    std::unordered_map<uint32_t, TypeSymbol*> typeIdMap;
    std::unordered_map<std::u32string, TypeSymbol*> typeNameMap;
    std::unordered_map<TypeSymbol*, std::vector<DerivedTypeSymbol*>> derivedTypeMap; 
    std::vector<std::unique_ptr<DerivedTypeSymbol>> derivedTypes;
    std::vector<TypeRequest> typeRequests;
    ConversionTable conversionTable;
    int GetNextDeclarationBlockIndex() { return declarationBlockIndex++; }
    void ResetDeclarationBlockIndex() { declarationBlockIndex = 0; }
};

void InitCoreSymbolTable(SymbolTable& symbolTable);

void InitSymbolTable();
void DoneSymbolTable();

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_SYMBOL_TABLE_INCLUDED
