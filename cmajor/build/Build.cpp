// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/build/Build.hpp>
#include <cmajor/emitter/Emitter.hpp>
#include <cmajor/parser/Project.hpp>
#include <cmajor/parser/Solution.hpp>
#include <cmajor/parser/CompileUnit.hpp>
#include <cmajor/parser/FileRegistry.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/binder/TypeBinder.hpp>
#include <cmajor/binder/OverloadResolution.hpp>
#include <cmajor/binder/StatementBinder.hpp>
#include <cmajor/binder/BoundStatement.hpp>
#include <cmajor/binder/ModuleBinder.hpp>
#include <cmajor/binder/ControlFlowAnalyzer.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/symbols/Warning.hpp>
#include <cmajor/symbols/Module.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/SymbolCreatorVisitor.hpp>
#include <cmajor/ast/SystemFileIndex.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/Path.hpp>
#include <cmajor/util/System.hpp>
#include <cmajor/util/TextUtils.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <mutex>
#include <chrono>

using namespace cmajor::emitter;
using namespace cmajor::parser;
using namespace cmajor::ast;
using namespace cmajor::symbols;
using namespace cmajor::binder;
using namespace cmajor::util;
using namespace cmajor::unicode;

namespace cmajor { namespace build {

CompileUnitGrammar* compileUnitGrammar = nullptr;

std::vector<std::unique_ptr<CompileUnitNode>> ParseSourcesInMainThread(const std::vector<std::string>& sourceFilePaths)
{
    if (!compileUnitGrammar)
    {
        compileUnitGrammar = CompileUnitGrammar::Create();
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::string s;
        if (sourceFilePaths.size() != 1)
        {
            s = "s";
        }
        std::cout << "parsing " << sourceFilePaths.size() << " source file" << s << " in main thread..." << std::endl;
    }
    std::vector<std::unique_ptr<CompileUnitNode>> compileUnits;
    for (const std::string& sourceFilePath : sourceFilePaths)
    {
        MappedInputFile sourceFile(sourceFilePath);
        int fileIndex = FileRegistry::Instance().RegisterFile(sourceFilePath);
        ParsingContext parsingContext;
        if (GetGlobalFlag(GlobalFlags::debugParsing))
        {
            compileUnitGrammar->SetLog(&std::cout);
        }
        std::u32string s(ToUtf32(std::string(sourceFile.Begin(), sourceFile.End())));
        std::unique_ptr<CompileUnitNode> compileUnit(compileUnitGrammar->Parse(&s[0], &s[0] + s.length(), fileIndex, sourceFilePath, &parsingContext));
        compileUnits.push_back(std::move(compileUnit));
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::string s;
        if (sourceFilePaths.size() != 1)
        {
            s = "s";
        }
        std::cout << sourceFilePaths.size() << " source file" << s << " parsed" << std::endl;
    }
    return compileUnits;
}

struct ParserData
{
    ParserData(const std::vector<std::string>& sourceFilePaths_, std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits_, const std::vector<int>& fileIndeces_,
        std::vector<std::exception_ptr>& exceptions_) : sourceFilePaths(sourceFilePaths_), compileUnits(compileUnits_), fileIndeces(fileIndeces_), stop(false), exceptions(exceptions_)
    {
    }
    const std::vector<std::string>& sourceFilePaths;
    std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits;
    const std::vector<int>& fileIndeces;
    std::list<int> indexQueue;
    std::mutex indexQueueMutex;
    std::atomic<bool> stop;
    std::vector<std::exception_ptr>& exceptions;
};

void ParseSourceFile(ParserData* parserData)
{
    int index = -1;
    try
    {
        while (!parserData->stop)
        {
            {
                std::lock_guard<std::mutex> lock(parserData->indexQueueMutex);
                if (parserData->indexQueue.empty()) return;
                index = parserData->indexQueue.front();
                parserData->indexQueue.pop_front();
            }
            const std::string& sourceFilePath = parserData->sourceFilePaths[index];
            MappedInputFile sourceFile(sourceFilePath);
            ParsingContext parsingContext;
            int fileIndex = parserData->fileIndeces[index];
            std::u32string s(ToUtf32(std::string(sourceFile.Begin(), sourceFile.End())));
            std::unique_ptr<CompileUnitNode> compileUnit(compileUnitGrammar->Parse(&s[0], &s[0] + s.length(), fileIndex, sourceFilePath, &parsingContext));
            parserData->compileUnits[index].reset(compileUnit.release());
        }
    }
    catch (...)
    {
        if (index != -1)
        {
            parserData->exceptions[index] = std::current_exception();
            parserData->stop = true;
        }
    }
}

std::vector<std::unique_ptr<CompileUnitNode>> ParseSourcesConcurrently(const std::vector<std::string>& sourceFilePaths, int numThreads)
{
    if (!compileUnitGrammar)
    {
        compileUnitGrammar = CompileUnitGrammar::Create();
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::string s;
        if (sourceFilePaths.size() != 1)
        {
            s = "s";
        }
        std::cout << "Parsing " << sourceFilePaths.size() << " source file" << s << " using " << numThreads << " threads..." << std::endl;
    }
    std::vector<std::unique_ptr<CompileUnitNode>> compileUnits;
    int n = int(sourceFilePaths.size());
    compileUnits.resize(n);
    std::vector<int> fileIndeces;
    for (int i = 0; i < n; ++i)
    {
        const std::string& sourceFilePath = sourceFilePaths[i];
        int fileIndex = FileRegistry::Instance().RegisterFile(sourceFilePath);
        fileIndeces.push_back(fileIndex);
    }
    std::vector<std::exception_ptr> exceptions;
    exceptions.resize(n);
    ParserData parserData(sourceFilePaths, compileUnits, fileIndeces, exceptions);
    for (int i = 0; i < n; ++i)
    {
        parserData.indexQueue.push_back(i);
    }
    std::vector<std::thread> threads;
    for (int i = 0; i < numThreads; ++i)
    {
        threads.push_back(std::thread(ParseSourceFile, &parserData));
        if (parserData.stop) break;
    }
    int numStartedThreads = int(threads.size());
    for (int i = 0; i < numStartedThreads; ++i)
    {
        if (threads[i].joinable())
        {
            threads[i].join();
        }
    }
    for (int i = 0; i < n; ++i)
    {
        if (exceptions[i])
        {
            std::rethrow_exception(exceptions[i]);
        }
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::string s;
        if (sourceFilePaths.size() != 1)
        {
            s = "s";
        }
        std::cout << sourceFilePaths.size() << " source file" << s << " parsed" << std::endl;
    }
    return compileUnits;
}

std::vector<std::unique_ptr<CompileUnitNode>> ParseSources(const std::vector<std::string>& sourceFilePaths)
{
    int numCores = std::thread::hardware_concurrency();
    if (numCores == 0 || sourceFilePaths.size() < numCores || GetGlobalFlag(GlobalFlags::debugParsing))
    {
        return ParseSourcesInMainThread(sourceFilePaths);
    }
    else
    {
        return ParseSourcesConcurrently(sourceFilePaths, numCores);
    }
}

void CreateSymbols(SymbolTable& symbolTable, const std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits)
{
    SymbolCreatorVisitor symbolCreator(symbolTable);
    for (const std::unique_ptr<CompileUnitNode>& compileUnit : compileUnits)
    {
        symbolTable.SetCurrentCompileUnit(compileUnit.get());
        compileUnit->Accept(symbolCreator);
    }
}

std::vector<std::unique_ptr<BoundCompileUnit>> BindTypes(Module& module, const std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits)
{
    std::vector<std::unique_ptr<BoundCompileUnit>> boundCompileUnits;
    for (const std::unique_ptr<CompileUnitNode>& compileUnit : compileUnits)
    {
        std::unique_ptr<BoundCompileUnit> boundCompileUnit(new BoundCompileUnit(module, compileUnit.get()));
        boundCompileUnit->PushBindingTypes();
        TypeBinder typeBinder(*boundCompileUnit);
        compileUnit->Accept(typeBinder);
        boundCompileUnit->PopBindingTypes();
        boundCompileUnits.push_back(std::move(boundCompileUnit));
    }
    return boundCompileUnits;
}

void BindStatements(BoundCompileUnit& boundCompileUnit)
{
    StatementBinder statementBinder(boundCompileUnit);
    boundCompileUnit.GetCompileUnitNode()->Accept(statementBinder);
}

void GenerateLibrary(const std::vector<std::string>& objectFilePaths, const std::string& libraryFilePath)
{
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Creating library..." << std::endl;
    }
    SetCurrentTooName(U"llvm-lib");
    std::vector<std::string> args;
    args.push_back("/out:" + QuotedPath(libraryFilePath));
    int n = objectFilePaths.size();
    for (int i = 0; i < n; ++i)
    {
        args.push_back(QuotedPath(objectFilePaths[i]));
    }
    std::string libCommandLine = "llvm-lib";
    for (const std::string& arg : args)
    {
        libCommandLine.append(1, ' ').append(arg);
    }
    std::string libErrorFilePath = Path::Combine(Path::GetDirectoryName(libraryFilePath), "llvm-lib.error");
    int redirectHandle = 2; // stderr
    try
    {
        System(libCommandLine, redirectHandle, libErrorFilePath);
        boost::filesystem::remove(boost::filesystem::path(libErrorFilePath));
    }
    catch (const std::exception& ex)
    {
        std::string errors = ReadFile(libErrorFilePath);
        throw std::runtime_error("generating library '" + libraryFilePath + "' failed: " + ex.what() + ":\nerrors:\n" + errors);
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "==> " << libraryFilePath << std::endl;
    }
}

void Link(const std::string& executableFilePath, const std::vector<std::string>& libraryFilePaths, Module& module)
{
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Linking..." << std::endl;
    }
    if (GetGlobalFlag(GlobalFlags::linkWithMsLink))
    {
        SetCurrentTooName(U"link");
    }
    else
    {
        SetCurrentTooName(U"lld-link");
    }
    boost::filesystem::path bdp = executableFilePath;
    bdp.remove_filename();
    boost::filesystem::create_directories(bdp);
    std::vector<std::string> args;
    args.push_back("/machine:x64");
    args.push_back("/entry:main");
    args.push_back("/debug");
    args.push_back("/out:" + QuotedPath(executableFilePath));
    args.push_back("/stack:16777216");
    for (const std::string& fun : module.AllExportedFunctions())
    {
        args.push_back("/export:" + fun);
    }
    for (const std::string& fun : module.ExportedFunctions())
    {
        args.push_back("/export:" + fun);
    }
    for (const std::string& data : module.AllExportedData())
    {
        args.push_back("/export:" + data);
    }
    for (const std::string& data : module.ExportedData())
    {
        args.push_back("/export:" + data);
    }
    std::string cmrtLibName = "cmrt200.lib";
    if (GetGlobalFlag(GlobalFlags::linkWithDebugRuntime))
    {
        cmrtLibName = "cmrt200d.lib";
    }
    args.push_back(QuotedPath(Path::Combine(Path::Combine(CmajorRootDir(), "lib"), cmrtLibName)));
    int n = libraryFilePaths.size();
    for (int i = 0; i < n; ++i)
    {
        args.push_back(QuotedPath(libraryFilePaths[i]));
    }
    std::string linkCommandLine;
    std::string linkErrorFilePath;
    if (GetGlobalFlag(GlobalFlags::linkWithMsLink))
    {
        linkCommandLine = "link";
        linkErrorFilePath = Path::Combine(Path::GetDirectoryName(executableFilePath), "link.error");
    }
    else
    {
        linkCommandLine = "lld-link";
        linkErrorFilePath = Path::Combine(Path::GetDirectoryName(executableFilePath), "lld-link.error");
    }
    for (const std::string& arg : args)
    {
        linkCommandLine.append(1, ' ').append(arg);
    }
    int redirectHandle = 2; // stderr
    try
    {
        System(linkCommandLine, redirectHandle, linkErrorFilePath);
        boost::filesystem::remove(boost::filesystem::path(linkErrorFilePath));
    }
    catch (const std::exception& ex)
    {
        std::string errors = ReadFile(linkErrorFilePath);
        throw std::runtime_error("linking executable '" + executableFilePath + "' failed: " + ex.what() + ":\nerrors:\n" + errors);
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "==> " << executableFilePath << std::endl;
    }
}

void CleanProject(Project* project)
{
    std::string config = GetConfig();
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Cleaning project '" << ToUtf8(project->Name()) << "' (" << project->FilePath() << ") using " << config << " configuration." << std::endl;
    }
    boost::filesystem::path mfp = project->ModuleFilePath();
    mfp.remove_filename();
    boost::filesystem::remove_all(mfp);
    if (project->GetTarget() == Target::program)
    {
        boost::filesystem::path efp = project->ExecutableFilePath();
        efp.remove_filename();
        boost::filesystem::remove_all(efp);
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Project '" << ToUtf8(project->Name()) << "' cleaned successfully." << std::endl;
    }
}

void ReadTypeIdCounter(const std::string& config)
{
    boost::filesystem::path typeIdCounterFile = Path::Combine(Path::Combine(CmajorRootDir(), "config"), "typeIdCounter." + config);
    if (boost::filesystem::exists(typeIdCounterFile))
    {
        std::ifstream f(typeIdCounterFile.generic_string());
        int nextTypeId = 1;
        f >> nextTypeId;
        TypeIdCounter::Instance().SetNextTypeId(nextTypeId);
    }
}

void WriteTypeIdCounter(const std::string& config)
{
    boost::filesystem::path typeIdCounterFile = Path::Combine(Path::Combine(CmajorRootDir(), "config"), "typeIdCounter." + config);
    std::ofstream f(typeIdCounterFile.generic_string());
    int nextTypeId = TypeIdCounter::Instance().GetNextTypeId();
    f << nextTypeId << std::endl;
    if (!f)
    {
        throw std::runtime_error("could not write to " + typeIdCounterFile.generic_string());
    }
}

void ReadFunctionIdCounter(const std::string& config)
{
    boost::filesystem::path functionIdCounterFile = Path::Combine(Path::Combine(CmajorRootDir(), "config"), "functionIdCounter." + config);
    if (boost::filesystem::exists(functionIdCounterFile))
    {
        std::ifstream f(functionIdCounterFile.generic_string());
        int nextFunctionId = 1;
        f >> nextFunctionId;
        FunctionIdCounter::Instance().SetNextFunctionId(nextFunctionId);
    }
}

void WriteFunctionIdCounter(const std::string& config)
{
    boost::filesystem::path functionIdCounterFile = Path::Combine(Path::Combine(CmajorRootDir(), "config"), "functionIdCounter." + config);
    std::ofstream f(functionIdCounterFile.generic_string());
    int nextFunctionId = FunctionIdCounter::Instance().GetNextFunctionId();
    f << nextFunctionId << std::endl;
    if (!f)
    {
        throw std::runtime_error("could not write to " + functionIdCounterFile.generic_string());
    }
}

void ReadSystemFileIndex(const std::string& config)
{
    boost::filesystem::path systemFileIndexFile = Path::Combine(Path::Combine(CmajorRootDir(), "config"), "systemFileIndex." + config);
    if (boost::filesystem::exists(systemFileIndexFile))
    {
        SystemFileIndex::Instance().Read(GetFullPath(systemFileIndexFile.generic_string()));
    }
}

void WriteSystemFileIndex(const std::string& config)
{
    boost::filesystem::path systemFileIndexFile = Path::Combine(Path::Combine(CmajorRootDir(), "config"), "systemFileIndex." + config);
    SystemFileIndex::Instance().Write(GetFullPath(systemFileIndexFile.generic_string()));
}

void CreateMainUnit(std::vector<std::string>& objectFilePaths, Module& module, EmittingContext& emittingContext)
{
    CompileUnitNode mainCompileUnit(Span(), boost::filesystem::path(module.OriginalFilePath()).parent_path().append("__main__.cm").generic_string());
    BoundCompileUnit mainUnit(module, &mainCompileUnit);
    FunctionSymbol mainFunctionSymbol(Span(), U"main");
    mainFunctionSymbol.SetGroupName(U"main");
    mainFunctionSymbol.SetMangledName(U"main");
    mainFunctionSymbol.SetNothrow();
    std::unique_ptr<BoundFunction> mainFunction(new BoundFunction(&mainFunctionSymbol));
    std::unique_ptr<BoundCompoundStatement> body(new BoundCompoundStatement(Span()));
    std::vector<FunctionScopeLookup> lookups;
    lookups.push_back(FunctionScopeLookup(ScopeLookup::this_, module.GetSymbolTable().GlobalNs().GetContainerScope()));
    std::vector<std::unique_ptr<BoundExpression>> initArguments;
    std::unique_ptr<BoundStatement> initStatement(new BoundExpressionStatement(
        ResolveOverload(U"RtInit", module.GetSymbolTable().GlobalNs().GetContainerScope(), lookups, initArguments, mainUnit, mainFunction.get(), Span())));
    body->AddStatement(std::move(initStatement));
    std::unique_ptr<BoundStatement> mainStatement(new BoundExpressionStatement(std::unique_ptr<BoundExpression>(new BoundFunctionCall(Span(), module.GetSymbolTable().MainFunctionSymbol()))));
    body->AddStatement(std::move(mainStatement));
    std::vector<std::unique_ptr<BoundExpression>> doneArguments;
    std::unique_ptr<BoundStatement> doneStatement(new BoundExpressionStatement(
        ResolveOverload(U"RtDone", module.GetSymbolTable().GlobalNs().GetContainerScope(), lookups, doneArguments, mainUnit, mainFunction.get(), Span())));
    body->AddStatement(std::move(doneStatement));
    std::vector<std::unique_ptr<BoundExpression>> exitArguments;
    exitArguments.push_back(std::unique_ptr<BoundExpression>(new BoundLiteral(std::unique_ptr<Value>(new IntValue(Span(), 0)), module.GetSymbolTable().GetTypeByName(U"int"))));
    std::unique_ptr<BoundStatement> exitStatement(new BoundExpressionStatement(
        ResolveOverload(U"RtExit", module.GetSymbolTable().GlobalNs().GetContainerScope(), lookups, exitArguments, mainUnit, mainFunction.get(), Span())));
    body->AddStatement(std::move(exitStatement));
    mainFunction->SetBody(std::move(body));
    mainUnit.AddBoundNode(std::move(mainFunction));
    GenerateCode(emittingContext, mainUnit);
    objectFilePaths.push_back(mainUnit.ObjectFilePath());
}

void BuildProject(Project* project)
{
    std::string config = GetConfig();
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Building project '" << ToUtf8(project->Name()) << "' (" << project->FilePath() << ") using " << config << " configuration." << std::endl;
    }
    ReadTypeIdCounter(config);
    ReadFunctionIdCounter(config);
    ReadSystemFileIndex(config);
    CompileWarningCollection::Instance().SetCurrentProjectName(project->Name());
    SetCurrentProjectName(project->Name());
    SetCurrentTooName(U"cmc");
    Module module(project->Name(), project->ModuleFilePath());
    if (module.IsSystemModule())
    {
        FileRegistry::Instance().PushObtainSystemFileIndeces();
    }
    std::vector<std::unique_ptr<CompileUnitNode>> compileUnits = ParseSources(project->SourceFilePaths());
    if (module.IsSystemModule())
    {
        FileRegistry::Instance().PopObtainSystemFileIndeces();
    }
    std::unique_ptr<ModuleBinder> moduleBinder;
    if (!compileUnits.empty())
    {
        moduleBinder.reset(new ModuleBinder(module, compileUnits[0].get()));
    }
    std::vector<ClassTypeSymbol*> classTypes;
    std::vector<ClassTemplateSpecializationSymbol*> classTemplateSpecializations;
    module.PrepareForCompilation(project->References(), project->SourceFilePaths(), classTypes, classTemplateSpecializations);
    CreateSymbols(module.GetSymbolTable(), compileUnits);
    if (moduleBinder)
    {
        for (ClassTemplateSpecializationSymbol* classTemplateSpecialization : classTemplateSpecializations)
        {
            moduleBinder->BindClassTemplateSpecialization(classTemplateSpecialization);
        }
        for (ClassTypeSymbol* classType : classTypes)
        {
            classType->SetSpecialMemberFunctions();
            classType->CreateLayouts();
        }
    }
    std::vector<std::unique_ptr<BoundCompileUnit>> boundCompileUnits = BindTypes(module, compileUnits);
    EmittingContext emittingContext;
    std::vector<std::string> objectFilePaths;
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Compiling..." << std::endl;
    }
    for (std::unique_ptr<BoundCompileUnit>& boundCompileUnit : boundCompileUnits)
    {
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            std::cout << "> " << boost::filesystem::path(boundCompileUnit->GetCompileUnitNode()->FilePath()).filename().generic_string() << std::endl;
        }
        BindStatements(*boundCompileUnit);
        if (boundCompileUnit->HasGotos())
        {
            AnalyzeControlFlow(*boundCompileUnit);
        }
        GenerateCode(emittingContext, *boundCompileUnit);
        objectFilePaths.push_back(boundCompileUnit->ObjectFilePath());
        boundCompileUnit.reset();
    }
    if (project->GetTarget() == Target::program)
    {
        if (!module.GetSymbolTable().MainFunctionSymbol())
        {
            throw std::runtime_error("program has no main function");
        }
        CreateMainUnit(objectFilePaths, module, emittingContext);
    }
    GenerateLibrary(objectFilePaths, project->LibraryFilePath());
    if (project->GetTarget() == Target::program)
    {
        Link(project->ExecutableFilePath(), module.LibraryFilePaths(), module);
        CreateClassFile(project->ExecutableFilePath(), module.GetSymbolTable());
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Writing module file..." << std::endl;
    }
    SymbolWriter writer(project->ModuleFilePath());
    module.Write(writer);
    WriteTypeIdCounter(config);
    WriteFunctionIdCounter(config);
    if (module.IsSystemModule())
    {
        WriteSystemFileIndex(config);
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "==> " << project->ModuleFilePath() << std::endl;
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Project '" << ToUtf8(project->Name()) << "' built successfully." << std::endl;
    }
    project->SetModuleFilePath(module.OriginalFilePath());
    project->SetLibraryFilePath(module.LibraryFilePath());
    if (module.IsSystemModule())
    {
        project->SetSystemProject();
    }
}

ProjectGrammar* projectGrammar = nullptr;

void BuildProject(const std::string& projectFilePath)
{
    if (!projectGrammar)
    {
        projectGrammar = ProjectGrammar::Create();
    }
    std::string config = GetConfig();
    MappedInputFile projectFile(projectFilePath);
    std::u32string p(ToUtf32(std::string(projectFile.Begin(), projectFile.End())));
    std::unique_ptr<Project> project(projectGrammar->Parse(&p[0], &p[0] + p.length(), 0, projectFilePath, config));
    project->ResolveDeclarations();
    if (GetGlobalFlag(GlobalFlags::clean))
    {
        CleanProject(project.get());
    }
    else
    {
        BuildProject(project.get());
    }
}

void CopySystemFiles(std::vector<Project*>& projects)
{
    boost::filesystem::path systemLibDir = CmajorSystemLibDir(GetConfig());
    boost::filesystem::create_directories(systemLibDir);
    for (Project* project : projects)
    {
        boost::filesystem::path from = project->ModuleFilePath();
        boost::filesystem::path to = systemLibDir / from.filename();
        if (boost::filesystem::exists(to))
        {
            boost::filesystem::remove(to);
        }
        boost::filesystem::copy(from, to);
        if (!project->LibraryFilePath().empty())
        {
            from = project->LibraryFilePath();
            to = systemLibDir / from.filename();
            if (boost::filesystem::exists(to))
            {
                boost::filesystem::remove(to);
            }
            boost::filesystem::copy(from, to);
        }
    }
}


SolutionGrammar* solutionGrammar = nullptr;

void BuildSolution(const std::string& solutionFilePath)
{
    if (!solutionGrammar)
    {
        solutionGrammar = SolutionGrammar::Create();
    }
    if (!projectGrammar)
    {
        projectGrammar = ProjectGrammar::Create();
    }
    MappedInputFile solutionFile(solutionFilePath);
    std::u32string s(ToUtf32(std::string(solutionFile.Begin(), solutionFile.End())));
    std::unique_ptr<Solution> solution(solutionGrammar->Parse(&s[0], &s[0] + s.length(), 0, solutionFilePath));
    solution->ResolveDeclarations();
    std::string config = GetConfig();
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        if (GetGlobalFlag(GlobalFlags::clean))
        {
            std::cout << "Cleaning solution '" << ToUtf8(solution->Name()) << "' (" << solution->FilePath() << ") using " << config << " configuration." << std::endl;
        }
        else
        {
            std::cout << "Building solution '" << ToUtf8(solution->Name()) << "' (" << solution->FilePath() << ") using " << config << " configuration." << std::endl;
        }
    }
    for (const std::string& projectFilePath : solution->ProjectFilePaths())
    {
        MappedInputFile projectFile(projectFilePath);
        std::u32string p(ToUtf32(std::string(projectFile.Begin(), projectFile.End())));
        std::unique_ptr<Project> project(projectGrammar->Parse(&p[0], &p[0] + p.length(), 0, projectFilePath, config));
        project->ResolveDeclarations();
        solution->AddProject(std::move(project));
    }
    std::vector<Project*> buildOrder = solution->CreateBuildOrder();
    bool isSystemSolution = false;
    for (Project* project : buildOrder)
    {
        if (GetGlobalFlag(GlobalFlags::clean))
        {
            CleanProject(project);
        }
        else
        {
            BuildProject(project);
            if (project->IsSystemProject())
            {
                isSystemSolution = true;
            }
        }
    }
    if (isSystemSolution)
    {
        CopySystemFiles(buildOrder);
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        if (GetGlobalFlag(GlobalFlags::clean))
        {
            std::cout << "Solution '" << ToUtf8(solution->Name()) << "' cleaned successfully." << std::endl;
        }
        else
        {
            std::cout << "Solution '" << ToUtf8(solution->Name()) << "' built successfully." << std::endl;
        }
    }
}

} } // namespace cmajor::build
