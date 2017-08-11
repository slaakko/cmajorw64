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
#include <cmajor/ir/Emitter.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/util/Unicode.hpp>
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

class Emitter : public cmajor::ir::Emitter, public BoundNodeVisitor
{
public:
    Emitter(EmittingContext& emittingContext_, const std::string& compileUnitModuleName_);
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
    void Visit(BoundBreakStatement& boundBreakStatement) override;
    void Visit(BoundContinueStatement& boundContinueStatement) override;
    void Visit(BoundGotoStatement& boundGotoStatement) override;
    void Visit(BoundConstructionStatement& boundConstructionStatement) override;
    void Visit(BoundAssignmentStatement& boundAssignmentStatement) override;
    void Visit(BoundExpressionStatement& boundExpressionStatement) override;
    void Visit(BoundEmptyStatement& boundEmptyStatement) override;
    void Visit(BoundSetVmtPtrStatement& boundSetVmtPtrStatement) override;
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
    void Visit(BoundConversion& boundConversion) override;
    void Visit(BoundConstructExpression& boundConstructExpression) override;
    void Visit(BoundIsExpression& boundIsExpression) override;
    void Visit(BoundAsExpression& boundAsExpression) override;
    void Visit(BoundTypeNameExpression& boundTypeNameExpression) override;
    void Visit(BoundBitCast& boundBitCast) override;
    void Visit(BoundFunctionPtr& boundFunctionPtr) override;
    llvm::Value* GetGlobalStringPtr(int stringId) override;
private:
    EmittingContext& emittingContext;
    SymbolTable* symbolTable;
    std::unique_ptr<llvm::Module> compileUnitModule;
    llvm::IRBuilder<>& builder;
    cmajor::ir::ValueStack& stack;
    llvm::LLVMContext& context;
    llvm::Function* function;
    llvm::BasicBlock* trueBlock;
    llvm::BasicBlock* falseBlock;
    BoundCompileUnit* compileUnit;
    BoundClass* currentClass;
    BoundFunction* currentFunction;
    BoundCompoundStatement* currentBlock;
    std::stack<BoundClass*> classStack;
    std::vector<BoundCompoundStatement*> blocks;
    std::unordered_map<BoundCompoundStatement*, std::vector<std::unique_ptr<BoundFunctionCall>>> blockDestructionMap;
    std::unordered_map<int, llvm::Value*> stringMap;
    int prevLineNumber;
    void ExitBlocks(BoundCompoundStatement* targetBlock);
    void CreateExitFunctionCall();
    void SetLineNumber(int32_t lineNumber) override;
};

Emitter::Emitter(EmittingContext& emittingContext_, const std::string& compileUnitModuleName_) :
    cmajor::ir::Emitter(emittingContext_.GetEmittingContextImpl()->Context()), emittingContext(emittingContext_), symbolTable(nullptr),
    compileUnitModule(new llvm::Module(compileUnitModuleName_, emittingContext.GetEmittingContextImpl()->Context())), builder(Builder()), stack(Stack()), 
    context(emittingContext.GetEmittingContextImpl()->Context()), compileUnit(nullptr), function(nullptr), trueBlock(nullptr), falseBlock(nullptr), 
    currentClass(nullptr), currentFunction(nullptr), currentBlock(nullptr), prevLineNumber(0)
{
    compileUnitModule->setTargetTriple(emittingContext.GetEmittingContextImpl()->TargetTriple());
    compileUnitModule->setDataLayout(emittingContext.GetEmittingContextImpl()->DataLayout());
    compileUnitModule->setSourceFileName(compileUnitModuleName_);
    SetModule(compileUnitModule.get());
}

void Emitter::Visit(BoundCompileUnit& boundCompileUnit)
{
    compileUnit = &boundCompileUnit;
    symbolTable = &boundCompileUnit.GetSymbolTable();
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
        std::ofstream optLlFile(boundCompileUnit.OptLLFilePath());
        llvm::raw_os_ostream optLlOs(optLlFile);
        compileUnitModule->print(optLlOs, nullptr);
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
    prevLineNumber = 0;
    FunctionSymbol* functionSymbol = boundFunction.GetFunctionSymbol();
    llvm::FunctionType* functionType = functionSymbol->IrType(*this);
    function = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction(ToUtf8(functionSymbol->MangledName()), functionType));
    if (functionSymbol->HasWeakOdrLinkage())
    {
        function->setLinkage(llvm::GlobalValue::LinkageTypes::WeakODRLinkage);
    }
    if (functionSymbol->DontThrow())
    {
        function->addFnAttr(llvm::Attribute::NoUnwind);
    }
    else
    {
        function->addFnAttr(llvm::Attribute::UWTable);
    }
    SetFunction(function);
    llvm::BasicBlock* entryBlock = llvm::BasicBlock::Create(context, "entry", function);
    builder.SetInsertPoint(entryBlock);
    if (currentClass)
    {
        ClassTypeSymbol* classTypeSymbol = currentClass->GetClassTypeSymbol();
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
        builder.CreateStore(&*it, parameter->IrObject());
        ++it;
    }
    BoundCompoundStatement* body = boundFunction.Body();
    body->Accept(*this);
    BoundStatement* lastStatement = nullptr;
    if (!body->Statements().empty())
    {
        lastStatement = body->Statements().back().get();
    }
    if (!lastStatement || lastStatement->GetBoundNodeType() != BoundNodeType::boundReturnStatement)
    {
        CreateExitFunctionCall();
        if (functionSymbol->ReturnType() && functionSymbol->ReturnType()->GetSymbolType() != SymbolType::voidTypeSymbol)
        {
            llvm::Value* defaultValue = functionSymbol->ReturnType()->CreateDefaultIrValue(*this);
            builder.CreateRet(defaultValue);
        }
        else
        {
            builder.CreateRetVoid();
        }
    }
}

void Emitter::Visit(BoundSequenceStatement& boundSequenceStatement)
{
    boundSequenceStatement.First()->Accept(*this);
    boundSequenceStatement.Second()->Accept(*this);
}

void Emitter::ExitBlocks(BoundCompoundStatement* targetBlock)
{
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
                    destructorCall->Accept(*this);
                }
            }
        }
    }
}

void Emitter::Visit(BoundCompoundStatement& boundCompoundStatement)
{
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
    ExitBlocks(nullptr);
    CreateExitFunctionCall();
    BoundFunctionCall* returnFunctionCall = boundReturnStatement.ReturnFunctionCall();
    if (returnFunctionCall)
    {
        returnFunctionCall->Accept(*this);
        llvm::Value* returnValue = stack.Pop();
        builder.CreateRet(returnValue);
    }
    else
    {
        builder.CreateRetVoid();
    }
    BoundCompoundStatement* body = currentFunction->Body();
    BoundStatement* lastStatement = nullptr;
    if (!body->Statements().empty())
    {
        lastStatement = body->Statements().back().get();
    }
    if (lastStatement && lastStatement != &boundReturnStatement)
    {
        llvm::BasicBlock* continueBlock = llvm::BasicBlock::Create(context, "continue", function);
        builder.SetInsertPoint(continueBlock);
    }
}

void Emitter::Visit(BoundIfStatement& boundIfStatement)
{
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
    boundIfStatement.Condition()->Accept(*this);
    llvm::Value* cond = stack.Pop();
    builder.CreateCondBr(cond, trueBlock, falseBlock);
    builder.SetInsertPoint(trueBlock);
    boundIfStatement.ThenS()->Accept(*this);
    builder.CreateBr(nextBlock);
    if (boundIfStatement.ElseS())
    {
        builder.SetInsertPoint(falseBlock);
        boundIfStatement.ElseS()->Accept(*this);
        builder.CreateBr(nextBlock);
    }
    trueBlock = prevTrueBlock;
    falseBlock = prevFalseBlock;
    builder.SetInsertPoint(nextBlock);
}

void Emitter::Visit(BoundWhileStatement& boundWhileStatement)
{
    llvm::BasicBlock* prevTrueBlock = trueBlock;
    llvm::BasicBlock* prevFalseBlock = falseBlock;
    trueBlock = llvm::BasicBlock::Create(context, "true", function);
    falseBlock = llvm::BasicBlock::Create(context, "next", function);
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(context, "cond", function);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(condBlock);
    boundWhileStatement.Condition()->Accept(*this);
    llvm::Value* cond = stack.Pop();
    builder.CreateCondBr(cond, trueBlock, falseBlock);
    builder.SetInsertPoint(trueBlock);
    boundWhileStatement.Statement()->Accept(*this);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(falseBlock);
    trueBlock = prevTrueBlock;
    falseBlock = prevFalseBlock;
}

void Emitter::Visit(BoundDoStatement& boundDoStatement)
{
    llvm::BasicBlock* prevTrueBlock = trueBlock;
    llvm::BasicBlock* prevFalseBlock = falseBlock;
    llvm::BasicBlock* doBlock = llvm::BasicBlock::Create(context, "do", function);
    falseBlock = llvm::BasicBlock::Create(context, "next", function);
    builder.CreateBr(doBlock);
    builder.SetInsertPoint(doBlock);
    boundDoStatement.Statement()->Accept(*this);
    boundDoStatement.Condition()->Accept(*this);
    llvm::Value* cond = stack.Pop();
    builder.CreateCondBr(cond, doBlock, falseBlock);
    builder.SetInsertPoint(falseBlock);
    trueBlock = prevTrueBlock;
    falseBlock = prevFalseBlock;
}

void Emitter::Visit(BoundForStatement& boundForStatement)
{
    llvm::BasicBlock* prevTrueBlock = trueBlock;
    llvm::BasicBlock* prevFalseBlock = falseBlock;
    boundForStatement.InitS()->Accept(*this);
    llvm::BasicBlock* condBlock = llvm::BasicBlock::Create(context, "cond", function);
    llvm::BasicBlock* actionBlock = llvm::BasicBlock::Create(context, "action", function);
    falseBlock = llvm::BasicBlock::Create(context, "next", function);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(condBlock);
    boundForStatement.Condition()->Accept(*this);
    llvm::Value* cond = stack.Pop();
    builder.CreateCondBr(cond, actionBlock, falseBlock);
    builder.SetInsertPoint(actionBlock);
    boundForStatement.ActionS()->Accept(*this);
    boundForStatement.LoopS()->Accept(*this);
    builder.CreateBr(condBlock);
    builder.SetInsertPoint(falseBlock);
    trueBlock = prevTrueBlock;
    falseBlock = prevFalseBlock;
}

void Emitter::Visit(BoundBreakStatement& boundBreakStatement)
{

}

void Emitter::Visit(BoundContinueStatement& boundContinueStatement)
{

}

void Emitter::Visit(BoundGotoStatement& boundGotoStatement)
{

}

void Emitter::Visit(BoundConstructionStatement& boundConstructionStatement)
{
    boundConstructionStatement.ConstructorCall()->Accept(*this);
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

void Emitter::Visit(BoundAssignmentStatement& boundAssignmentStatement)
{
    boundAssignmentStatement.AssignmentCall()->Accept(*this);
}

void Emitter::Visit(BoundExpressionStatement& boundExpressionStatement)
{
    boundExpressionStatement.Expression()->Accept(*this);
    if (boundExpressionStatement.Expression()->HasValue())
    {
        stack.Pop();
    }
}

void Emitter::Visit(BoundEmptyStatement& boundEmptyStatement)
{
    llvm::Type* retType = llvm::Type::getVoidTy(context);
    std::vector<llvm::Type*> paramTypes;
    llvm::FunctionType* doNothingFunType = llvm::FunctionType::get(retType, paramTypes, false);
    llvm::Function* doNothingFun = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction("llvm.donothing", doNothingFunType));
    ArgVector args;
    builder.CreateCall(doNothingFunType, doNothingFun, args);
}

void Emitter::Visit(BoundSetVmtPtrStatement& boundSetVmtPtrStatement)
{
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

void Emitter::Visit(BoundParameter& boundParameter)
{
    boundParameter.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundLocalVariable& boundLocalVariable)
{
    boundLocalVariable.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundMemberVariable& boundMemberVariable)
{
    boundMemberVariable.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundConstant& boundConstant)
{
    boundConstant.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundEnumConstant& boundEnumConstant)
{
    boundEnumConstant.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundLiteral& boundLiteral)
{
    boundLiteral.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundTemporary& boundTemporary)
{
    boundTemporary.Load(*this, OperationFlags::none);
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
}

void Emitter::Visit(BoundConversion& boundConversion)
{
    boundConversion.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundConstructExpression& boundConstructExpression)
{
    boundConstructExpression.Load(*this, OperationFlags::none);
}

void Emitter::Visit(BoundIsExpression& boundIsExpression)
{
    boundIsExpression.Load(*this, OperationFlags::none);
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

llvm::Value* Emitter::GetGlobalStringPtr(int stringId)
{
    auto it = stringMap.find(stringId);
    if (it != stringMap.cend())
    {
        return it->second;
    }
    else
    {
        llvm::Value* stringValue = builder.CreateGlobalStringPtr(compileUnit->GetString(stringId));
        stringMap[stringId] = stringValue;
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
    builder.CreateCall(exitFunction, exitFunctionArgs);
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
    builder.CreateCall(setLineNumberFunction, setLineNumberFunctionArgs);
}

void GenerateCode(EmittingContext& emittingContext, BoundCompileUnit& boundCompileUnit)
{
    Emitter emitter(emittingContext, boundCompileUnit.GetCompileUnitNode()->FilePath());
    boundCompileUnit.Accept(emitter);
}

} } // namespace cmajor::emitter
