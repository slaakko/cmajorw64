// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/emitter/BasicEmitter.hpp>
#include <cmajor/emitter/EmittingContextImpl.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/symbols/InterfaceTypeSymbol.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/System.hpp>
#include <cmajor/util/TextUtils.hpp>
#include <fstream>

namespace cmajor { namespace emitter {

using namespace cmajor::unicode;

BasicEmitter::BasicEmitter(EmittingContext& emittingContext_, const std::string& compileUnitModuleName_, cmajor::symbols::Module& symbolsModule_) :
    cmajor::ir::Emitter(emittingContext_.GetEmittingContextImpl()->Context()), emittingContext(emittingContext_), symbolTable(nullptr),
    compileUnitModule(new llvm::Module(compileUnitModuleName_, emittingContext.GetEmittingContextImpl()->Context())), symbolsModule(symbolsModule_), builder(Builder()), stack(Stack()),
    context(emittingContext.GetEmittingContextImpl()->Context()), compileUnit(nullptr), function(nullptr), trueBlock(nullptr), falseBlock(nullptr), breakTarget(nullptr), continueTarget(nullptr),
    handlerBlock(nullptr), cleanupBlock(nullptr), newCleanupNeeded(false), currentPad(nullptr), genJumpingBoolCode(false), currentClass(nullptr), currentFunction(nullptr),
    currentBlock(nullptr), breakTargetBlock(nullptr), continueTargetBlock(nullptr), sequenceSecond(nullptr), currentCaseMap(nullptr), defaultDest(nullptr), prevLineNumber(0), destructorCallGenerated(false),
    lastInstructionWasRet(false), basicBlockOpen(false)
{
    compileUnitModule->setTargetTriple(emittingContext.GetEmittingContextImpl()->TargetTriple());
    compileUnitModule->setDataLayout(emittingContext.GetEmittingContextImpl()->DataLayout());
    compileUnitModule->setSourceFileName(compileUnitModuleName_);
    SetModule(compileUnitModule.get());
}

void BasicEmitter::GenJumpingBoolCode()
{
    if (!genJumpingBoolCode) return;
    Assert(trueBlock, "true block not set");
    Assert(falseBlock, "false block not set");
    llvm::Value* cond = stack.Pop();
    if (sequenceSecond)
    {
        genJumpingBoolCode = false;
        sequenceSecond->SetGenerated();
        sequenceSecond->Accept(*this);
        genJumpingBoolCode = true;
    }
    builder.CreateCondBr(cond, trueBlock, falseBlock);
}

void BasicEmitter::SetTarget(BoundStatement* labeledStatement)
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

void BasicEmitter::ClearFlags()
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
}

void BasicEmitter::Visit(BoundCompileUnit& boundCompileUnit)
{
    compileUnit = &boundCompileUnit;
    symbolTable = &boundCompileUnit.GetSymbolTable();
    ConstantArrayRepository& constantArrayRepository = boundCompileUnit.GetConstantArrayRepository();
    for (ConstantSymbol* constantSymbol : constantArrayRepository.ConstantArrays())
    {
        constantSymbol->ArrayIrObject(*this, true);
    }
    ConstantStructureRepository& constantStructureRepository = boundCompileUnit.GetConstantStructureRepository();
    for (ConstantSymbol* constantSymbol : constantStructureRepository.ConstantStructures())
    {
        constantSymbol->StructureIrObject(*this, true);
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
        throw std::runtime_error("Emitter: verification of module '" + compileUnitModule->getSourceFileName() + "' failed. " + errorMessage.str());
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

void BasicEmitter::Visit(BoundClass& boundClass)
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

void BasicEmitter::Visit(BoundFunction& boundFunction)
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
    if (GetGlobalFlag(GlobalFlags::release) && functionSymbol->IsInline())
    {
        function->addFnAttr(llvm::Attribute::InlineHint);
        functionSymbol->SetLinkOnceOdrLinkage();
    }
    if (functionSymbol->HasLinkOnceOdrLinkage())
    {
        llvm::Comdat* comdat = compileUnitModule->getOrInsertComdat(ToUtf8(functionSymbol->MangledName()));
        comdat->setSelectionKind(llvm::Comdat::SelectionKind::Any);
        function->setLinkage(llvm::GlobalValue::LinkageTypes::LinkOnceODRLinkage);
        function->setComdat(comdat);
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
        llvm::Function* personalityFunction = GetPersonalityFunction();
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

void BasicEmitter::Visit(BoundSequenceStatement& boundSequenceStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundSequenceStatement);
    BoundStatement* prevSequence = sequenceSecond;
    sequenceSecond = boundSequenceStatement.Second();
    boundSequenceStatement.First()->Accept(*this);
    sequenceSecond = prevSequence;
    if (!boundSequenceStatement.Second()->Generated())
    {
        boundSequenceStatement.Second()->Accept(*this);
    }
}

void BasicEmitter::ExitBlocks(BoundCompoundStatement* targetBlock)
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
                    newCleanupNeeded = true;
                }
            }
        }
    }
}

void BasicEmitter::Visit(BoundCompoundStatement& boundCompoundStatement)
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

void BasicEmitter::Visit(BoundIfStatement& boundIfStatement)
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

void BasicEmitter::Visit(BoundWhileStatement& boundWhileStatement)
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

void BasicEmitter::Visit(BoundDoStatement& boundDoStatement)
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

void BasicEmitter::Visit(BoundForStatement& boundForStatement)
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

void BasicEmitter::Visit(BoundSwitchStatement& boundSwitchStatement)
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

void BasicEmitter::Visit(BoundCaseStatement& boundCaseStatement)
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

void BasicEmitter::Visit(BoundDefaultStatement& boundDefaultStatement)
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

void BasicEmitter::Visit(BoundConstructionStatement& boundConstructionStatement)
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

void BasicEmitter::Visit(BoundAssignmentStatement& boundAssignmentStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundAssignmentStatement);
    boundAssignmentStatement.AssignmentCall()->Accept(*this);
}

void BasicEmitter::Visit(BoundExpressionStatement& boundExpressionStatement)
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

void BasicEmitter::Visit(BoundEmptyStatement& boundEmptyStatement)
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

void BasicEmitter::Visit(BoundSetVmtPtrStatement& boundSetVmtPtrStatement)
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

void BasicEmitter::Visit(BoundThrowStatement& boundThrowStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundThrowStatement);
    boundThrowStatement.ThrowCallExpr()->Accept(*this);
}

void BasicEmitter::Visit(BoundParameter& boundParameter)
{
    boundParameter.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundLocalVariable& boundLocalVariable)
{
    boundLocalVariable.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundMemberVariable& boundMemberVariable)
{
    boundMemberVariable.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundConstant& boundConstant)
{
    boundConstant.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundEnumConstant& boundEnumConstant)
{
    boundEnumConstant.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundLiteral& boundLiteral)
{
    boundLiteral.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundTemporary& boundTemporary)
{
    boundTemporary.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundSizeOfExpression& boundSizeOfExpression)
{
    boundSizeOfExpression.Load(*this, OperationFlags::none);
}

void BasicEmitter::Visit(BoundAddressOfExpression& boundAddressOfExpression)
{
    boundAddressOfExpression.Load(*this, OperationFlags::none);
}

void BasicEmitter::Visit(BoundDereferenceExpression& boundDereferenceExpression)
{
    boundDereferenceExpression.Load(*this, OperationFlags::none);
}

void BasicEmitter::Visit(BoundReferenceToPointerExpression& boundReferenceToPointerExpression)
{
    boundReferenceToPointerExpression.Load(*this, OperationFlags::none);
}

void BasicEmitter::Visit(BoundFunctionCall& boundFunctionCall)
{
    boundFunctionCall.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundDelegateCall& boundDelegateCall)
{
    boundDelegateCall.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundClassDelegateCall& boundClassDelegateCall)
{
    boundClassDelegateCall.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundConversion& boundConversion)
{
    boundConversion.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundConstructExpression& boundConstructExpression)
{
    boundConstructExpression.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundConstructAndReturnTemporaryExpression& boundConstructAndReturnTemporaryExpression)
{
    boundConstructAndReturnTemporaryExpression.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundClassOrClassDelegateConversionResult& boundClassOrClassDelegateConversionResult)
{
    boundClassOrClassDelegateConversionResult.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundIsExpression& boundIsExpression)
{
    boundIsExpression.Load(*this, OperationFlags::none);
    GenJumpingBoolCode();
}

void BasicEmitter::Visit(BoundAsExpression& boundAsExpression)
{
    boundAsExpression.Load(*this, OperationFlags::none);
}

void BasicEmitter::Visit(BoundTypeNameExpression& boundTypeNameExpression)
{
    boundTypeNameExpression.Load(*this, OperationFlags::none);
}

void BasicEmitter::Visit(BoundBitCast& boundBitCast)
{
    boundBitCast.Load(*this, OperationFlags::none);
}

void BasicEmitter::Visit(BoundFunctionPtr& boundFunctionPtr)
{
    boundFunctionPtr.Load(*this, OperationFlags::none);
}

void BasicEmitter::Visit(BoundDisjunction& boundDisjunction)
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

void BasicEmitter::Visit(BoundConjunction& boundConjunction)
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

llvm::Value* BasicEmitter::GetGlobalStringPtr(int stringId)
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

llvm::Value* BasicEmitter::GetGlobalWStringConstant(int stringId)
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

llvm::Value* BasicEmitter::GetGlobalUStringConstant(int stringId)
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

void BasicEmitter::CreateExitFunctionCall()
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

void BasicEmitter::SetLineNumber(int32_t lineNumber)
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

} } // namespace cmajor::emitter
