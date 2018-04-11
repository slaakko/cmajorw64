// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/build/Build.hpp>
#include <cmajor/emitter/Emitter.hpp>
#include <cmajor/parser/Project.hpp>
#include <cmajor/parser/Solution.hpp>
#include <cmajor/parser/CompileUnit.hpp>
//#include <cmajor/parser/FileRegistry.hpp>
#include <cmajor/parsing/Exception.hpp>
#include <cmajor/binder/BoundCompileUnit.hpp>
#include <cmajor/binder/BoundFunction.hpp>
#include <cmajor/binder/TypeBinder.hpp>
#include <cmajor/binder/OverloadResolution.hpp>
#include <cmajor/binder/StatementBinder.hpp>
#include <cmajor/binder/BoundStatement.hpp>
#include <cmajor/binder/ModuleBinder.hpp>
#include <cmajor/binder/ControlFlowAnalyzer.hpp>
#include <cmajor/binder/AttributeBinder.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/symbols/Warning.hpp>
#include <cmajor/symbols/Module.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/SymbolCreatorVisitor.hpp>
#include <cmajor/symbols/Meta.hpp>
#include <cmajor/ast/Attribute.hpp>
#include <cmajor/ast/Function.hpp>
#include <cmajor/ast/BasicType.hpp>
#include <cmajor/ast/Identifier.hpp>
#include <cmajor/ast/TypeExpr.hpp>
#include <cmajor/ast/Expression.hpp>
#include <cmajor/ast/Literal.hpp>
#include <cmajor/ast/SystemFileIndex.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/Path.hpp>
#include <cmajor/util/System.hpp>
#include <cmajor/util/TextUtils.hpp>
#include <cmajor/util/Log.hpp>
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <mutex>
#include <chrono>
#include <thread>
#include <list>
#include <condition_variable>

using namespace cmajor::emitter;
using namespace cmajor::parser;
using namespace cmajor::parsing;
using namespace cmajor::ast;
using namespace cmajor::symbols;
using namespace cmajor::binder;
using namespace cmajor::util;
using namespace cmajor::unicode;

namespace cmajor { namespace build {

CompileUnitGrammar* compileUnitGrammar = nullptr;

std::vector<std::unique_ptr<CompileUnitNode>> ParseSourcesInMainThread(Module* module, const std::vector<std::string>& sourceFilePaths, bool& stop)
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
        LogMessage(module->LogStreamId(), "Parsing " + std::to_string(sourceFilePaths.size()) + " source file" + s + " in main thread...");
    }
    std::vector<std::unique_ptr<CompileUnitNode>> compileUnits;
    for (const std::string& sourceFilePath : sourceFilePaths)
    {
        if (stop)
        {
            return std::vector<std::unique_ptr<CompileUnitNode>>();
        }
        if (boost::filesystem::file_size(sourceFilePath) == 0)
        {
            std::unique_ptr<CompileUnitNode> compileUnit(new CompileUnitNode(Span(), sourceFilePath));
            compileUnits.push_back(std::move(compileUnit));
        }
        else
        {
            MappedInputFile sourceFile(sourceFilePath);
            int32_t fileIndex = module->GetFileTable().RegisterFilePath(sourceFilePath);
            ParsingContext parsingContext;
            if (GetGlobalFlag(GlobalFlags::debugParsing))
            {
                compileUnitGrammar->SetLog(&std::cout);
            }
            std::u32string s(ToUtf32(std::string(sourceFile.Begin(), sourceFile.End())));
            std::unique_ptr<CompileUnitNode> compileUnit(compileUnitGrammar->Parse(&s[0], &s[0] + s.length(), fileIndex, sourceFilePath, &parsingContext));
            if (GetGlobalFlag(GlobalFlags::generateDebugInfo))
            {
                compileUnit->ComputeLineStarts(s);
            }
            compileUnits.push_back(std::move(compileUnit));
        }
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::string s;
        if (sourceFilePaths.size() != 1)
        {
            s = "s";
        }
        LogMessage(module->LogStreamId(), "Source file" + s + " parsed.");
    }
    return compileUnits;
}

struct ParserData
{
    ParserData(const std::vector<std::string>& sourceFilePaths_, std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits_, const std::vector<uint32_t>& fileIndeces_,
        std::vector<std::exception_ptr>& exceptions_, bool& stop_) : sourceFilePaths(sourceFilePaths_), compileUnits(compileUnits_), fileIndeces(fileIndeces_), stop(stop_), 
        exceptions(exceptions_)
    {
    }
    const std::vector<std::string>& sourceFilePaths;
    std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits;
    const std::vector<uint32_t>& fileIndeces;
    std::list<int> indexQueue;
    std::mutex indexQueueMutex;
    bool& stop;
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
            if (boost::filesystem::file_size(sourceFilePath) == 0)
            {
                std::unique_ptr<CompileUnitNode> compileUnit(new CompileUnitNode(Span(), sourceFilePath));
                parserData->compileUnits[index].reset(compileUnit.release());
            }
            else
            {
                MappedInputFile sourceFile(sourceFilePath);
                ParsingContext parsingContext;
                int fileIndex = parserData->fileIndeces[index];
                std::u32string s(ToUtf32(std::string(sourceFile.Begin(), sourceFile.End())));
                std::unique_ptr<CompileUnitNode> compileUnit(compileUnitGrammar->Parse(&s[0], &s[0] + s.length(), fileIndex, sourceFilePath, &parsingContext));
                if (GetGlobalFlag(GlobalFlags::generateDebugInfo))
                {
                    compileUnit->ComputeLineStarts(s);
                }
                parserData->compileUnits[index].reset(compileUnit.release());
            }
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

std::vector<std::unique_ptr<CompileUnitNode>> ParseSourcesConcurrently(Module* module, const std::vector<std::string>& sourceFilePaths, int numThreads, bool& stop)
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
        LogMessage(module->LogStreamId(), "Parsing " + std::to_string(sourceFilePaths.size()) + " source file" + s + " using " + std::to_string(numThreads) + " threads...");
    }
    std::vector<std::unique_ptr<CompileUnitNode>> compileUnits;
    int n = int(sourceFilePaths.size());
    compileUnits.resize(n);
    std::vector<uint32_t> fileIndeces;
    for (int i = 0; i < n; ++i)
    {
        const std::string& sourceFilePath = sourceFilePaths[i];
        int32_t fileIndex = module->GetFileTable().RegisterFilePath(sourceFilePath);
        fileIndeces.push_back(fileIndex);
    }
    std::vector<std::exception_ptr> exceptions;
    exceptions.resize(n);
    ParserData parserData(sourceFilePaths, compileUnits, fileIndeces, exceptions, stop);
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
        LogMessage(module->LogStreamId(), "Source file" + s + " parsed.");
    }
    return compileUnits;
}

/*
void ReadUserFileIndexCounter(const std::string& config)
{
    boost::filesystem::path userFileIndexCounterFile = Path::Combine(Path::Combine(CmajorRootDir(), "config"), "userFileIndexCounter." + config);
    if (boost::filesystem::exists(userFileIndexCounterFile))
    {
        std::ifstream f(userFileIndexCounterFile.generic_string());
        uint32_t nextUserFileIndex = 1;
        f >> nextUserFileIndex;
        FileRegistry::Instance().SetNextUserFileIndex(nextUserFileIndex);
    }
}

void WriteUserFiledIndexCounter(const std::string& config)
{
    boost::filesystem::path userFileIndexCounterFile = Path::Combine(Path::Combine(CmajorRootDir(), "config"), "userFileIndexCounter." + config);
    std::ofstream f(userFileIndexCounterFile.generic_string());
    uint32_t nextUserFileIndex = FileRegistry::Instance().GetNextUserFileIndex();
    f << nextUserFileIndex << std::endl;
    if (!f)
    {
        throw std::runtime_error("could not write to " + userFileIndexCounterFile.generic_string());
    }
}
*/

std::vector<std::unique_ptr<CompileUnitNode>> ParseSources(Module* module, const std::vector<std::string>& sourceFilePaths, bool& stop)
{
    try
    {
        //FileRegistry::Instance().Clear();
        //ReadUserFileIndexCounter(GetConfig());
        int numCores = std::thread::hardware_concurrency();
        if (numCores == 0 || sourceFilePaths.size() < numCores || GetGlobalFlag(GlobalFlags::debugParsing))
        {
            return ParseSourcesInMainThread(module, sourceFilePaths, stop);
        }
        else
        {
            return ParseSourcesConcurrently(module, sourceFilePaths, numCores, stop);
        }
    }
    catch (ParsingException& ex)
    {
        ex.SetProject(ToUtf8(module->Name()));
        throw ex;
    }
    catch (const AttributeNotUniqueException& ex)
    {
        throw Exception(module, ex.what(), ex.GetSpan(), ex.PrevSpan());
    }
}

void CreateSymbols(SymbolTable& symbolTable, const std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits, bool& stop)
{
    SymbolCreatorVisitor symbolCreator(symbolTable);
    for (const std::unique_ptr<CompileUnitNode>& compileUnit : compileUnits)
    {
        if (stop)
        {
            return;
        }
        symbolTable.SetCurrentCompileUnit(compileUnit.get());
        compileUnit->Accept(symbolCreator);
    }
}

std::vector<std::unique_ptr<BoundCompileUnit>> BindTypes(Module& module, const std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits, AttributeBinder* attributeBinder, 
    bool& stop)
{
    std::vector<std::unique_ptr<BoundCompileUnit>> boundCompileUnits;
    for (const std::unique_ptr<CompileUnitNode>& compileUnit : compileUnits)
    {
        if (stop)
        {
            return std::vector<std::unique_ptr<BoundCompileUnit>>();
        }
        std::unique_ptr<BoundCompileUnit> boundCompileUnit(new BoundCompileUnit(module, compileUnit.get(), attributeBinder));
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

#ifdef _WIN32

void GenerateLibrary(Module* module, const std::vector<std::string>& objectFilePaths, const std::string& libraryFilePath)
{
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        LogMessage(module->LogStreamId(), "Creating library...");
    }
    module->SetCurrentToolName(U"llvm-lib");
    std::vector<std::string> args;
    args.push_back("/out:" + QuotedPath(libraryFilePath));
    int n = objectFilePaths.size();
    for (int i = 0; i < n; ++i)
    {
        args.push_back(QuotedPath(objectFilePaths[i]));
    }
    std::string libErrorFilePath = Path::Combine(Path::GetDirectoryName(libraryFilePath), "llvm-lib.error");
    std::string libCommandLine = "cmfileredirector -2 " + libErrorFilePath + " llvm-lib";
    for (const std::string& arg : args)
    {
        libCommandLine.append(1, ' ').append(arg);
    }
    try
    {
        System(libCommandLine);
        boost::filesystem::remove(boost::filesystem::path(libErrorFilePath));
    }
    catch (const std::exception& ex)
    {
        std::string errors = ReadFile(libErrorFilePath);
        throw std::runtime_error("generating library '" + libraryFilePath + "' failed: " + ex.what() + ":\nerrors:\n" + errors);
    }
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        LogMessage(module->LogStreamId(), "==> " + libraryFilePath);
    }
}

#else

void GenerateLibrary(Module* module, const std::vector<std::string>& objectFilePaths, const std::string& libraryFilePath)
{
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        LogMessage(module->LogStreamId(), "Creating library...");
    }
    boost::filesystem::remove(libraryFilePath);
    SetCurrentToolName(U"ar");
    std::vector<std::string> args;
    args.push_back("-o " + QuotedPath(libraryFilePath));
    int n = objectFilePaths.size();
    for (int i = 0; i < n; ++i)
    {
        args.push_back(QuotedPath(objectFilePaths[i]));
    }
    std::string libErrorFilePath = Path::Combine(Path::GetDirectoryName(libraryFilePath), "llvm-lib.error");
    std::string libCommandLine = "cmfileredirector -2 " + libErrorFilePath + " ar q";
    for (const std::string& arg : args)
    {
        libCommandLine.append(1, ' ').append(arg);
    }
    std::string libErrorFilePath = Path::Combine(Path::GetDirectoryName(libraryFilePath), "ar.error");
    try
    {
        System(libCommandLine);
        boost::filesystem::remove(boost::filesystem::path(libErrorFilePath));
    }
    catch (const std::exception& ex)
    {
        std::string errors = ReadFile(libErrorFilePath);
        throw std::runtime_error("generating library '" + libraryFilePath + "' failed: " + ex.what() + ":\nerrors:\n" + errors);
    }
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        LogMessage(module->LogStreamId(), "==> " + libraryFilePath);
    }
}

#endif

#ifdef _WIN32

void CreateDefFile(const std::string& defFilePath, Module& module)
{
    std::ofstream defFile(defFilePath);
    CodeFormatter formatter(defFile);
    formatter.WriteLine("EXPORTS");
    formatter.IncIndent();
    for (const std::string& fun : module.AllExportedFunctions())
    {
        formatter.WriteLine(fun);
    }
    for (const std::string& fun : module.ExportedFunctions())
    {
        formatter.WriteLine(fun);
    }
    for (const std::string& data : module.AllExportedData())
    {
        formatter.WriteLine(data + " DATA");
    }
    for (const std::string& data : module.ExportedData())
    {
        formatter.WriteLine(data + " DATA");
    }
}

void Link(const std::string& executableFilePath, const std::string& libraryFilePath, const std::vector<std::string>& libraryFilePaths, Module& module)
{
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        LogMessage(module.LogStreamId(), "Linking...");
    }
    if (GetGlobalFlag(GlobalFlags::linkUsingMsLink))
    {
        module.SetCurrentToolName(U"link");
    }
    else
    {
        module.SetCurrentToolName(U"lld-link");
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
    std::string defFilePath = GetFullPath(boost::filesystem::path(libraryFilePath).replace_extension(".def").generic_string());
    CreateDefFile(defFilePath, module);
    args.push_back("/def:" + QuotedPath(defFilePath));
    std::string cmrtLibName = "cmrt210.lib";
    if (GetGlobalFlag(GlobalFlags::linkWithDebugRuntime))
    {
        cmrtLibName = "cmrt210d.lib";
    }
    args.push_back(QuotedPath(Path::Combine(Path::Combine(CmajorRootDir(), "lib"), cmrtLibName)));
    int n = libraryFilePaths.size();
    for (int i = 0; i < n; ++i)
    {
        args.push_back(QuotedPath(libraryFilePaths[i]));
    }
    std::string linkCommandLine;
    std::string linkErrorFilePath;
    if (GetGlobalFlag(GlobalFlags::linkUsingMsLink))
    {
        linkErrorFilePath = Path::Combine(Path::GetDirectoryName(executableFilePath), "link.error");
        linkCommandLine = "cmfileredirector -2 " + linkErrorFilePath + " link";
    }
    else
    {
        linkErrorFilePath = Path::Combine(Path::GetDirectoryName(executableFilePath), "lld-link.error");
        linkCommandLine = "cmfileredirector -2 " + linkErrorFilePath + " lld-link";
    }
    for (const std::string& arg : args)
    {
        linkCommandLine.append(1, ' ').append(arg);
    }
    try
    {
        System(linkCommandLine);
        boost::filesystem::remove(boost::filesystem::path(linkErrorFilePath));
    }
    catch (const std::exception& ex)
    {
        std::string errors = ReadFile(linkErrorFilePath);
        throw std::runtime_error("linking executable '" + executableFilePath + "' failed: " + ex.what() + ":\nerrors:\n" + errors);
    }
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        LogMessage(module.LogStreamId(), "==> " + executableFilePath);
    }
}

#else

void CreateDynamicListFile(const std::string& dynamicListFilePath, Module& module)
{
    std::ofstream defFile(dynamicListFilePath);
    CodeFormatter formatter(defFile);
    formatter.WriteLine("{");
    formatter.IncIndent();
    for (const std::string& fun : module.AllExportedFunctions())
    {
        formatter.WriteLine(fun + ";");
    }
    for (const std::string& fun : module.ExportedFunctions())
    {
        formatter.WriteLine(fun + ";");
    }
    for (const std::string& data : module.AllExportedData())
    {
        formatter.WriteLine(data + ";");
    }
    for (const std::string& data : module.ExportedData())
    {
        formatter.WriteLine(data + ";");
    }
    formatter.DecIndent();
    formatter.WriteLine("};");
}

void Link(const std::string& executableFilePath, const std::string& libraryFilePath, const std::vector<std::string>& libraryFilePaths, Module& module)
{
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        LogMessage(module.LogStreamId(), "Linking...");
    }
    SetCurrentToolName(U"clang++");
    boost::filesystem::path bdp = executableFilePath;
    bdp.remove_filename();
    boost::filesystem::create_directories(bdp);
    std::vector<std::string> args;
    args.push_back("-L" + Path::Combine(CmajorRootDir(), "lib"));
    char* cmajorLibDir = getenv("CMAJOR_LIBDIR");
    if (cmajorLibDir && *cmajorLibDir)
    {
        args.push_back("-L" + std::string(cmajorLibDir));
    }
    std::string dynamicListFilePath = GetFullPath(boost::filesystem::path(libraryFilePath).replace_extension(".export").generic_string());
    CreateDynamicListFile(dynamicListFilePath, module);
    args.push_back("-Wl,--dynamic-list=" + dynamicListFilePath);
    args.push_back("-Xlinker --start-group");
    int n = libraryFilePaths.size();
    args.push_back(QuotedPath(libraryFilePaths.back()));
    for (int i = 0; i < n - 1; ++i)
    {
        args.push_back(QuotedPath(libraryFilePaths[i]));
    }
    if (GetGlobalFlag(GlobalFlags::linkWithDebugRuntime))
    {
        std::string cmrtLibName = "cmrtd";
        args.push_back("-l" + cmrtLibName);
        args.push_back("-lgmpintf");
    }
    else
    {
        std::string cmrtLibName = "cmrt";
        args.push_back("-l" + cmrtLibName);
        args.push_back("-lgmpintf");
    }
    args.push_back("-lboost_filesystem -lboost_iostreams -lboost_system -lgmp -lbz2 -lz");
    args.push_back("-Xlinker --end-group");
    args.push_back("-o " + QuotedPath(executableFilePath));
    std::string linkCommandLine;
    std::string linkErrorFilePath;
    linkErrorFilePath = Path::Combine(Path::GetDirectoryName(executableFilePath), "clang++.error");
    linkCommandLine = "cmfileredirector -2 " + linkErrorFilePath + " clang++";
    for (const std::string& arg : args)
    {
        linkCommandLine.append(1, ' ').append(arg);
    }
    try
    {
        System(linkCommandLine);
        boost::filesystem::remove(boost::filesystem::path(linkErrorFilePath));
    }
    catch (const std::exception& ex)
    {
        std::string errors = ReadFile(linkErrorFilePath);
        throw std::runtime_error("linking executable '" + executableFilePath + "' failed: " + ex.what() + ":\nerrors:\n" + errors);
    }
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        LogMessage(module.LogStreamId(), "==> " + executableFilePath);
    }
}

#endif

void CleanProject(Project* project)
{
    std::string config = GetConfig();
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        LogMessage(project->LogStreamId(), "Cleaning project '" + ToUtf8(project->Name()) + "' (" + project->FilePath() + ") using " + config + " configuration...");
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
        LogMessage(project->LogStreamId(), "Project '" + ToUtf8(project->Name()) + "' cleaned successfully.");
    }
}

/*
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
*/

void CheckMainFunctionSymbol(Module& module)
{
    FunctionSymbol* userMain = module.GetSymbolTable().MainFunctionSymbol();
    if (!userMain)
    {
        throw Exception(&module, "program has no main function", Span());
    }
    if (!userMain->Parameters().empty())
    {
        if (userMain->Parameters().size() != 2)
        {
            throw Exception(&module, "main function must either take no parameters or take two parameters", userMain->GetSpan());
        }
        if (!TypesEqual(userMain->Parameters()[0]->GetType(), module.GetSymbolTable().GetTypeByName(U"int")))
        {
            throw Exception(&module, "first parameter of main function must be of int type", userMain->GetSpan());
        }
        if (!TypesEqual(userMain->Parameters()[1]->GetType(), module.GetSymbolTable().GetTypeByName(U"char")->AddConst(userMain->GetSpan())->AddPointer(userMain->GetSpan())->AddPointer(userMain->GetSpan())))
        {
            throw Exception(&module, "second parameter of main function must be of 'const char**' type", userMain->GetSpan());
        }
    }
    if (userMain->ReturnType() && !userMain->ReturnType()->IsVoidType())
    {
        if (!TypesEqual(userMain->ReturnType(), module.GetSymbolTable().GetTypeByName(U"int")))
        {
            throw Exception(&module, "main function must either be void function or return an int", userMain->GetSpan());
        }
    }
}

void CreateJsonRegistrationUnit(std::vector<std::string>& objectFilePaths, Module& module, EmittingContext& emittingContext, AttributeBinder* attributeBinder)
{
    CompileUnitNode jsonRegistrationCompileUnit(Span(), boost::filesystem::path(module.OriginalFilePath()).parent_path().append("__json__.cm").generic_string());
    jsonRegistrationCompileUnit.SetSynthesizedUnit();
    jsonRegistrationCompileUnit.GlobalNs()->AddMember(new NamespaceImportNode(Span(), new IdentifierNode(Span(), U"System")));
    jsonRegistrationCompileUnit.GlobalNs()->AddMember(new NamespaceImportNode(Span(), new IdentifierNode(Span(), U"System.Json")));
    FunctionNode* jsonRegistrationFunction(new FunctionNode(Span(), Specifiers::public_, new IntNode(Span()), U"RegisterJsonClasses", nullptr));
    jsonRegistrationFunction->SetReturnTypeExpr(new VoidNode(Span()));
    CompoundStatementNode* jsonRegistrationFunctionBody = new CompoundStatementNode(Span());
    const std::unordered_set<std::u32string>& jsonClasses = module.GetSymbolTable().JsonClasses();
    for (const std::u32string& jsonClass : jsonClasses)
    {
        InvokeNode* invokeRegisterJsonClass = new InvokeNode(Span(), new IdentifierNode(Span(), U"RegisterJsonClass"));
        invokeRegisterJsonClass->AddArgument(new TypeNameNode(Span(), new IdentifierNode(Span(), jsonClass)));
        invokeRegisterJsonClass->AddArgument(new DotNode(Span(), new IdentifierNode(Span(), jsonClass), new IdentifierNode(Span(), U"Create")));
        ExpressionStatementNode* registerStatement = new ExpressionStatementNode(Span(), invokeRegisterJsonClass);
        jsonRegistrationFunctionBody->AddStatement(registerStatement);
    }
    jsonRegistrationFunction->SetBody(jsonRegistrationFunctionBody);
    jsonRegistrationCompileUnit.GlobalNs()->AddMember(jsonRegistrationFunction);
    SymbolCreatorVisitor symbolCreator(module.GetSymbolTable());
    jsonRegistrationCompileUnit.Accept(symbolCreator);
    BoundCompileUnit boundJsonCompileUnit(module, &jsonRegistrationCompileUnit, attributeBinder);
    boundJsonCompileUnit.PushBindingTypes();
    TypeBinder typeBinder(boundJsonCompileUnit);
    jsonRegistrationCompileUnit.Accept(typeBinder);
    boundJsonCompileUnit.PopBindingTypes();
    StatementBinder statementBinder(boundJsonCompileUnit);
    jsonRegistrationCompileUnit.Accept(statementBinder);
    if (boundJsonCompileUnit.HasGotos())
    {
        AnalyzeControlFlow(boundJsonCompileUnit);
    }
    GenerateCode(emittingContext, boundJsonCompileUnit);
    objectFilePaths.push_back(boundJsonCompileUnit.ObjectFilePath());
}

void CreateMainUnit(std::vector<std::string>& objectFilePaths, Module& module, EmittingContext& emittingContext, AttributeBinder* attributeBinder)
{
    CompileUnitNode mainCompileUnit(Span(), boost::filesystem::path(module.OriginalFilePath()).parent_path().append("__main__.cm").generic_string());
    mainCompileUnit.SetSynthesizedUnit();
    mainCompileUnit.GlobalNs()->AddMember(new NamespaceImportNode(Span(), new IdentifierNode(Span(), U"System")));
    FunctionNode* mainFunction(new FunctionNode(Span(), Specifiers::public_, new IntNode(Span()), U"main", nullptr));
#ifndef _WIN32
    mainFunction->AddParameter(new ParameterNode(Span(), new IntNode(Span()), new IdentifierNode(Span(), U"argc")));
    mainFunction->AddParameter(new ParameterNode(Span(), new PointerNode(Span(), new PointerNode(Span(), new CharNode(Span()))), new IdentifierNode(Span(), U"argv")));
#endif
    mainFunction->SetProgramMain();
    CompoundStatementNode* mainFunctionBody = new CompoundStatementNode(Span());
    ConstructionStatementNode* constructExitCode = new ConstructionStatementNode(Span(), new IntNode(Span()), new IdentifierNode(Span(), U"exitCode"));
    mainFunctionBody->AddStatement(constructExitCode);
    ExpressionStatementNode* rtInitCall = nullptr;
    if (GetGlobalFlag(GlobalFlags::profile))
    {
        rtInitCall = new ExpressionStatementNode(Span(), new InvokeNode(Span(), new IdentifierNode(Span(), U"RtStartProfiling")));
    }
    else
    {
        rtInitCall = new ExpressionStatementNode(Span(), new InvokeNode(Span(), new IdentifierNode(Span(), U"RtInit")));
    }
    mainFunctionBody->AddStatement(rtInitCall);
#ifdef _WIN32
    ConstructionStatementNode* argc = new ConstructionStatementNode(Span(), new IntNode(Span()), new IdentifierNode(Span(), U"argc"));
    argc->AddArgument(new InvokeNode(Span(), new IdentifierNode(Span(), U"RtArgc")));
    mainFunctionBody->AddStatement(argc);
    ConstructionStatementNode* argv = new ConstructionStatementNode(Span(), new ConstNode(Span(), new PointerNode(Span(), new PointerNode(Span(), new CharNode(Span())))), new IdentifierNode(Span(), U"argv"));
    argv->AddArgument(new InvokeNode(Span(), new IdentifierNode(Span(), U"RtArgv")));
    mainFunctionBody->AddStatement(argv);
#endif
    CompoundStatementNode* tryBlock = new CompoundStatementNode(Span());
    if (!module.GetSymbolTable().JsonClasses().empty())
    {
        ExpressionStatementNode* registerJsonClassesCall = new ExpressionStatementNode(Span(), new InvokeNode(Span(), new IdentifierNode(Span(), U"RegisterJsonClasses")));
        tryBlock->AddStatement(registerJsonClassesCall);
    }
    FunctionSymbol* userMain = module.GetSymbolTable().MainFunctionSymbol();
    InvokeNode* invokeMain = new InvokeNode(Span(), new IdentifierNode(Span(), userMain->GroupName()));
    if (!userMain->Parameters().empty())
    {
        invokeMain->AddArgument(new IdentifierNode(Span(), U"argc"));
        invokeMain->AddArgument(new IdentifierNode(Span(), U"argv"));
    }
    StatementNode* callMainStatement = nullptr;
    if (!userMain->ReturnType() || userMain->ReturnType()->IsVoidType())
    {
        callMainStatement = new ExpressionStatementNode(Span(), invokeMain);
    }
    else
    {
        callMainStatement = new AssignmentStatementNode(Span(), new IdentifierNode(Span(), U"exitCode"), invokeMain);
    }
    tryBlock->AddStatement(callMainStatement);
    TryStatementNode* tryStatement = new TryStatementNode(Span(), tryBlock);
    CompoundStatementNode* catchBlock = new CompoundStatementNode(Span());
    InvokeNode* consoleError = new InvokeNode(Span(), new DotNode(Span(), new IdentifierNode(Span(), U"System.Console"), new IdentifierNode(Span(), U"Error")));
    DotNode* writeLine = new DotNode(Span(), consoleError, new IdentifierNode(Span(), U"WriteLine"));
    InvokeNode* printEx = new InvokeNode(Span(), writeLine);
    InvokeNode* exToString = new InvokeNode(Span(), new DotNode(Span(), new IdentifierNode(Span(), U"ex"), new IdentifierNode(Span(), U"ToString")));
    printEx->AddArgument(exToString);
    ExpressionStatementNode* printExStatement = new ExpressionStatementNode(Span(), printEx);
    catchBlock->AddStatement(printExStatement);
    AssignmentStatementNode* assignExitCodeStatement = new AssignmentStatementNode(Span(), new IdentifierNode(Span(), U"exitCode"), new IntLiteralNode(Span(), 1));
    catchBlock->AddStatement(assignExitCodeStatement);
    CatchNode* catchAll = new CatchNode(Span(), new ConstNode(Span(), new LValueRefNode(Span(), new IdentifierNode(Span(), U"System.Exception"))), new IdentifierNode(Span(), U"ex"), catchBlock);
    tryStatement->AddCatch(catchAll);
    mainFunctionBody->AddStatement(tryStatement);
    ExpressionStatementNode* rtDoneCall = nullptr;
    if (GetGlobalFlag(GlobalFlags::profile))
    {
        rtDoneCall = new ExpressionStatementNode(Span(), new InvokeNode(Span(), new IdentifierNode(Span(), U"RtEndProfiling")));
    }
    else
    {
        rtDoneCall = new ExpressionStatementNode(Span(), new InvokeNode(Span(), new IdentifierNode(Span(), U"RtDone")));
    }
    mainFunctionBody->AddStatement(rtDoneCall);
    InvokeNode* exitCall = new InvokeNode(Span(), new IdentifierNode(Span(), U"RtExit"));
    exitCall->AddArgument(new IdentifierNode(Span(), U"exitCode"));
    ExpressionStatementNode* rtExitCall = new ExpressionStatementNode(Span(), exitCall);
    mainFunctionBody->AddStatement(rtExitCall);
    ReturnStatementNode* returnStatement = new ReturnStatementNode(Span(), new IdentifierNode(Span(), U"exitCode"));
    mainFunctionBody->AddStatement(returnStatement);
    mainFunction->SetBody(mainFunctionBody);
    mainCompileUnit.GlobalNs()->AddMember(mainFunction);
    SymbolCreatorVisitor symbolCreator(module.GetSymbolTable());
    mainCompileUnit.Accept(symbolCreator);
    BoundCompileUnit boundMainCompileUnit(module, &mainCompileUnit, attributeBinder);
    boundMainCompileUnit.PushBindingTypes();
    TypeBinder typeBinder(boundMainCompileUnit);
    mainCompileUnit.Accept(typeBinder);
    boundMainCompileUnit.PopBindingTypes();
    StatementBinder statementBinder(boundMainCompileUnit);
    mainCompileUnit.Accept(statementBinder);
    if (boundMainCompileUnit.HasGotos())
    {
        AnalyzeControlFlow(boundMainCompileUnit);
    }
    GenerateCode(emittingContext, boundMainCompileUnit);
    objectFilePaths.push_back(boundMainCompileUnit.ObjectFilePath());
}

void SetDefines(Module* module, const std::string& definesFilePath)
{
    module->ClearDefines();
    if (GetConfig() == "debug")
    {
        module->DefineSymbol(U"DEBUG");
    }
    else if (GetConfig() == "release")
    {
        module->DefineSymbol(U"RELEASE");
    }
    else if (GetConfig() == "profile")
    {
        module->DefineSymbol(U"RELEASE");
        module->DefineSymbol(U"PROFILE");
    }
#ifdef _WIN32
    module->DefineSymbol(U"WINDOWS");
#else
    module->DefineSymbol(U"LINUX");
#endif
    std::ifstream definesFile(definesFilePath);
    if (definesFile)
    {
        std::string line;
        while (std::getline(definesFile, line))
        {
            module->DefineSymbol(ToUtf32(line));
        }
    }
}

void InstallSystemLibraries(Module* systemInstallModule)
{
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        LogMessage(systemInstallModule->LogStreamId(), "Installing system libraries...");
    }
    boost::filesystem::path systemLibDir = CmajorSystemLibDir(GetConfig());
    boost::filesystem::create_directories(systemLibDir);
    for (Module* systemModule : systemInstallModule->AllReferencedModules())
    {
        boost::filesystem::path from = systemModule->OriginalFilePath();
        boost::filesystem::path to = systemLibDir / from.filename();
        if (boost::filesystem::exists(to))
        {
            boost::filesystem::remove(to);
        }
        boost::filesystem::copy(from, to);
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            LogMessage(systemInstallModule->LogStreamId(), from.generic_string() + " -> " + to.generic_string());
        }
        if (!systemModule->LibraryFilePath().empty())
        {
            from = systemModule->LibraryFilePath();
            to = systemLibDir / from.filename();
            if (boost::filesystem::exists(to))
            {
                boost::filesystem::remove(to);
            }
            boost::filesystem::copy(from, to);
            if (GetGlobalFlag(GlobalFlags::verbose))
            {
                LogMessage(systemInstallModule->LogStreamId(), from.generic_string() + " -> " + to.generic_string());
            }
        }
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        LogMessage(systemInstallModule->LogStreamId(), "System libraries installed.");
    }
}

void BuildProject(Project* project, std::unique_ptr<Module>& rootModule, bool& stop)
{
    if (project->GetTarget() == Target::unitTest)
    {
        throw std::runtime_error("cannot build unit test project '" + ToUtf8(project->Name()) + "' using cmc, use cmunit.");
    }
    std::string config = GetConfig();
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        LogMessage(project->LogStreamId(), "===== Building project '" + ToUtf8(project->Name()) + "' (" + project->FilePath() + ") using " + config + " configuration.");
    }
    //ReadTypeIdCounter(config); todo
    //ReadFunctionIdCounter(config); todo
    //ReadSystemFileIndex(config);
    rootModule.reset(new Module(project->Name(), project->ModuleFilePath()));
    {
        rootModule->SetLogStreamId(project->LogStreamId());
        rootModule->SetCurrentProjectName(project->Name());
        rootModule->SetCurrentToolName(U"cmc");
        boost::filesystem::path libraryFilePath = project->LibraryFilePath();
        boost::filesystem::path libDir = libraryFilePath.remove_filename();
        std::string definesFilePath = GetFullPath((libDir / boost::filesystem::path("defines.txt")).generic_string());
        SetDefines(rootModule.get(), definesFilePath);
        /*/
            if (module.IsSystemModule())
            {
                FileRegistry::Instance().PushObtainSystemFileIndeces();
            }
        */
        std::vector<std::unique_ptr<CompileUnitNode>> compileUnits = ParseSources(rootModule.get(), project->SourceFilePaths(), stop);
        /*
            if (module.IsSystemModule())
            {
                FileRegistry::Instance().PopObtainSystemFileIndeces();
            }
        */
        AttributeBinder attributeBinder(rootModule.get());
        std::unique_ptr<ModuleBinder> moduleBinder;
        if (!compileUnits.empty())
        {
            moduleBinder.reset(new ModuleBinder(*rootModule, compileUnits[0].get(), &attributeBinder));
        }
        std::vector<ClassTypeSymbol*> classTypes;
        std::vector<ClassTemplateSpecializationSymbol*> classTemplateSpecializations;
        rootModule->PrepareForCompilation(project->References(), classTypes, classTemplateSpecializations);
        cmajor::symbols::MetaInit(rootModule->GetSymbolTable());
        CreateSymbols(rootModule->GetSymbolTable(), compileUnits, stop);
        if (moduleBinder)
        {
            for (ClassTemplateSpecializationSymbol* classTemplateSpecialization : classTemplateSpecializations)
            {
                if (stop)
                {
                    return;
                }
                moduleBinder->BindClassTemplateSpecialization(classTemplateSpecialization);
            }
            for (ClassTypeSymbol* classType : classTypes)
            {
                if (stop)
                {
                    return;
                }
                classType->SetSpecialMemberFunctions();
                classType->CreateLayouts();
            }
        }
        std::vector<std::unique_ptr<BoundCompileUnit>> boundCompileUnits = BindTypes(*rootModule, compileUnits, &attributeBinder, stop);
        if (stop)
        {
            return;
        }
        EmittingContext emittingContext;
        std::vector<std::string> objectFilePaths;
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            LogMessage(project->LogStreamId(), "Compiling...");
        }
        int32_t compileUnitIndex = 0;
        for (std::unique_ptr<BoundCompileUnit>& boundCompileUnit : boundCompileUnits)
        {
            if (stop)
            {
                return;
            }
            boundCompileUnit->SetCompileUnitIndex(compileUnitIndex);
            if (GetGlobalFlag(GlobalFlags::verbose))
            {
                LogMessage(project->LogStreamId(), "> " + boost::filesystem::path(boundCompileUnit->GetCompileUnitNode()->FilePath()).filename().generic_string());
            }
            BindStatements(*boundCompileUnit);
            if (boundCompileUnit->HasGotos())
            {
                AnalyzeControlFlow(*boundCompileUnit);
            }
            GenerateCode(emittingContext, *boundCompileUnit);
            objectFilePaths.push_back(boundCompileUnit->ObjectFilePath());
            boundCompileUnit.reset();
            ++compileUnitIndex;
        }
        if (project->GetTarget() == Target::program)
        {
            CheckMainFunctionSymbol(*rootModule);
            if (!rootModule->GetSymbolTable().JsonClasses().empty())
            {
                CreateJsonRegistrationUnit(objectFilePaths, *rootModule, emittingContext, &attributeBinder);
            }
            CreateMainUnit(objectFilePaths, *rootModule, emittingContext, &attributeBinder);
        }
        if (!objectFilePaths.empty())
        {
            GenerateLibrary(rootModule.get(), objectFilePaths, project->LibraryFilePath());
        }
        if (project->GetTarget() == Target::program)
        {
            Link(project->ExecutableFilePath(), project->LibraryFilePath(), rootModule->LibraryFilePaths(), *rootModule);
            CreateClassFile(project->ExecutableFilePath(), rootModule->GetSymbolTable());
        }
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            LogMessage(project->LogStreamId(), "Writing module file...");
        }
        SymbolWriter writer(project->ModuleFilePath());
        rootModule->Write(writer);
        //WriteTypeIdCounter(config); todo
        //WriteFunctionIdCounter(config); todo
        /*
        if (module.IsSystemModule())
        {
            WriteSystemFileIndex(config);
        }
        */
        //WriteUserFiledIndexCounter(config);
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            LogMessage(project->LogStreamId(), "==> " + project->ModuleFilePath());
        }
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            LogMessage(project->LogStreamId(), "Project '" + ToUtf8(project->Name()) + "' built successfully.");
        }
        project->SetModuleFilePath(rootModule->OriginalFilePath());
        project->SetLibraryFilePath(rootModule->LibraryFilePath());
        if (rootModule->IsSystemModule())
        {
            project->SetSystemProject();
        }
        if (rootModule->Name() == U"System.Install")
        {
            InstallSystemLibraries(rootModule.get());
        }
    }
    rootModule.reset();
}

ProjectGrammar* projectGrammar = nullptr;

void BuildProject(const std::string& projectFilePath, std::unique_ptr<Module>& rootModule)
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
        bool stop = false;
        BuildProject(project.get(), rootModule, stop);
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

class ProjectQueue
{
public:
    ProjectQueue(bool& stop_);
    void Put(Project* project);
    Project* Get(int secs, bool ret);
private:
    std::list<Project*> queue;
    std::mutex mtx;
    std::condition_variable cond;
    bool& stop;
};

ProjectQueue::ProjectQueue(bool& stop_) : stop(stop_)
{
}

void ProjectQueue::Put(Project* project)
{
    std::lock_guard<std::mutex> lock(mtx);
    queue.push_back(project);
    cond.notify_all();
}

Project* ProjectQueue::Get(int secs, bool ret)
{
    while (!stop)
    {
        std::unique_lock<std::mutex> lock(mtx);
        if (cond.wait_for(lock, std::chrono::duration<std::uint64_t>(std::chrono::seconds(secs))) == std::cv_status::no_timeout)
        {
            if (!queue.empty())
            {
                Project* project = queue.front();
                queue.pop_front();
                return project;
            }
            else if (ret)
            {
                return nullptr;
            }
        }
        else if (ret)
        {
            return nullptr;
        }
    }
    return nullptr;
}

struct BuildData
{
    BuildData(bool& stop_, ProjectQueue& buildQueue_, ProjectQueue& readyQueue_, std::vector<std::unique_ptr<Module>>& rootModules_, bool& isSystemSolution_) :
        stop(stop_), buildQueue(buildQueue_), readyQueue(readyQueue_), rootModules(rootModules_), isSystemSolution(isSystemSolution_)
    {
    }
    bool& stop;
    ProjectQueue& buildQueue;
    ProjectQueue& readyQueue;
    std::vector<std::unique_ptr<Module>>& rootModules;
    std::mutex exceptionMutex;
    std::vector<std::exception_ptr> exceptions;
    bool& isSystemSolution;
};

void ProjectBuilder(BuildData* buildData)
{
    try
    {
        Project* toBuild = buildData->buildQueue.Get(1, false);
        while (toBuild && !buildData->stop)
        {
            BuildProject(toBuild, buildData->rootModules[toBuild->Index()], buildData->stop);
            if (toBuild->IsSystemProject())
            {
                buildData->isSystemSolution = true;
            }
            toBuild->SetBuilt();
            buildData->readyQueue.Put(toBuild);
            toBuild = buildData->buildQueue.Get(1, false);
        }
    }
    catch (...)
    {
        std::lock_guard<std::mutex> lock(buildData->exceptionMutex);
        buildData->exceptions.push_back(std::current_exception());
        buildData->stop = true;
    }
}

SolutionGrammar* solutionGrammar = nullptr;

void BuildSolution(const std::string& solutionFilePath, std::vector<std::unique_ptr<Module>>& rootModules)
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
            LogMessage(-1, "Cleaning solution '" + ToUtf8(solution->Name()) + "' (" + solution->FilePath() + ") using " + config + " configuration.");
        }
        else
        {
            LogMessage(-1, "Building solution '" + ToUtf8(solution->Name()) + "' (" + solution->FilePath() + ") using " + config + " configuration.");
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
    std::vector<Project*> projectsToBuild;
    bool isSystemSolution = false;
    int n = buildOrder.size();
    for (int i = 0; i < n; ++i)
    {
        Project* project = buildOrder[i];
        project->SetLogStreamId(i);
        project->SetIndex(i);
        if (project->GetTarget() == Target::unitTest)
        {
            if (GetGlobalFlag(GlobalFlags::verbose))
            {
                LogMessage(-1, "skipping unit test project '" + ToUtf8(project->Name()) + "'");
            }
            continue;
        }
        if (GetGlobalFlag(GlobalFlags::clean))
        {
            CleanProject(project);
        }
        else
        {
            projectsToBuild.push_back(project);
        }
    }
    if (!projectsToBuild.empty())
    {
        int numProjectsToBuild = projectsToBuild.size();
        int numThreads = std::thread::hardware_concurrency() * 2;
        rootModules.resize(numProjectsToBuild);
        if (GetGlobalFlag(GlobalFlags::verbose))
        {
            LogMessage(-1, "Building " + std::to_string(numProjectsToBuild) + " projects using " + std::to_string(numThreads) + " threads...");
        }
        bool stop = false;
        ProjectQueue buildQueue(stop);
        ProjectQueue readyQueue(stop);
        BuildData buildData(stop, buildQueue, readyQueue, rootModules, isSystemSolution);
        std::vector<std::thread> threads;
        for (int i = 0; i < numThreads; ++i)
        {
            threads.push_back(std::thread(ProjectBuilder, &buildData));
            if (buildData.stop) break;
        }
        while (numProjectsToBuild > 0 && !stop)
        {
            std::vector<Project*> building;
            for (Project* project : projectsToBuild)
            {
                if (project->Ready())
                {
                    building.push_back(project);
                    buildQueue.Put(project);
                }
            }
            for (Project* project : building)
            {
                projectsToBuild.erase(std::remove(projectsToBuild.begin(), projectsToBuild.end(), project), projectsToBuild.end());
            }
            Project* ready = readyQueue.Get(1, true);
            while (ready)
            {
                --numProjectsToBuild;
                ready = readyQueue.Get(1, true); 
            }
        }
        if (stop)
        {
            LogMessage(-1, "Build stopped.");
        }
        stop = true;
        int numStartedThreads = threads.size();
        for (int i = 0; i < numStartedThreads; ++i)
        {
            if (threads[i].joinable())
            {
                threads[i].join();
            }
        }
        if (!buildData.exceptions.empty())
        {
            std::rethrow_exception(buildData.exceptions.front());
        }
/*
        if (isSystemSolution)
        {
            CopySystemFiles(buildOrder);
        }
*/
    }
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        if (GetGlobalFlag(GlobalFlags::clean))
        {
            LogMessage(-1, "Solution '" + ToUtf8(solution->Name()) + "' cleaned successfully.");
        }
        else
        {
            LogMessage(-1, "Solution '" + ToUtf8(solution->Name()) + "' built successfully.");
        }
    }
}

void BuildMsBuildProject(const std::string& projectName, const std::string& projectDirectory, const std::string& target, 
    const std::vector<std::string>& sourceFiles, const std::vector<std::string>& referenceFiles, std::unique_ptr<Module>& rootModule)
{
    std::string projectFilePath = GetFullPath(Path::Combine(projectDirectory, projectName + ".cmproj"));
    std::unique_ptr<Project> project(new Project(ToUtf32(projectName), projectFilePath, GetConfig()));
    if (target == "program")
    {
        project->AddDeclaration(new TargetDeclaration(Target::program));
    }
    else if (target == "library")
    {
        project->AddDeclaration(new TargetDeclaration(Target::library));
    }
    else if (target == "unitTest")
    {
        project->AddDeclaration(new TargetDeclaration(Target::unitTest));
    }
    else
    {
        throw std::runtime_error("unsupported target '" + target + "'");
    }
    for (const std::string& sourceFile : sourceFiles)
    {
        project->AddDeclaration(new SourceFileDeclaration(sourceFile));
    }
    for (const std::string& referenceFile : referenceFiles)
    {
        project->AddDeclaration(new ReferenceDeclaration(referenceFile));
    }
    project->ResolveDeclarations();
    project->SetLogStreamId(-1);
    bool stop = false;
    BuildProject(project.get(), rootModule, stop);
}

} } // namespace cmajor::build
