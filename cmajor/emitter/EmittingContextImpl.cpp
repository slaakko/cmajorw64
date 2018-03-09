// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/emitter/EmittingContextImpl.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>

namespace cmajor { namespace emitter {

using namespace cmajor::symbols;

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

} } // namespace cmajor::emitter
