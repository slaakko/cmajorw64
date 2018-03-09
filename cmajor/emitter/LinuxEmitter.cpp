// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/emitter/LinuxEmitter.hpp>

namespace cmajor { namespace emitter {

LinuxEmitter::LinuxEmitter(EmittingContext& emittingContext_, const std::string& compileUnitModuleName_, cmajor::symbols::Module& symbolsModule_) : 
    BasicEmitter(emittingContext_, compileUnitModuleName_, symbolsModule_)
{
}

llvm::Function* LinuxEmitter::GetPersonalityFunction() const
{
    llvm::FunctionType* personalityFunctionType = llvm::FunctionType::get(builder.getInt32Ty(), true);
    llvm::Function* personalityFunction = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction("__gxx_personality_v0", personalityFunctionType));
    return personalityFunction;
}

void LinuxEmitter::Visit(BoundReturnStatement& boundReturnStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundReturnStatement);
    BoundFunctionCall* returnFunctionCall = boundReturnStatement.ReturnFunctionCall();
    if (returnFunctionCall)
    {
        returnFunctionCall->Accept(*this);
        llvm::Value* returnValue = stack.Pop();
        if (sequenceSecond)
        {
            sequenceSecond->SetGenerated();
            sequenceSecond->Accept(*this);
        }
        ExitBlocks(nullptr);
        CreateExitFunctionCall();
        builder.CreateRet(returnValue);
        lastInstructionWasRet = true;
    }
    else
    {
        ExitBlocks(nullptr);
        CreateExitFunctionCall();
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
}

void LinuxEmitter::Visit(BoundGotoCaseStatement& boundGotoCaseStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundGotoCaseStatement);
    Assert(breakTargetBlock, "break target not set");
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
}

void LinuxEmitter::Visit(BoundGotoDefaultStatement& boundGotoDefaultStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundGotoDefaultStatement);
    Assert(breakTargetBlock, "break target not set");
    ExitBlocks(breakTargetBlock);
    if (defaultDest)
    {
        builder.CreateBr(defaultDest);
    }
    else
    {
        throw Exception("no default destination", boundGotoDefaultStatement.GetSpan());
    }
}

void LinuxEmitter::Visit(BoundBreakStatement& boundBreakStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundBreakStatement);
    Assert(breakTarget && breakTargetBlock, "break target not set");
    ExitBlocks(breakTargetBlock);
    builder.CreateBr(breakTarget);
    if (!currentCaseMap) // not in switch
    {
        llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(context, "next", function);
        SetCurrentBasicBlock(nextBlock);
        basicBlockOpen = true;
    }
}

void LinuxEmitter::Visit(BoundContinueStatement& boundContinueStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundContinueStatement);
    Assert(continueTarget && continueTargetBlock, "continue target not set");
    ExitBlocks(continueTargetBlock);
    builder.CreateBr(continueTarget);
    llvm::BasicBlock* nextBlock = llvm::BasicBlock::Create(context, "next", function);
    SetCurrentBasicBlock(nextBlock);
    basicBlockOpen = true;
}

void LinuxEmitter::Visit(BoundGotoStatement& boundGotoStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundGotoStatement);
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
}

void LinuxEmitter::Visit(BoundTryStatement& boundTryStatement)
{
    ClearFlags();
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
    std::vector<llvm::Type*> lpElemTypes;
    lpElemTypes.push_back(builder.getInt8PtrTy());
    lpElemTypes.push_back(builder.getInt8PtrTy());
    llvm::StructType* lpType = llvm::StructType::create(lpElemTypes);
    llvm::LandingPadInst* lp = builder.CreateLandingPad(lpType, 1);
    lp->addClause(llvm::Constant::getNullValue(llvm::PointerType::get(lpType, 0)));
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
        llvm::Value* handleThisEx = builder.CreateCall(handleException, handleExceptionArgs);
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
        builder.CreateBr(nextTarget);
    }
    SetCurrentBasicBlock(resumeTarget);
    builder.CreateResume(lp);
}

void LinuxEmitter::Visit(BoundRethrowStatement& boundRethrowStatement)
{
    destructorCallGenerated = false;
    lastInstructionWasRet = false;
    basicBlockOpen = false;
    SetTarget(&boundRethrowStatement);
    boundRethrowStatement.ReleaseCall()->Accept(*this);
    llvm::FunctionType* rethrowFunctionType = llvm::FunctionType::get(builder.getVoidTy(), false);
    llvm::Function* cxaRethrowFunction = llvm::cast<llvm::Function>(compileUnitModule->getOrInsertFunction("__cxa_rethrow", rethrowFunctionType,  nullptr));
    ArgVector rethrowArgs;
    builder.CreateCall(cxaRethrowFunction, rethrowArgs);
}

void LinuxEmitter::CreateCleanup()
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

void LinuxEmitter::GenerateCodeForCleanups()
{
    for (const std::unique_ptr<Cleanup>& cleanup : cleanups)
    {
        SetCurrentBasicBlock(cleanup->cleanupBlock);
        std::vector<llvm::Type*> lpElemTypes;
        lpElemTypes.push_back(builder.getInt8PtrTy());
        lpElemTypes.push_back(builder.getInt8PtrTy());
        llvm::StructType* lpType = llvm::StructType::create(lpElemTypes);
        llvm::LandingPadInst* lp = builder.CreateLandingPad(lpType, 0);
        lp->setCleanup(true);
        for (const std::unique_ptr<BoundFunctionCall>& destructorCall : cleanup->destructors)
        {
            destructorCall->Accept(*this);
        }
        builder.CreateResume(lp);
    }
}

} } // namespace cmajor::emitter
