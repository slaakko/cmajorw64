// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
#define CMAJOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
#include <cmajor/binder/BoundNode.hpp>
#include <cmajor/binder/OperationRepository.hpp>
#include <cmajor/binder/FunctionTemplateRepository.hpp>
#include <cmajor/binder/ClassTemplateRepository.hpp>
#include <cmajor/binder/InlineFunctionRepository.hpp>
#include <cmajor/binder/StringRepository.hpp>
#include <cmajor/binder/ConceptRepository.hpp>
#include <cmajor/symbols/Module.hpp>
#include <cmajor/symbols/ConversionTable.hpp>
#include <cmajor/ast/CompileUnit.hpp>

namespace cmajor { namespace binder {

using namespace cmajor::symbols;

class BoundExpression;

class BoundCompileUnit : public BoundNode
{
public:
    BoundCompileUnit(Module& module_, CompileUnitNode* compileUnitNode_);
    void Load(Emitter& emitter, OperationFlags flags) override;
    void Store(Emitter& emitter, OperationFlags flags) override;
    void Accept(BoundNodeVisitor& visitor) override;
    Module& GetModule() { return module; }
    SymbolTable& GetSymbolTable() { return symbolTable; }
    CompileUnitNode* GetCompileUnitNode() const { return compileUnitNode; }
    void AddFileScope(FileScope* fileScope);
    void RemoveLastFileScope();
    FileScope* ReleaseLastFileScope();
    FileScope* FirstFileScope() const { Assert(!fileScopes.empty(), "file scopes empty");  return fileScopes.front().get(); }
    const std::vector<std::unique_ptr<FileScope>>& FileScopes() const { return fileScopes; }
    void AddBoundNode(std::unique_ptr<BoundNode>&& boundNode);
    const std::vector<std::unique_ptr<BoundNode>>& BoundNodes() const { return boundNodes; }
    FunctionSymbol* GetConversion(TypeSymbol* sourceType, TypeSymbol* targetType, ContainerScope* containerScope, const Span& span);
    void CollectViableFunctions(const std::u32string& groupName, ContainerScope* containerScope, std::vector<std::unique_ptr<BoundExpression>>& arguments, BoundFunction* currentFunction,
        std::unordered_set<FunctionSymbol*>& viableFunctions, std::unique_ptr<Exception>& exception, const Span& span);
    FunctionSymbol* InstantiateFunctionTemplate(FunctionSymbol* functionTemplate, const std::unordered_map<TemplateParameterSymbol*, TypeSymbol*>& templateParameterMapping, const Span& span);
    void InstantiateClassTemplateMemberFunction(FunctionSymbol* memberFunction, ContainerScope* containerScope, const Span& span);
    void InstantiateInlineFunction(FunctionSymbol* inlineFunction, ContainerScope* containerScope, const Span& span);
    int Install(const std::string& str);
    int Install(const std::u16string& str);
    int Install(const std::u32string& str);
    const std::string& GetUtf8String(int stringId) const;
    const std::u16string& GetUtf16String(int stringId) const;
    const std::u32string& GetUtf32String(int stringId) const;
    const std::string& SourceFilePath() const { return compileUnitNode->FilePath(); }
    const std::string& LLFilePath() const { return llFilePath; }
    const std::string& OptLLFilePath() const { return optLLFilePath; }
    const std::string& ObjectFilePath() const { return objectFilePath; }
    void SetHasGotos() { hasGotos = true; }
    bool HasGotos() const { return hasGotos; }
    ClassTemplateRepository& GetClassTemplateRepository() { return classTemplateRepository; }
    ConceptRepository& GetConceptRepository() { return conceptRepository; }
    void PushBindingTypes();
    void PopBindingTypes();
    bool BindingTypes() const { return bindingTypes; }
    void FinalizeBinding(ClassTypeSymbol* classType);
private:
    Module& module;
    SymbolTable& symbolTable;
    CompileUnitNode* compileUnitNode;
    std::string llFilePath;
    std::string optLLFilePath;
    std::string objectFilePath;
    std::vector<std::unique_ptr<FileScope>> fileScopes;
    std::vector<std::unique_ptr<BoundNode>> boundNodes;
    bool hasGotos;
    OperationRepository operationRepository;
    FunctionTemplateRepository functionTemplateRepository;
    ClassTemplateRepository classTemplateRepository;
    InlineFunctionRepository inlineFunctionRepository;
    StringRepository<std::string> utf8StringRepository;
    StringRepository<std::u16string> utf16StringRepository;
    StringRepository<std::u32string> utf32StringRepository;
    ConceptRepository conceptRepository;
    ConversionTable conversionTable;
    bool bindingTypes;
    std::stack<bool> bindingTypesStack;
};

} } // namespace cmajor::binder

#endif // CMAJOR_BINDER_BOUND_COMPILE_UNIT_INCLUDED
