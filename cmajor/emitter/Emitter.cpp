// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/emitter/Emitter.hpp>
#include <cmajor/binder/BoundClass.hpp>
#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/binder/BoundStatement.hpp>
#include <cmajor/binder/BoundExpression.hpp>
#include <cmajor/binder/BoundNodeVisitor.hpp>
#include <cmajor/symbols/Module.hpp>
#include <cmajor/symbols/InterfaceTypeSymbol.hpp>
#include <cmajor/ir/Emitter.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/System.hpp>
#include <cmajor/util/TextUtils.hpp>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/raw_os_ostream.h>
#include <llvm/Support/CommandLine.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/DataLayout.h>
#include <llvm/IR/Verifier.h>
#include <llvm/Analysis/Passes.h>
#include <llvm/Transforms/IPO/PassManagerBuilder.h>
#include <llvm/Transforms/IPO.h>
#include <llvm/Analysis/TargetTransformInfo.h>
#include <llvm/Analysis/TargetLibraryInfo.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/CodeGen/TargetPassConfig.h>
#include <fstream>

namespace cmajor { namespace emitter {

using namespace cmajor::binder;
using namespace cmajor::symbols;
using namespace cmajor::unicode;

class EmittingContextImpl
{
public:
    EmittingContextImpl();
    llvm::LLVMContext& Context() { return context; }
    const std::string& TargetTriple() const { return targetTriple; }
    llvm::DataLayout& DataLayout() { return *dataLayout; }
    llvm::TargetMachine& TargetMachine() { return *targetMachine; }
private:
    llvm::LLVMContext context;
    std::string targetTriple;
    int optimizationLevel;
    std::unique_ptr<llvm::TargetMachine> targetMachine;
    std::unique_ptr<llvm::DataLayout> dataLayout;
};

EmittingContextImpl::EmittingContextImpl() : targetTriple(), optimizationLevel(0), targetMachine(), dataLayout()
{
    llvm::InitializeAllTargetInfos();
    llvm::InitializeAllTargets();
    llvm::InitializeAllTargetMCs();
    llvm::InitializeAllAsmParsers();
    llvm::InitializeAllAsmPrinters();
    llvm::PassRegistry* passRegistry = llvm::PassRegistry::getPassRegistry();
    llvm::initializeCore(*passRegistry);
    llvm::initializeScalarOpts(*passRegistry);
    llvm::initializeVectorization(*passRegistry);
    llvm::initializeIPO(*passRegistry);
    llvm::initializeAnalysis(*passRegistry);
    llvm::initializeTransformUtils(*passRegistry);
    llvm::initializeInstCombine(*passRegistry);
    llvm::initializeInstrumentation(*passRegistry);
    llvm::initializeTarget(*passRegistry);
    llvm::initializeCodeGen(*passRegistry);
    targetTriple = llvm::sys::getDefaultTargetTriple();
    std::string error;
    const llvm::Target* target = llvm::TargetRegistry::lookupTarget(targetTriple, error);
    if (!target)
    {
        throw std::runtime_error("EmittingContext: TargetRegistry::lookupTarget failed: " + error);
    }
    llvm::TargetOptions targetOptions = {};
    targetOptions.ExceptionModel = llvm::ExceptionHandling::WinEH;
    llvm::Optional<llvm::Reloc::Model> relocModel;
    llvm::CodeModel::Model codeModel = llvm::CodeModel::Default;
    llvm::CodeGenOpt::Level codeGenLevel = llvm::CodeGenOpt::None;
    optimizationLevel = GetOptimizationLevel();
    switch (optimizationLevel)
    {
        case 0: codeGenLevel = llvm::CodeGenOpt::None; break;
        case 1: codeGenLevel = llvm::CodeGenOpt::Less; break;
        case 2: codeGenLevel = llvm::CodeGenOpt::Default; break;
        case 3: codeGenLevel = llvm::CodeGenOpt::Aggressive; break;
    }
    targetMachine.reset(target->createTargetMachine(targetTriple, "generic", "", targetOptions, relocModel, codeModel, codeGenLevel));
    dataLayout.reset(new llvm::DataLayout(targetMachine->createDataLayout()));
}

EmittingContext::EmittingContext() : emittingContextImpl(new EmittingContextImpl())
{
}

EmittingContext::~EmittingContext()
{
    delete emittingContextImpl;
}

struct Cleanup
{
    Cleanup(llvm::BasicBlock* cleanupBlock_, llvm::BasicBlock* handlerBlock_, Pad* currentPad_) : cleanupBlock(cleanupBlock_), handlerBlock(handlerBlock_), currentPad(currentPad_) {}
    llvm::BasicBlock* cleanupBlock;
    llvm::BasicBlock* handlerBlock;
    Pad* currentPad;
    std::vector<std::unique_ptr<BoundFunctionCall>> destructors;
};

class Emitter : public cmajor::ir::Emitter, public BoundNodeVisitor
{
public:
    Emitter(EmittingContext& emittingContext_, const std::string& compileUnitModuleName_, cmajor::symbols::Module& symbolsModule_);
    void Visit(BoundCompileUnit& boundCompileUnit) override;
    void Visit(BoundClass& boundClass) override;
    void Visit(BoundFunction& boundFunction) override;
    void Visit(BoundSequenceStatement& boundSequenceStatement) override;
    void Visit(BoundCompoundStatement& boundCompoundStatement) override;
    void Visit(BoundReturnStatement& boundReturnStatement) override;
    void Visit(BoundIfStatement& boundIfStatement) override;
    void Visit(BoundWhileStatement& boundWhileStatement) override;
    void Visit(BoundDoStatement& boundDoStatement) override;
    void Visit(BoundForStatement& boundForStatement) override;
    void Visit(BoundSwitchStatement& boundSwitchStatement) override;
    void Visit(BoundCaseStatement& boundCaseStatement) override;
    void Visit(BoundDefaultStatement& boundDefaultStatement) override;
    void Visit(BoundGotoCaseStatement& boundGotoCaseStatement) override;
    void Visit(BoundGotoDefaultStatement& boundGotoDefaultStatement) override;
    void Visit(BoundBreakStatement& boundBreakStatement) override;
    void Visit(BoundContinueStatement& boundContinueStatement) override;
    void Visit(BoundGotoStatement& boundGotoStatement) override;
    void Visit(BoundConstructionStatement& boundConstructionStatement) override;
    void Visit(BoundAssignmentStatement& boundAssignmentStatement) override;
    void Visit(BoundExpressionStatement& boundExpressionStatement) override;
    void Visit(BoundEmptyStatement& boundEmptyStatement) override;
    void Visit(BoundSetVmtPtrStatement& boundSetVmtPtrStatement) override;
    void Visit(BoundThrowStatement& boundThrowStatement) override;
    void Visit(BoundRethrowStatement& boundRethrowStatement) override;
    void Visit(BoundTryStatement& boundTryStatement) override;
    void Visit(BoundParameter& boundParameter) override;
    void Visit(BoundLocalVariable& boundLocalVariable) override;
    void Visit(BoundMemberVariable& boundMemberVariable) override;
    void Visit(BoundConstant& boundConstant) override;
    void Visit(BoundEnumConstant& boundEnumConstant) override;
    void Visit(BoundLiteral& boundLiteral) override;
    void Visit(BoundTemporary& boundTemporary) override;
    void Visit(BoundSizeOfExpression& boundSizeOfExpression) override;
    void Visit(BoundAddressOfExpression& boundAddressOfExpression) override;
    void Visit(BoundDereferenceExpression& boundDereferenceExpression) override;
    void Visit(BoundReferenceToPointerExpression& boundReferenceToPointerExpression) override;
    void Visit(BoundFunctionCall& boundFunctionCall) override;
    void Visit(BoundDelegateCall& boundDelegateCall) override;
    void Visit(BoundClassDelegateCall& boundClassDelegateCall) override;
    void Visit(BoundConversion& boundConversion) override;
    void Visit(BoundConstructExpression& boundConstructExpression) override;
    void Visit(BoundConstructAndReturnTemporaryExpression& boundConstructAndReturnTemporaryExpression) override;
    void Visit(BoundClassOrClassDelegateConversionResult& boundClassOrClassDelegateConversionResult) override;
    void Visit(BoundIsExpression& boundIsExpression) override;
    void Visit(BoundAsExpression& boundAsExpression) override;
    void Visit(BoundTypeNameExpression& boundTypeNameExpression) override;
    void Visit(BoundBitCast& boundBitCast) override;
    void Visit(BoundFunctionPtr& boundFunctionPtr) override;
    void Visit(BoundDisjunction& boundDisjunction) override;
    void Visit(BoundConjunction& boundConjunction) override;
    llvm::Value* GetGlobalStringPtr(int stringId) override;
    llvm::Value* GetGlobalWStringConstant(int stringId) override;
    llvm::Value* GetGlobalUStringConstant(int stringId) override;
    llvm::BasicBlock* HandlerBlock() override { return handlerBlock; }
    llvm::BasicBlock* CleanupBlock() override { return cleanupBlock; }
    bool NewCleanupNeeded() override { return newCleanupNeeded; }
    void CreateCleanup() override;
    Pad* CurrentPad() override { return currentPad;  }
private:
    EmittingContext& emittingContext;
    SymbolTable* symbolTable;
    std::unique_ptr<llvm::Module> compileUnitModule;
    cmajor::symbols::Module& symbolsModule;
    llvm::IRBuilder<>& builder;
    cmajor::ir::ValueStack& stack;
    llvm::LLVMContext& context;
    llvm::Function* function;
    llvm::BasicBlock* trueBlock;
    llvm::BasicBlock* falseBlock;
    llvm::BasicBlock* breakTarget;
    llvm::BasicBlock* continueTarget;
    llvm::BasicBlock* handlerBlock;
    llvm::BasicBlock* cleanupBlock;
    bool newCleanupNeeded;
    std::vector<std::unique_ptr<Cleanup>> cleanups;
    std::vector<std::unique_ptr<Pad>> pads;
    std::unordered_map<BoundStatement*, llvm::BasicBlock*> labeledStatementMap;
    Pad* currentPad;
    bool genJumpingBoolCode;
    BoundCompileUnit* compileUnit;
    BoundClass* currentClass;
    BoundFunction* currentFunction;
    BoundCompoundStatement* currentBlock;
    BoundCompoundStatement* breakTargetBlock;
    BoundCompoundStatement* continueTargetBlock;
    std::unordered_map<IntegralValue, llvm::BasicBlock*, IntegralValueHash>* currentCaseMap;
    llvm::BasicBlock* defaultDest;
    std::stack<BoundClass*> classStack;
    std::vector<BoundCompoundStatement*> blocks;
    std::unordered_map<BoundCompoundStatement*, std::vector<std::unique_ptr<BoundFunctionCall>>> blockDestructionMap;
    std::unordered_map<int, llvm::Value*> utf8stringMap;
    std::unordered_map<int, llvm::Value*> utf16stringMap;
    std::unordered_map<int, llvm::Value*> utf32stringMap;
    int prevLineNumber;
    bool destructorCallGenerated;
    bool lastInstructionWasRet;
    bool basicBlockOpen;
    void GenJumpingBoolCode();
    void ExitBlocks(BoundCompoundStatement* targetBlock);
    void CreateExitFunctionCall();
    void SetLineNumber(int32_t lineNumber) override;
    void GenerateCodeForCleanups();
    void SetTarget(BoundStatement* labeledStatement);
};

Emitter::Emitter(EmittingContext& emittingContext_, const std::string& compileUnitModuleName_, cmajor::symbols::Module& symbolsModule_) :
    cmajor::ir::Emitter(emittingContext_.GetEmittingContextImpl()->Context()), emittingContext(emittingContext_), symbolTable(nullptr),
    compileUnitModule(new llvm::Module(compileUnitModuleName_, emittingContext.GetEmittingContextImpl()->Context())), symbolsModule(symbolsModule_), builder(Builder()), stack(Stack()),
    context(emittingContext.GetEmittingContextImpl()->Context()), compileUnit(nullptr), function(nullptr), trueBlock(nullptr), falseBlock(nullptr), breakTarget(nullptr), continueTarget(nullptr),
    handlerBlock(nullptr), cleanupBlock(nullptr), newCleanupNeeded(false), currentPad(nullptr), genJumpingBoolCode(false), currentClass(nullptr), currentFunction(nullptr), 
    currentBlock(nullptr), breakTargetBlock(nullptr), continueTargetBlock(nullptr), currentCaseMap(nullptr), defaultDest(nullptr), prevLineNumber(0), destructorCallGenerated(false), 
    lastInstructionWasRet(false), basicBlockOpen(false)
{
    compileUnitModule->setTargetTriple(emittingContext.GetEmittingContextImpl()->TargetTriple());
    compileUnitModule->setDataLayout(emittingContext.GetEmittingContextImpl()->DataLayout());
    compileUnitModule->setSourceFileName(compileUnitModuleName_);
    SetModule(compileUnitModule.get());
}

void Emitter::GenJumpingBoolCode()
{
    if (!genJumpingBoolCode) return;
    Assert(trueBlock, "true block not set");
    Assert(falseBlock, "false block not set");
    llvm::Value* cond = stack.Pop();
    builder.CreateCondBr(cond, trueBlock, falseBlock);
}

void Emitter::SetTarget(BoundStatement* labeledStatement)
{
    if (labeledStatement->Label().empty()) return;
    auto it = labeledStatementMap.find(labeledStatement);
    if (it != labeledStatementMap.cend())
    {
        llvm::BasicBlock* target = it->second;
        builder.CreateBr(target);
        SetCurrentBasicBlock(target);
    }
    else
    {
        throw Exception("target for labeled statement not found", labeledStatement->GetSpan());
    }
}

void Emitter::Visit(BoundCompileUnit& boundCompileUnit)
{
    compileUnit = &boundCompileUnit;
    symbolTable = &boundCompileUnit.GetSymbolTable();
    ConstantArrayRepository& constantArrayRepository = boundCompileUnit.GetConstantArrayRepository();
    for (ConstantSymbol* constantSymbol : constantArrayRepository.ConstantArrays())
    {
        constantSymbol->ArrayIrObject(*this, true);
    }
    int n = boundCompileUnit.BoundNodes().size();
    for (int i = 0; i < n; ++i)
    {
        BoundNode* boundNode = boundCompileUnit.BoundNodes()[i].get();
        boundNode->Accept(*this);
    }
    if (GetGlobalFlag(GlobalFlags::emitLlvm))
    {
        std::ofstream llFile(boundCompileUnit.LLFilePath());
        llvm::raw_os_ostream llOs(llFile);
        compileUnitModule->print(llOs, nullptr);
    }
    std::string errorMessageStore;
    llvm::raw_string_ostream errorMessage(errorMessageStore);
    if (verifyModule(*compileUnitModule, &errorMessage))
    {
        throw std::runtime_error("Emitter: verification of module '" + compileUnitModule->getSourceFileName() +"' failed. " + errorMessage.str());
    }
    llvm::legacy::PassManager passManager;
    std::error_code errorCode;
    llvm::raw_fd_ostream objectFile(boundCompileUnit.ObjectFilePath(), errorCode, llvm::sys::fs::F_None);
    if (emittingContext.GetEmittingContextImpl()->TargetMachine().addPassesToEmitFile(passManager, objectFile, llvm::TargetMachine::CGFT_ObjectFile))
    {
        throw std::runtime_error("Emitter: cannot emit object code file '" + boundCompileUnit.ObjectFilePath() + "': addPassesToEmitFile failed");
    }
    passManager.run(*compileUnitModule);
    objectFile.flush();
    if (objectFile.has_error())
    {
        throw std::runtime_error("Emitter: could not emit object code file '" + boundCompileUnit.ObjectFilePath() + "': " + errorCode.message());
    }
    if (GetGlobalFlag(GlobalFlags::emitOptLlvm))
    {
        std::string optCommandLine;
        optCommandLine.append("opt -O").append(std::to_string(GetOptimizationLevel())).append(" ").append(QuotedPath(boundCompileUnit.LLFilePath())).append(" -S -o ");
        optCommandLine.append(QuotedPath(boundCompileUnit.OptLLFilePath()));
        System(optCommandLine);
    }
}

void Emitter::Visit(BoundClass& boundClass)
{
    classStack.push(currentClass);
    currentClass = &boundClass;
    int n = boundClass.Members().size();
    for (int i = 0; i < n; ++i)
    {
        BoundNode* boundNode = boundClass.Members()[i].get();
        boundNode->Accept(*this);
    }
    currentClass = classStack.top();
    classStack.pop();
}

void Emitter::Visit(BoundFunction& boundFunction)
{
    if (!boundFunction.Body()) return;
    currentFunction = &boundFunction;
    handlerBlock = nullptr;
    cleanupBlock = nullptr;
    newCleanupNeeded = false;
    currentPad = nullptr;
    prevLineNumber = 0;
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    cleanups.clear();
    pads.clear();
    labeledStatementMap.clear();
    FunctionSymbol* functionSymbol = boundFunction.GetFunctionSymbol();
    llvm::FunctionType* functionType = functionSymbol->IrType(*this);
    function = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction(ToUtf8(functionSymbol->MangledName()), functionType));
    if (functionSymbol->HasLinkOnceOdrLinkage())
    {
        llvm::Comdat* comdat = compileUnitModule->getOrInsertComdat(ToUtf8(functionSymbol->MangledName()));
        comdat->setSelectionKind(llvm::Comdat::SelectionKind::Any);
        function->setLinkage(llvm::GlobalValue::LinkageTypes::LinkOnceODRLinkage);
        function->setComdat(comdat);
    }
    if (GetGlobalFlag(GlobalFlags::release) && functionSymbol->IsInline())
    {
        function->addFnAttr(llvm::Attribute::InlineHint);
    }
    SetFunction(function);
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "entry", function);
    SetCurrentBasicBlock(entryBlock);
    if (currentClass)
    {
        ClassTypeSymbol* classTypeSymbol = currentClass->GetClassTypeSymbol();
        classTypeSymbol->SetModule(&symbolsModule);
        if (!classTypeSymbol->IsVmtObjectCreated())
        {
            classTypeSymbol->VmtObject(*this, true);
        }
        if (!classTypeSymbol->IsStaticObjectCreated())
        {
            classTypeSymbol->StaticObject(*this, true);
        }
    }
    int np = functionSymbol->Parameters().size();
    for (int i = 0; i < np; ++i)
    {
        ParameterSymbol* parameter = functionSymbol->Parameters()[i];
        llvm::AllocaInst* allocaInst = builder.CreateAlloca(parameter->GetType()->IrType(*this));
        parameter->SetIrObject(allocaInst);
    }
    if (functionSymbol->ReturnParam())
    {
        ParameterSymbol* parameter = functionSymbol->ReturnParam();
        llvm::AllocaInst* allocaInst = builder.CreateAlloca(parameter->GetType()->IrType(*this));
        parameter->SetIrObject(allocaInst);
    }
    int nlv = functionSymbol->LocalVariables().size();
    for (int i = 0; i < nlv; ++i)
    {
        LocalVariableSymbol* localVariable = functionSymbol->LocalVariables()[i];
        llvm::AllocaInst* allocaInst = builder.CreateAlloca(localVariable->GetType()->IrType(*this));
        localVariable->SetIrObject(allocaInst);
    }
    if (!functionSymbol->DontThrow())
    {
        int funId = compileUnit->Install(ToUtf8(functionSymbol->FullName()));
        int sfpId = compileUnit->Install(compileUnit->SourceFilePath());
        llvm::Value* funValue = GetGlobalStringPtr(funId);
        llvm::Value* sfpValue = GetGlobalStringPtr(sfpId);
        std::vector<llvm::Type*> enterFunctionParamTypes;
        enterFunctionParamTypes.push_back(builder.getInt8PtrTy());
        enterFunctionParamTypes.push_back(builder.getInt8PtrTy());
        llvm::FunctionType* enterFunctionType = llvm::FunctionType::get(builder.getVoidTy(), enterFunctionParamTypes, false);
        llvm::Function* enterFunction = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction("RtEnterFunction", enterFunctionType));
        ArgVector enterFunctionArgs;
        enterFunctionArgs.push_back(funValue);
        enterFunctionArgs.push_back(sfpValue);
        builder.CreateCall(enterFunction, enterFunctionArgs);
    }
    auto it = function->args().begin();
    for (int i = 0; i < np; ++i)
    {
        ParameterSymbol* parameter = functionSymbol->Parameters()[i];
        if (parameter->GetType()->IsClassTypeSymbol())
        {
            ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(parameter->GetType());
            llvm::FunctionType* copyCtor = classType->CopyConstructor()->IrType(*this);
            llvm::Function* callee = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction(ToUtf8(classType->CopyConstructor()->MangledName()), copyCtor));
            ArgVector args;
            args.push_back(parameter->IrObject());
            args.push_back(&*it);
            builder.CreateCall(callee, args);
        }
        else if (parameter->GetType()->GetSymbolType() == SymbolType::classDelegateTypeSymbol)
        {
            ClassDelegateTypeSymbol* classDelegateType = static_cast<ClassDelegateTypeSymbol*>(parameter->GetType());
            FunctionSymbol* copyConstructor = classDelegateType->CopyConstructor();
            std::vector<GenObject*> copyCtorArgs;
            LlvmValue paramValue(parameter->IrObject());
            copyCtorArgs.push_back(&paramValue);
            LlvmValue argumentValue(&*it);
            copyCtorArgs.push_back(&argumentValue);
            copyConstructor->GenerateCall(*this, copyCtorArgs, OperationFlags::none);
        }
        else if (parameter->GetType()->GetSymbolType() == SymbolType::interfaceTypeSymbol)
        {
            InterfaceTypeSymbol* interfaceType = static_cast<InterfaceTypeSymbol*>(parameter->GetType());
            FunctionSymbol* copyConstructor = interfaceType->CopyConstructor();
            std::vector<GenObject*> copyCtorArgs;
            LlvmValue paramValue(parameter->IrObject());
            paramValue.SetType(interfaceType->AddPointer(Span()));
            copyCtorArgs.push_back(&paramValue);
            LlvmValue argumentValue(&*it);
            argumentValue.SetType(interfaceType->AddPointer(Span()));
            copyCtorArgs.push_back(&argumentValue);
            copyConstructor->GenerateCall(*this, copyCtorArgs, OperationFlags::none);
        }
        else
        {
            builder.CreateStore(&*it, parameter->IrObject());
        }
        ++it;
    }
    if (functionSymbol->ReturnParam())
    {
        builder.CreateStore(&*it, functionSymbol->ReturnParam()->IrObject());
    }
    for (BoundStatement* labeledStatement : boundFunction.LabeledStatements())
    {
        llvm::BasicBlock* target = llvm::BasicBlock::Create(context, ToUtf8(labeledStatement->Label()), function);
        labeledStatementMap[labeledStatement] = target;
    }
    BoundCompoundStatement* body = boundFunction.Body();
    body->Accept(*this);
    BoundStatement* lastStatement = nullptr;
    if (!body->Statements().empty())
    {
        lastStatement = body->Statements().back().get();
    }
    if (!lastStatement || lastStatement->GetBoundNodeType() != BoundNodeType::boundReturnStatement || lastStatement->GetBoundNodeType() == BoundNodeType::boundReturnStatement && destructorCallGenerated)
    {
        CreateExitFunctionCall();
        if (functionSymbol->ReturnType() && functionSymbol->ReturnType()->GetSymbolType() != SymbolType::voidTypeSymbol && !functionSymbol->ReturnsClassInterfaceOrClassDelegateByValue())
        {
            llvm::Value* defaultValue = functionSymbol->ReturnType()->CreateDefaultIrValue(*this);
            builder.CreateRet(defaultValue);
            lastInstructionWasRet = true;
        }
        else
        {
            builder.CreateRetVoid();
            lastInstructionWasRet = true;
        }
    }
    if (functionSymbol->HasTry() || !cleanups.empty())
    {
        llvm::FunctionType* personalityFunctionType = llvm::FunctionType::get(builder.getInt32Ty(), true);
        llvm::Function* personalityFunction = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction("__CxxFrameHandler3", personalityFunctionType));
        function->setPersonalityFn(llvm::ConstantExpr::getBitCast(personalityFunction, builder.getInt8PtrTy()));
    }
    if (functionSymbol->DontThrow() && !functionSymbol->HasTry() && cleanups.empty())
    {
        function->addFnAttr(llvm::Attribute::NoUnwind);
    }
    else
    {
        function->addFnAttr(llvm::Attribute::UWTable);
    }
    GenerateCodeForCleanups();
}

void Emitter::Visit(BoundSequenceStatement& boundSequenceStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundSequenceStatement);
    boundSequenceStatement.First()->Accept(*this);
    boundSequenceStatement.Second()->Accept(*this);
}

void Emitter::ExitBlocks(BoundCompoundStatement* targetBlock)
{
    bool createBasicBlock = false;
    BoundStatement* lastStatement = nullptr;
    if (!currentFunction->Body()->Statements().empty())
    {
        lastStatement = currentFunction->Body()->Statements().back().get();
    }
    BoundStatement* currentBlockLastStatement = nullptr;
    if (currentBlock && !currentBlock->Statements().empty())
    {
        currentBlockLastStatement = currentBlock->Statements().back().get();
    }
    if (lastStatement && currentBlockLastStatement && lastStatement == currentBlockLastStatement && currentBlockLastStatement->GetBoundNodeType() == BoundNodeType::boundReturnStatement)
    {
        createBasicBlock = true;
    }
    int n = blocks.size();
    for (int i = n - 1; i >= 0; --i)
    {
        BoundCompoundStatement* block = blocks[i];
        if (block == targetBlock)
        {
            break;
        }
        auto it = blockDestructionMap.find(block);
        if (it != blockDestructionMap.cend())
        {
            std::vector<std::unique_ptr<BoundFunctionCall>>& destructorCallVec = it->second;
            int nd = destructorCallVec.size();
            for (int i = nd - 1; i >= 0; --i)
            {
                std::unique_ptr<BoundFunctionCall>& destructorCall = destructorCallVec[i];
                if (destructorCall)
                {
                    if (createBasicBlock)
                    {
                        llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(context, "next", function);
                        if (!lastInstructionWasRet)
                        {
                            builder.CreateBr(nextBlock);
                        }
                        SetCurrentBasicBlock(nextBlock);
                        createBasicBlock = false;
                    }
                    destructorCall->Accept(*this);
                    destructorCallGenerated = true;
                }
            }
        }
    }
}

void Emitter::Visit(BoundCompoundStatement& boundCompoundStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundCompoundStatement);
    BoundCompoundStatement* prevBlock = currentBlock;
    currentBlock = &boundCompoundStatement;
    blockDestructionMap[currentBlock] = std::vector<std::unique_ptr<BoundFunctionCall>>();
    blocks.push_back(currentBlock);
    SetLineNumber(boundCompoundStatement.GetSpan().LineNumber());
    int n = boundCompoundStatement.Statements().size();
    for (int i = 0; i < n; ++i)
    {
        BoundStatement* boundStatement = boundCompoundStatement.Statements()[i].get();
        boundStatement->Accept(*this);
    }
    ExitBlocks(prevBlock);
    blocks.pop_back();
    currentBlock = prevBlock;
}

void Emitter::Visit(BoundReturnStatement& boundReturnStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundReturnStatement);
    Pad* prevCurrentPad = currentPad;
    while (currentPad != nullptr)
    {
        llvm::BasicBlock* returnTarget = llvm::BasicBlock::Create(context, "return", function);
        builder.CreateCatchRet(llvm::cast<llvm::CatchPadInst>(currentPad->value), returnTarget);
        SetCurrentBasicBlock(returnTarget);
        currentPad = currentPad->parent;
    }
    ExitBlocks(nullptr);
    CreateExitFunctionCall();
    BoundFunctionCall* returnFunctionCall = boundReturnStatement.ReturnFunctionCall();
    if (returnFunctionCall)
    {
        returnFunctionCall->Accept(*this);
        llvm::Value* returnValue = stack.Pop();
        builder.CreateRet(returnValue);
        lastInstructionWasRet = true;
    }
    else
    {
        builder.CreateRetVoid();
        lastInstructionWasRet = true;
    }
    BoundCompoundStatement* body = currentFunction->Body();
    BoundStatement* lastStatement = nullptr;
    if (!body->Statements().empty())
    {
        lastStatement = body->Statements().back().get();
    }
    if (lastStatement && lastStatement != &boundReturnStatement)
    {
        llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(context, "next", function);
        SetCurrentBasicBlock(nextBlock);
        basicBlockOpen = true;
        lastInstructionWasRet = false;
    }
    currentPad = prevCurrentPad;
}

void Emitter::Visit(BoundIfStatement& boundIfStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundIfStatement);
    llvm::BasicBlock* prevTrueBlock = trueBlock;
    llvm::BasicBlock* prevFalseBlock = falseBlock;
    trueBlock = llvm::BasicBlock::Create(context, "true", function);
    llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(context, "next", function);
    if (boundIfStatement.ElseS())
    {
        falseBlock = llvm::BasicBlock::Create(context, "false", function);
    }
    else
    {
        falseBlock = nextBlock;
    }
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    genJumpingBoolCode = true;
    boundIfStatement.Condition()->Accept(*this);
    genJumpingBoolCode = prevGenJumpingBoolCode;
    SetCurrentBasicBlock(trueBlock);
    boundIfStatement.ThenS()->Accept(*this);
    builder.CreateBr(nextBlock);
    if (boundIfStatement.ElseS())
    {
        SetCurrentBasicBlock(falseBlock);
        boundIfStatement.ElseS()->Accept(*this);
        builder.CreateBr(nextBlock);
    }
    trueBlock = prevTrueBlock;
    falseBlock = prevFalseBlock;
    SetCurrentBasicBlock(nextBlock);
    basicBlockOpen = true;
}

void Emitter::Visit(BoundWhileStatement& boundWhileStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundWhileStatement);
    llvm::BasicBlock* prevTrueBlock = trueBlock;
    llvm::BasicBlock* prevFalseBlock = falseBlock;
    llvm::BasicBlock* prevBreakTarget = breakTarget;
    llvm::BasicBlock* prevContinueTarget = continueTarget;
    BoundCompoundStatement* prevBreakTargetBlock = breakTargetBlock;
    BoundCompoundStatement* prevContinueTargetBlock = continueTargetBlock;
    breakTargetBlock = currentBlock;
    continueTargetBlock = currentBlock;
    trueBlock = llvm::BasicBlock::Create(context, "true", function);
    falseBlock = llvm::BasicBlock::Create(context, "next", function);
    breakTarget = falseBlock;
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(context, "cond", function);
    builder.CreateBr(condBlock);
    SetCurrentBasicBlock(condBlock);
    continueTarget = condBlock;
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    genJumpingBoolCode = true;
    boundWhileStatement.Condition()->Accept(*this);
    genJumpingBoolCode = prevGenJumpingBoolCode;
    SetCurrentBasicBlock(trueBlock);
    boundWhileStatement.Statement()->Accept(*this);
    builder.CreateBr(condBlock);
    SetCurrentBasicBlock(falseBlock);
    breakTargetBlock = prevBreakTargetBlock;
    continueTargetBlock = prevContinueTargetBlock;
    breakTarget = prevBreakTarget;
    continueTarget = prevContinueTarget;
    trueBlock = prevTrueBlock;
    falseBlock = prevFalseBlock;
}

void Emitter::Visit(BoundDoStatement& boundDoStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundDoStatement);
    llvm::BasicBlock* prevTrueBlock = trueBlock;
    llvm::BasicBlock* prevFalseBlock = falseBlock;
    llvm::BasicBlock* prevBreakTarget = breakTarget;
    llvm::BasicBlock* prevContinueTarget = continueTarget;
    llvm::BasicBlock* doBlock = llvm::BasicBlock::Create(context, "do", function);
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(context, "cond", function);
    BoundCompoundStatement* prevBreakTargetBlock = breakTargetBlock;
    BoundCompoundStatement* prevContinueTargetBlock = continueTargetBlock;
    breakTargetBlock = currentBlock;
    continueTargetBlock = currentBlock;
    trueBlock = doBlock;
    falseBlock = llvm::BasicBlock::Create(context, "next", function);
    breakTarget = falseBlock;
    continueTarget = condBlock;
    builder.CreateBr(doBlock);
    SetCurrentBasicBlock(doBlock);
    boundDoStatement.Statement()->Accept(*this);
    builder.CreateBr(condBlock);
    SetCurrentBasicBlock(condBlock);
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    genJumpingBoolCode = true;
    boundDoStatement.Condition()->Accept(*this);
    genJumpingBoolCode = prevGenJumpingBoolCode;
    SetCurrentBasicBlock(falseBlock);
    basicBlockOpen = true;
    breakTargetBlock = prevBreakTargetBlock;
    continueTargetBlock = prevContinueTargetBlock;
    breakTarget = prevBreakTarget;
    continueTarget = prevContinueTarget;
    trueBlock = prevTrueBlock;
    falseBlock = prevFalseBlock;
}

void Emitter::Visit(BoundForStatement& boundForStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundForStatement);
    llvm::BasicBlock* prevTrueBlock = trueBlock;
    llvm::BasicBlock* prevFalseBlock = falseBlock;
    llvm::BasicBlock* prevBreakTarget = breakTarget;
    llvm::BasicBlock* prevContinueTarget = continueTarget;
    boundForStatement.InitS()->Accept(*this);
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(context, "cond", function);
    llvm::BasicBlock* actionBlock = llvm::BasicBlock::Create(context, "action", function);
    llvm::BasicBlock* loopBlock = llvm::BasicBlock::Create(context, "loop", function);
    trueBlock = actionBlock;
    falseBlock = llvm::BasicBlock::Create(context, "next", function);
    breakTarget = falseBlock;
    continueTarget = loopBlock;
    BoundCompoundStatement* prevBreakTargetBlock = breakTargetBlock;
    BoundCompoundStatement* prevContinueTargetBlock = continueTargetBlock;
    breakTargetBlock = currentBlock;
    continueTargetBlock = currentBlock;
    builder.CreateBr(condBlock);
    SetCurrentBasicBlock(condBlock);
    bool prevGenJumpingBoolCode = genJumpingBoolCode;
    genJumpingBoolCode = true;
    boundForStatement.Condition()->Accept(*this);
    genJumpingBoolCode = prevGenJumpingBoolCode;
    SetCurrentBasicBlock(actionBlock);
    boundForStatement.ActionS()->Accept(*this);
    builder.CreateBr(loopBlock);
    SetCurrentBasicBlock(loopBlock);
    boundForStatement.LoopS()->Accept(*this);
    builder.CreateBr(condBlock);
    SetCurrentBasicBlock(falseBlock);
    basicBlockOpen = true;
    breakTargetBlock = prevBreakTargetBlock;
    continueTargetBlock = prevContinueTargetBlock;
    breakTarget = prevBreakTarget;
    continueTarget = prevContinueTarget;
    trueBlock = prevTrueBlock;
    falseBlock = prevFalseBlock;
}

void Emitter::Visit(BoundSwitchStatement& boundSwitchStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundSwitchStatement);
    llvm::BasicBlock* prevBreakTarget = breakTarget;
    BoundCompoundStatement* prevBreakTargetBlock = breakTargetBlock;
    breakTargetBlock = currentBlock;
    boundSwitchStatement.Condition()->Accept(*this);
    llvm::Value* condition = stack.Pop();
    llvm::BasicBlock* prevDefaultDest = defaultDest;
    llvm::BasicBlock* next = nullptr;
    if (boundSwitchStatement.DefaultStatement())
    {
        defaultDest = llvm::BasicBlock::Create(context, "default", function);
        next = llvm::BasicBlock::Create(context, "next", function);
    }
    else
    {
        defaultDest = llvm::BasicBlock::Create(context, "next", function);
        next = defaultDest;
    }
    breakTarget = next;
    int n = boundSwitchStatement.CaseStatements().size();
    llvm::SwitchInst* switchInst = builder.CreateSwitch(condition, defaultDest, n);
    std::unordered_map<IntegralValue, llvm::BasicBlock*, IntegralValueHash>* prevCaseMap = currentCaseMap;
    std::unordered_map<IntegralValue, llvm::BasicBlock*, IntegralValueHash> caseMap;
    currentCaseMap = &caseMap;
    for (int i = 0; i < n; ++i)
    {
        const std::unique_ptr<BoundCaseStatement>& caseS = boundSwitchStatement.CaseStatements()[i];
        llvm::BasicBlock* caseDest = llvm::BasicBlock::Create(context, "case" + std::to_string(i), function);
        for (const std::unique_ptr<Value>& caseValue : caseS->CaseValues())
        {
            IntegralValue integralCaseValue(caseValue.get());
            caseMap[integralCaseValue] = caseDest;
            switchInst->addCase(llvm::cast<llvm::ConstantInt>(caseValue->IrValue(*this)), caseDest);
        }
    }
    for (int i = 0; i < n; ++i)
    {
        const std::unique_ptr<BoundCaseStatement>& caseS = boundSwitchStatement.CaseStatements()[i];
        caseS->Accept(*this);
        if (basicBlockOpen)
        {
            builder.CreateBr(next);
            basicBlockOpen = false;
        }
    }
    if (boundSwitchStatement.DefaultStatement())
    {
        boundSwitchStatement.DefaultStatement()->Accept(*this);
        if (basicBlockOpen)
        {
            builder.CreateBr(next);
            basicBlockOpen = false;
        }
    }
    SetCurrentBasicBlock(next);
    basicBlockOpen = true;
    currentCaseMap = prevCaseMap;
    defaultDest = prevDefaultDest;
    breakTargetBlock = prevBreakTargetBlock;
    breakTarget = prevBreakTarget;
}

void Emitter::Visit(BoundCaseStatement& boundCaseStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundCaseStatement);
    if (!boundCaseStatement.CaseValues().empty())
    {
        IntegralValue integralCaseValue(boundCaseStatement.CaseValues().front().get());
        auto it = currentCaseMap->find(integralCaseValue);
        if (it != currentCaseMap->cend())
        {
            llvm::BasicBlock* caseDest = it->second;
            SetCurrentBasicBlock(caseDest);
            boundCaseStatement.CompoundStatement()->Accept(*this);
        }
        else
        {
            throw Exception("case not found", boundCaseStatement.GetSpan());
        }
    }
    else
    {
        throw Exception("no cases", boundCaseStatement.GetSpan());
    }
}

void Emitter::Visit(BoundDefaultStatement& boundDefaultStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundDefaultStatement);
    if (defaultDest)
    {
        SetCurrentBasicBlock(defaultDest);
        boundDefaultStatement.CompoundStatement()->Accept(*this);
    }
    else
    {
        throw Exception("no default destination", boundDefaultStatement.GetSpan());
    }
}

void Emitter::Visit(BoundGotoCaseStatement& boundGotoCaseStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundGotoCaseStatement);
    Assert(breakTargetBlock, "break target not set");
    Pad* prevCurrentPad = currentPad;
    BoundStatement* parent = currentBlock;
    while (currentPad != nullptr && parent && parent != breakTargetBlock)
    {
        if (parent->GetBoundNodeType() == BoundNodeType::boundTryStatement)
        {
            llvm::BasicBlock* fromCatchTarget = llvm::BasicBlock::Create(context, "fromCatch", function);
            builder.CreateCatchRet(llvm::cast<llvm::CatchPadInst>(currentPad->value), fromCatchTarget);
            SetCurrentBasicBlock(fromCatchTarget);
            currentPad = currentPad->parent;
        }
        parent = parent->Parent();
    }
    ExitBlocks(breakTargetBlock);
    IntegralValue integralCaseValue(boundGotoCaseStatement.CaseValue());
    auto it = currentCaseMap->find(integralCaseValue);
    if (it != currentCaseMap->cend())
    {
        llvm::BasicBlock* caseDest = it->second;
        builder.CreateBr(caseDest);
    }
    else
    {
        throw Exception("case not found", boundGotoCaseStatement.GetSpan());
    }
    currentPad = prevCurrentPad;
}

void Emitter::Visit(BoundGotoDefaultStatement& boundGotoDefaultStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundGotoDefaultStatement);
    Assert(breakTargetBlock, "break target not set");
    Pad* prevCurrentPad = currentPad;
    BoundStatement* parent = currentBlock;
    while (currentPad != nullptr && parent && parent != breakTargetBlock)
    {
        if (parent->GetBoundNodeType() == BoundNodeType::boundTryStatement)
        {
            llvm::BasicBlock* fromCatchTarget = llvm::BasicBlock::Create(context, "fromCatch", function);
            builder.CreateCatchRet(llvm::cast<llvm::CatchPadInst>(currentPad->value), fromCatchTarget);
            SetCurrentBasicBlock(fromCatchTarget);
            currentPad = currentPad->parent;
        }
        parent = parent->Parent();
    }
    ExitBlocks(breakTargetBlock);
    if (defaultDest)
    {
        builder.CreateBr(defaultDest);
    }
    else
    {
        throw Exception("no default destination", boundGotoDefaultStatement.GetSpan());
    }
    currentPad = prevCurrentPad;
}

void Emitter::Visit(BoundBreakStatement& boundBreakStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundBreakStatement);
    Assert(breakTarget && breakTargetBlock, "break target not set");
    Pad* prevCurrentPad = currentPad;
    BoundStatement* parent = currentBlock;
    while (currentPad != nullptr && parent && parent != breakTargetBlock)
    {
        if (parent->GetBoundNodeType() == BoundNodeType::boundTryStatement)
        {
            llvm::BasicBlock* fromCatchTarget = llvm::BasicBlock::Create(context, "fromCatch", function);
            builder.CreateCatchRet(llvm::cast<llvm::CatchPadInst>(currentPad->value), fromCatchTarget);
            SetCurrentBasicBlock(fromCatchTarget);
            currentPad = currentPad->parent;
        }
        parent = parent->Parent();
    }
    ExitBlocks(breakTargetBlock);
    builder.CreateBr(breakTarget);
    if (!currentCaseMap) // not in switch
    {
        llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(context, "next", function);
        SetCurrentBasicBlock(nextBlock);
        basicBlockOpen = true;
    }
    currentPad = prevCurrentPad;
}

void Emitter::Visit(BoundContinueStatement& boundContinueStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundContinueStatement);
    Assert(continueTarget && continueTargetBlock, "continue target not set");
    Pad* prevCurrentPad = currentPad;
    BoundStatement* parent = currentBlock;
    while (currentPad != nullptr && parent && parent != continueTargetBlock)
    {
        if (parent->GetBoundNodeType() == BoundNodeType::boundTryStatement)
        {
            llvm::BasicBlock* fromCatchTarget = llvm::BasicBlock::Create(context, "fromCatch", function);
            builder.CreateCatchRet(llvm::cast<llvm::CatchPadInst>(currentPad->value), fromCatchTarget);
            SetCurrentBasicBlock(fromCatchTarget);
            currentPad = currentPad->parent;
        }
        parent = parent->Parent();
    }
    ExitBlocks(continueTargetBlock);
    builder.CreateBr(continueTarget);
    llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(context, "next", function);
    SetCurrentBasicBlock(nextBlock);
    basicBlockOpen = true;
    currentPad = prevCurrentPad;
}

void Emitter::Visit(BoundGotoStatement& boundGotoStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundGotoStatement);
    Pad* prevCurrentPad = currentPad;
    BoundStatement* parent = currentBlock;
    while (currentPad != nullptr && parent && parent != boundGotoStatement.TargetBlock())
    {
        if (parent->GetBoundNodeType() == BoundNodeType::boundTryStatement)
        {
            llvm::BasicBlock* fromCatchTarget = llvm::BasicBlock::Create(context, "fromCatch", function);
            builder.CreateCatchRet(llvm::cast<llvm::CatchPadInst>(currentPad->value), fromCatchTarget);
            SetCurrentBasicBlock(fromCatchTarget);
            currentPad = currentPad->parent;
        }
        parent = parent->Parent();
    }
    ExitBlocks(boundGotoStatement.TargetBlock());
    auto it = labeledStatementMap.find(boundGotoStatement.TargetStatement());
    if (it != labeledStatementMap.cend())
    {
        llvm::BasicBlock* target = it->second;
        builder.CreateBr(target);
    }
    else
    {
        throw Exception("goto target not found", boundGotoStatement.GetSpan());
    }
    llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(context, "next", function);
    SetCurrentBasicBlock(nextBlock);
    basicBlockOpen = true;
    currentPad = prevCurrentPad;
}

void Emitter::Visit(BoundConstructionStatement& boundConstructionStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundConstructionStatement);
    boundConstructionStatement.ConstructorCall()->Accept(*this);
    if (!boundConstructionStatement.ConstructorCall()->GetFunctionSymbol()->IsBasicTypeOperation())
    {
        int n = boundConstructionStatement.ConstructorCall()->Arguments().size();
        if (n > 0)
        {
            const std::unique_ptr<BoundExpression>& firstArgument = boundConstructionStatement.ConstructorCall()->Arguments()[0];
            TypeSymbol* firstArgumentBaseType = firstArgument->GetType()->BaseType();
            if (firstArgumentBaseType->IsClassTypeSymbol())
            {
                if (firstArgument->GetType()->IsPointerType() && firstArgument->GetType()->RemovePointer(boundConstructionStatement.GetSpan())->IsClassTypeSymbol())
                {
                    ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(firstArgumentBaseType);
                    if (classType->Destructor())
                    {
                        newCleanupNeeded = true;
                        std::unique_ptr<BoundExpression> classPtrArgument(firstArgument->Clone());
                        std::unique_ptr<BoundFunctionCall> destructorCall(new BoundFunctionCall(boundConstructionStatement.GetSpan(), classType->Destructor()));
                        destructorCall->AddArgument(std::move(classPtrArgument));
                        Assert(currentBlock, "current block not set");
                        auto it = blockDestructionMap.find(currentBlock);
                        if (it != blockDestructionMap.cend())
                        {
                            std::vector<std::unique_ptr<BoundFunctionCall>>& destructorCallVec = it->second;
                            destructorCallVec.push_back(std::move(destructorCall));
                        }
                        else
                        {
                            Assert(false, "block destruction not found");
                        }
                    }
                }
            }
        }
    }
}

void Emitter::Visit(BoundAssignmentStatement& boundAssignmentStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundAssignmentStatement);
    boundAssignmentStatement.AssignmentCall()->Accept(*this);
}

void Emitter::Visit(BoundExpressionStatement& boundExpressionStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundExpressionStatement);
    boundExpressionStatement.Expression()->Accept(*this);
    if (boundExpressionStatement.Expression()->HasValue())
    {
        stack.Pop();
    }
}

void Emitter::Visit(BoundEmptyStatement& boundEmptyStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundEmptyStatement);
    llvm::Type* retType = llvm::Type::getVoidTy(context);
    std::vector<llvm::Type*> paramTypes;
    llvm::FunctionType* doNothingFunType = llvm::FunctionType::get(retType, paramTypes, false);
    llvm::Function* doNothingFun = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction("llvm.donothing", doNothingFunType));
    ArgVector args;
    std::vector<llvm::OperandBundleDef> bundles;
    if (currentPad != nullptr)
    {
        std::vector<llvm::Value*> inputs;
        inputs.push_back(currentPad->value);
        bundles.push_back(llvm::OperandBundleDef("funclet", inputs));
    }
    if (currentPad == nullptr)
    {
        builder.CreateCall(doNothingFunType, doNothingFun, args);
    }
    else
    {
        llvm::CallInst::Create(doNothingFun, args, bundles, "", CurrentBasicBlock());
    }
}

void Emitter::Visit(BoundSetVmtPtrStatement& boundSetVmtPtrStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundSetVmtPtrStatement);
    BoundExpression* classPtr = boundSetVmtPtrStatement.ClassPtr();
    TypeSymbol* type = classPtr->GetType()->BaseType();
    Assert(type->IsClassTypeSymbol(), "class type expected");
    ClassTypeSymbol* classType = static_cast<ClassTypeSymbol*>(type);
    int32_t vmtPtrIndex = classType->VmtPtrIndex();
    Assert(vmtPtrIndex != -1, "invalid vmt ptr index");
    classPtr->Accept(*this);
    llvm::Value* classPtrValue = stack.Pop();
    ArgVector indeces;
    indeces.push_back(builder.getInt32(0));
    indeces.push_back(builder.getInt32(vmtPtrIndex));
    llvm::Value* ptr = builder.CreateGEP(classPtrValue, indeces);
    llvm::Value* vmtPtr = builder.CreateBitCast(boundSetVmtPtrStatement.ClassType()->VmtObject(*this, false), builder.getInt8PtrTy());
    builder.CreateStore(vmtPtr, ptr);
}

void Emitter::Visit(BoundThrowStatement& boundThrowStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundThrowStatement);
    boundThrowStatement.ThrowCall()->Accept(*this);
}

void Emitter::Visit(BoundRethrowStatement& boundRethrowStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundRethrowStatement);
    boundRethrowStatement.ReleaseCall()->Accept(*this);
    llvm::Function* cxxThrowFunction = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction("_CxxThrowException",
        builder.getVoidTy(), builder.getInt8PtrTy(), builder.getInt8PtrTy(), nullptr));
    ArgVector rethrowArgs;
    rethrowArgs.push_back(llvm::Constant::getNullValue(builder.getInt8PtrTy()));
    rethrowArgs.push_back(llvm::Constant::getNullValue(builder.getInt8PtrTy()));
    std::vector<llvm::OperandBundleDef> bundles;
    std::vector<llvm::Value*> inputs;
    inputs.push_back(currentPad->value);
    bundles.push_back(llvm::OperandBundleDef("funclet", inputs));
    llvm::CallInst::Create(cxxThrowFunction, rethrowArgs, bundles, "", CurrentBasicBlock());
}

void Emitter::Visit(BoundTryStatement& boundTryStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundTryStatement);
    llvm::BasicBlock* prevHandlerBlock = handlerBlock;
    llvm::BasicBlock* prevCleanupBlock = cleanupBlock;
    handlerBlock = llvm::BasicBlock::Create(context, "handlers", function);
    cleanupBlock = nullptr;
    boundTryStatement.TryBlock()->Accept(*this);
    llvm::BasicBlock* nextTarget = llvm::BasicBlock::Create(context, "next", function);
    builder.CreateBr(nextTarget);
    SetCurrentBasicBlock(handlerBlock);
    handlerBlock = prevHandlerBlock;
    Pad* parentPad = currentPad;
    llvm::CatchSwitchInst* catchSwitch = nullptr;
    if (parentPad == nullptr)
    {
        catchSwitch = builder.CreateCatchSwitch(llvm::ConstantTokenNone::get(context), prevHandlerBlock, 1);
    }
    else
    {
        catchSwitch = builder.CreateCatchSwitch(parentPad->value, prevHandlerBlock, 1);
    }
    Pad* pad = new Pad();
    pads.push_back(std::unique_ptr<Pad>(pad));
    pad->parent = parentPad;
    pad->value = catchSwitch;
    currentPad = pad;
    llvm::BasicBlock* catchPadTarget = llvm::BasicBlock::Create(context, "catchpad", function);
    catchSwitch->addHandler(catchPadTarget);
    SetCurrentBasicBlock(catchPadTarget);
    ArgVector catchPadArgs;
    catchPadArgs.push_back(llvm::Constant::getNullValue(builder.getInt8PtrTy()));
    catchPadArgs.push_back(builder.getInt32(64));
    catchPadArgs.push_back(llvm::Constant::getNullValue(builder.getInt8PtrTy()));
    llvm::CatchPadInst* catchPad = builder.CreateCatchPad(currentPad->value, catchPadArgs);
    currentPad->value = catchPad;
    llvm::BasicBlock* catchTarget = llvm::BasicBlock::Create(context, "catch", function);
    llvm::BasicBlock* resumeTarget = llvm::BasicBlock::Create(context, "resume", function);
    builder.CreateBr(catchTarget);
    int n = boundTryStatement.Catches().size();
    for (int i = 0; i < n; ++i)
    {
        const std::unique_ptr<BoundCatchStatement>& boundCatchStatement = boundTryStatement.Catches()[i];
        uint32_t catchTypeId = boundCatchStatement->CatchedType()->BaseType()->TypeId();
        SetCurrentBasicBlock(catchTarget);
        std::vector<llvm::Type*> handleExceptionParamTypes;
        handleExceptionParamTypes.push_back(builder.getInt32Ty());
        llvm::FunctionType* handleExceptionFunctionType = llvm::FunctionType::get(builder.getInt1Ty(), handleExceptionParamTypes, false);
        ArgVector handleExceptionArgs;
        handleExceptionArgs.push_back(builder.getInt32(catchTypeId));
        llvm::Function* handleException = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction("RtHandleException", handleExceptionFunctionType));
        llvm::Value* handleThisEx = nullptr;
        if (currentPad == nullptr)
        {
            handleThisEx = builder.CreateCall(handleException, handleExceptionArgs);
        }
        else
        {
            std::vector<llvm::OperandBundleDef> bundles;
            std::vector<llvm::Value*> inputs;
            inputs.push_back(currentPad->value);
            bundles.push_back(llvm::OperandBundleDef("funclet", inputs));
            handleThisEx = llvm::CallInst::Create(handleException, handleExceptionArgs, bundles, "", CurrentBasicBlock());
        }
        llvm::BasicBlock* nextHandlerTarget = nullptr;
        if (i < n - 1)
        {
            catchTarget = llvm::BasicBlock::Create(context, "catch", function);
            nextHandlerTarget = catchTarget;
        }
        else
        {
            nextHandlerTarget = resumeTarget;
        }
        llvm::BasicBlock* thisHandlerTarget = llvm::BasicBlock::Create(context, "handler", function);
        builder.CreateCondBr(handleThisEx, thisHandlerTarget, nextHandlerTarget);
        SetCurrentBasicBlock(thisHandlerTarget);
        boundCatchStatement->CatchBlock()->Accept(*this);
        builder.CreateCatchRet(llvm::cast<llvm::CatchPadInst>(currentPad->value), nextTarget);
    }
    SetCurrentBasicBlock(resumeTarget);
    llvm::Function* cxxThrowFunction = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction("_CxxThrowException", 
        builder.getVoidTy(), builder.getInt8PtrTy(), builder.getInt8PtrTy(), nullptr));
    ArgVector rethrowArgs;
    rethrowArgs.push_back(llvm::Constant::getNullValue(builder.getInt8PtrTy()));
    rethrowArgs.push_back(llvm::Constant::getNullValue(builder.getInt8PtrTy()));
    std::vector<llvm::OperandBundleDef> bundles;
    std::vector<llvm::Value*> inputs;
    inputs.push_back(currentPad->value);
    bundles.push_back(llvm::OperandBundleDef("funclet", inputs));
    llvm::CallInst::Create(cxxThrowFunction, rethrowArgs, bundles, "", resumeTarget);
    builder.CreateBr(nextTarget);
    currentPad = parentPad;
    SetCurrentBasicBlock(nextTarget);
    basicBlockOpen = true;
    cleanupBlock = prevCleanupBlock;
}

void Emitter::Visit(BoundParameter& boundParameter)
{
    boundParameter.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundLocalVariable& boundLocalVariable)
{
    boundLocalVariable.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundMemberVariable& boundMemberVariable)
{
    boundMemberVariable.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundConstant& boundConstant)
{
    boundConstant.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundEnumConstant& boundEnumConstant)
{
    boundEnumConstant.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundLiteral& boundLiteral)
{
    boundLiteral.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundTemporary& boundTemporary)
{
    boundTemporary.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundSizeOfExpression& boundSizeOfExpression)
{
    boundSizeOfExpression.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundAddressOfExpression& boundAddressOfExpression)
{
    boundAddressOfExpression.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundDereferenceExpression& boundDereferenceExpression)
{
    boundDereferenceExpression.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundReferenceToPointerExpression& boundReferenceToPointerExpression)
{
    boundReferenceToPointerExpression.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundFunctionCall& boundFunctionCall)
{
    boundFunctionCall.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundDelegateCall& boundDelegateCall)
{
    boundDelegateCall.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundClassDelegateCall& boundClassDelegateCall)
{
    boundClassDelegateCall.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundConversion& boundConversion)
{
    boundConversion.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundConstructExpression& boundConstructExpression)
{
    boundConstructExpression.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundConstructAndReturnTemporaryExpression& boundConstructAndReturnTemporaryExpression)
{
    boundConstructAndReturnTemporaryExpression.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundClassOrClassDelegateConversionResult& boundClassOrClassDelegateConversionResult)
{
    boundClassOrClassDelegateConversionResult.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundIsExpression& boundIsExpression)
{
    boundIsExpression.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void Emitter::Visit(BoundAsExpression& boundAsExpression)
{
    boundAsExpression.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundTypeNameExpression& boundTypeNameExpression)
{
    boundTypeNameExpression.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundBitCast& boundBitCast)
{
    boundBitCast.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundFunctionPtr& boundFunctionPtr)
{
    boundFunctionPtr.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundDisjunction& boundDisjunction)
{
    if (genJumpingBoolCode)
    {
        Assert(trueBlock, "true block not set");
        Assert(falseBlock, "false block not set");
        llvm::BasicBlock* rightBlock = llvm::BasicBlock::Create(context, "right", function);
        llvm::BasicBlock* prevFalseBlock = falseBlock;
        falseBlock = rightBlock;
        boundDisjunction.Left()->Accept(*this);
        SetCurrentBasicBlock(rightBlock);
        falseBlock = prevFalseBlock;
        boundDisjunction.Right()->Accept(*this);
        boundDisjunction.DestroyTemporaries(*this);
    }
}

void Emitter::Visit(BoundConjunction& boundConjunction)
{
    if (genJumpingBoolCode)
    {
        Assert(trueBlock, "true block not set");
        Assert(falseBlock, "false block not set");
        llvm::BasicBlock* rightBlock = llvm::BasicBlock::Create(context, "right", function);
        llvm::BasicBlock* prevTrueBlock = trueBlock;
        trueBlock = rightBlock;
        boundConjunction.Left()->Accept(*this);
        trueBlock = prevTrueBlock;
        SetCurrentBasicBlock(rightBlock);
        boundConjunction.Right()->Accept(*this);
        boundConjunction.DestroyTemporaries(*this);
    }
}

llvm::Value* Emitter::GetGlobalStringPtr(int stringId)
{
    auto it = utf8stringMap.find(stringId);
    if (it != utf8stringMap.cend())
    {
        return it->second;
    }
    else
    {
        llvm::Value* stringValue = builder.CreateGlobalStringPtr(compileUnit->GetUtf8String(stringId));
        utf8stringMap[stringId] = stringValue;
        return stringValue;
    }
}

llvm::Value* Emitter::GetGlobalWStringConstant(int stringId)
{
    auto it = utf16stringMap.find(stringId);
    if (it != utf16stringMap.cend())
    {
        return it->second;
    }
    else
    {
        const std::u16string& str = compileUnit->GetUtf16String(stringId);
        uint64_t length = str.length();
        std::vector<llvm::Constant*> wcharConstants;
        for (char16_t c : str)
        {
            wcharConstants.push_back(builder.getInt16(static_cast<uint16_t>(c)));
        }
        wcharConstants.push_back(builder.getInt16(static_cast<uint32_t>(0)));
        llvm::Constant* stringObject = compileUnitModule->getOrInsertGlobal("wstring" + std::to_string(stringId), llvm::ArrayType::get(builder.getInt16Ty(), length + 1));
        llvm::GlobalVariable* stringGlobal = llvm::cast<llvm::GlobalVariable>(stringObject);
        stringGlobal->setLinkage(llvm::GlobalValue::PrivateLinkage);
        llvm::Constant* constant = llvm::ConstantArray::get(llvm::ArrayType::get(builder.getInt16Ty(), length + 1), wcharConstants);
        stringGlobal->setInitializer(constant);
        llvm::Value* stringValue = stringGlobal;
        utf16stringMap[stringId] = stringValue;
        return stringValue;
    }
}

llvm::Value* Emitter::GetGlobalUStringConstant(int stringId)
{
    auto it = utf32stringMap.find(stringId);
    if (it != utf32stringMap.cend())
    {
        return it->second;
    }
    else
    {
        const std::u32string& str = compileUnit->GetUtf32String(stringId);
        uint64_t length = str.length();
        std::vector<llvm::Constant*> ucharConstants;
        for (char32_t c : str)
        {
            ucharConstants.push_back(builder.getInt32(static_cast<uint32_t>(c)));
        }
        ucharConstants.push_back(builder.getInt32(static_cast<uint32_t>(0)));
        llvm::Constant* stringObject = compileUnitModule->getOrInsertGlobal("ustring" + std::to_string(stringId), llvm::ArrayType::get(builder.getInt32Ty(), length + 1));
        llvm::GlobalVariable* stringGlobal = llvm::cast<llvm::GlobalVariable>(stringObject);
        stringGlobal->setLinkage(llvm::GlobalValue::PrivateLinkage);
        llvm::Constant* constant = llvm::ConstantArray::get(llvm::ArrayType::get(builder.getInt32Ty(), length + 1), ucharConstants);
        stringGlobal->setInitializer(constant);
        llvm::Value* stringValue = stringGlobal;
        utf32stringMap[stringId] = stringValue;
        return stringValue;
    }
}

void Emitter::CreateExitFunctionCall()
{
    if (currentFunction->GetFunctionSymbol()->DontThrow()) return;
    std::vector<llvm::Type*> exitFunctionParamTypes;
    llvm::FunctionType* exitFunctionType = llvm::FunctionType::get(builder.getVoidTy(), exitFunctionParamTypes, false);
    llvm::Function* exitFunction = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction("RtExitFunction", exitFunctionType));
    ArgVector exitFunctionArgs;
    if (currentPad == nullptr)
    {
        builder.CreateCall(exitFunction, exitFunctionArgs);
    }
    else
    {
        std::vector<llvm::OperandBundleDef> bundles;
        std::vector<llvm::Value*> inputs;
        inputs.push_back(currentPad->value);
        bundles.push_back(llvm::OperandBundleDef("funclet", inputs));
        llvm::CallInst::Create(exitFunction, exitFunctionArgs, bundles, "", CurrentBasicBlock());
    }
}

void Emitter::SetLineNumber(int32_t lineNumber)
{
    if (currentFunction->GetFunctionSymbol()->DontThrow()) return;
    if (prevLineNumber == lineNumber) return;
    prevLineNumber = lineNumber;
    std::vector<llvm::Type*> setLineNumberFunctionParamTypes;
    setLineNumberFunctionParamTypes.push_back(builder.getInt32Ty());
    llvm::FunctionType* setLineNumberFunctionType = llvm::FunctionType::get(builder.getVoidTy(), setLineNumberFunctionParamTypes, false);
    llvm::Function* setLineNumberFunction = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction("RtSetLineNumber", setLineNumberFunctionType));
    ArgVector setLineNumberFunctionArgs;
    setLineNumberFunctionArgs.push_back(builder.getInt32(lineNumber));
    if (currentPad == nullptr)
    {
        builder.CreateCall(setLineNumberFunction, setLineNumberFunctionArgs);
    }
    else
    {
        std::vector<llvm::OperandBundleDef> bundles;
        std::vector<llvm::Value*> inputs;
        inputs.push_back(currentPad->value);
        bundles.push_back(llvm::OperandBundleDef("funclet", inputs));
        llvm::CallInst::Create(setLineNumberFunction, setLineNumberFunctionArgs, bundles, "", CurrentBasicBlock());
    }
}

void Emitter::CreateCleanup()
{
    cleanupBlock = llvm::BasicBlock::Create(context, "cleanup", function);
    BoundCompoundStatement* targetBlock = nullptr;
    BoundStatement* parent = currentBlock->Parent();
    while (parent && parent->GetBoundNodeType() != BoundNodeType::boundTryStatement)
    {
        parent = parent->Parent();
    }
    if (parent)
    {
        targetBlock = parent->Block();
    }
    Cleanup* cleanup = new Cleanup(cleanupBlock, handlerBlock, currentPad);
    int n = blocks.size();
    for (int i = n - 1; i >= 0; --i)
    {
        BoundCompoundStatement* block = blocks[i];
        if (block == targetBlock)
        {
            break;
        }
        auto it = blockDestructionMap.find(block);
        if (it != blockDestructionMap.cend())
        {
            std::vector<std::unique_ptr<BoundFunctionCall>>& destructorCallVec = it->second;
            int nd = destructorCallVec.size();
            for (int i = nd - 1; i >= 0; --i)
            {
                std::unique_ptr<BoundFunctionCall>& destructorCall = destructorCallVec[i];
                if (destructorCall)
                {
                    cleanup->destructors.push_back(std::unique_ptr<BoundFunctionCall>(static_cast<BoundFunctionCall*>(destructorCall->Clone())));
                }
            }
        }
    }
    cleanups.push_back(std::unique_ptr<Cleanup>(cleanup));
    newCleanupNeeded = false;
}

void Emitter::GenerateCodeForCleanups()
{
    for (const std::unique_ptr<Cleanup>& cleanup : cleanups)
    {
        SetCurrentBasicBlock(cleanup->cleanupBlock);
        Pad* parentPad = cleanup->currentPad;
        llvm::CleanupPadInst* cleanupPad = nullptr;
        if (parentPad)
        {
            ArgVector args;
            cleanupPad = builder.CreateCleanupPad(parentPad->value, args);
        }
        else
        {
            ArgVector args;
            cleanupPad = builder.CreateCleanupPad(llvm::ConstantTokenNone::get(context), args);
        }
        Pad pad;
        pad.parent = parentPad;
        pad.value = cleanupPad;
        currentPad = &pad;
        for (const std::unique_ptr<BoundFunctionCall>& destructorCall : cleanup->destructors)
        {
            destructorCall->Accept(*this);
        }
        llvm::BasicBlock* unwindTarget = cleanup->handlerBlock;
        builder.CreateCleanupRet(llvm::cast<llvm::CleanupPadInst>(cleanupPad), unwindTarget);
    }
}

void GenerateCode(EmittingContext& emittingContext, BoundCompileUnit& boundCompileUnit)
{
    Emitter emitter(emittingContext, boundCompileUnit.GetCompileUnitNode()->FilePath(), boundCompileUnit.GetModule());
    boundCompileUnit.Accept(emitter);
}

} } // namespace cmajor::emitter
