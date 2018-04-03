// =================================
// Copyright (c) 2018 Seppo Laakko
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
#include <iostream>
#include <fstream>
#include <stdexcept>
#include <mutex>
#include <chrono>
#include <thread>

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
        uint32_t fileIndex = FileRegistry::Instance().RegisterNewFile(sourceFilePath);
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
    ParserData(const std::vector<std::string>& sourceFilePaths_, std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits_, const std::vector<uint32_t>& fileIndeces_,
        std::vector<std::exception_ptr>& exceptions_) : sourceFilePaths(sourceFilePaths_), compileUnits(compileUnits_), fileIndeces(fileIndeces_), stop(false), exceptions(exceptions_)
    {
    }
    const std::vector<std::string>& sourceFilePaths;
    std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits;
    const std::vector<uint32_t>& fileIndeces;
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
            if (GetGlobalFlag(GlobalFlags::generateDebugInfo))
            {
                compileUnit->ComputeLineStarts(s);
            }
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
    std::vector<uint32_t> fileIndeces;
    for (int i = 0; i < n; ++i)
    {
        const std::string& sourceFilePath = sourceFilePaths[i];
        uint32_t fileIndex = FileRegistry::Instance().RegisterNewFile(sourceFilePath);
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

std::vector<std::unique_ptr<CompileUnitNode>> ParseSources(const std::vector<std::string>& sourceFilePaths)
{
    try
    {
        FileRegistry::Instance().Clear();
        ReadUserFileIndexCounter(GetConfig());
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
    catch (const AttributeNotUniqueException& ex)
    {
        throw Exception(ex.what(), ex.GetSpan(), ex.PrevSpan());
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

std::vector<std::unique_ptr<BoundCompileUnit>> BindTypes(Module& module, const std::vector<std::unique_ptr<CompileUnitNode>>& compileUnits, AttributeBinder* attributeBinder)
{
    std::vector<std::unique_ptr<BoundCompileUnit>> boundCompileUnits;
    for (const std::unique_ptr<CompileUnitNode>& compileUnit : compileUnits)
    {
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

void GenerateLibrary(const std::vector<std::string>& objectFilePaths, const std::string& libraryFilePath)
{
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        std::cout << "Creating library..." << std::endl;
    }
    SetCurrentToolName(U"llvm-lib");
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
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        std::cout << "==> " << libraryFilePath << std::endl;
    }
}

#else

void GenerateLibrary(const std::vector<std::string>& objectFilePaths, const std::string& libraryFilePath)
{
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        std::cout << "Creating library..." << std::endl;
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
    std::string libCommandLine = "ar q";
    for (const std::string& arg : args)
    {
        libCommandLine.append(1, ' ').append(arg);
    }
    std::string libErrorFilePath = Path::Combine(Path::GetDirectoryName(libraryFilePath), "ar.error");
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
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        std::cout << "==> " << libraryFilePath << std::endl;
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
        std::cout << "Linking..." << std::endl;
    }
    if (GetGlobalFlag(GlobalFlags::linkUsingMsLink))
    {
        SetCurrentToolName(U"link");
    }
    else
    {
        SetCurrentToolName(U"lld-link");
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
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        std::cout << "==> " << executableFilePath << std::endl;
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
        std::cout << "Linking..." << std::endl;
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
    linkCommandLine = "clang++";
    linkErrorFilePath = Path::Combine(Path::GetDirectoryName(executableFilePath), "clang++.error");
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
    if (GetGlobalFlag(GlobalFlags::verbose) && !GetGlobalFlag(GlobalFlags::unitTest))
    {
        std::cout << "==> " << executableFilePath << std::endl;
    }
}

#endif

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

void CheckMainFunctionSymbol(Module& module)
{
    FunctionSymbol* userMain = module.GetSymbolTable().MainFunctionSymbol();
    if (!userMain)
    {
        throw Exception("program has no main function", Span());
    }
    if (!userMain->Parameters().empty())
    {
        if (userMain->Parameters().size() != 2)
        {
            throw Exception("main function must either take no parameters or take two parameters", userMain->GetSpan());
        }
        if (!TypesEqual(userMain->Parameters()[0]->GetType(), module.GetSymbolTable().GetTypeByName(U"int")))
        {
            throw Exception("first parameter of main function must be of int type", userMain->GetSpan());
        }
        if (!TypesEqual(userMain->Parameters()[1]->GetType(), module.GetSymbolTable().GetTypeByName(U"char")->AddConst(userMain->GetSpan())->AddPointer(userMain->GetSpan())->AddPointer(userMain->GetSpan())))
        {
            throw Exception("second parameter of main function must be of 'const char**' type", userMain->GetSpan());
        }
    }
    if (userMain->ReturnType() && !userMain->ReturnType()->IsVoidType())
    {
        if (!TypesEqual(userMain->ReturnType(), module.GetSymbolTable().GetTypeByName(U"int")))
        {
            throw Exception("main function must either be void function or return an int", userMain->GetSpan());
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

void SetDefines(const std::string& definesFilePath)
{
    ClearDefines();
    if (GetConfig() == "debug")
    {
        DefineSymbol(U"DEBUG");
    }
    else if (GetConfig() == "release")
    {
        DefineSymbol(U"RELEASE");
    }
    else if (GetConfig() == "profile")
    {
        DefineSymbol(U"RELEASE");
        DefineSymbol(U"PROFILE");
    }
#ifdef _WIN32
    DefineSymbol(U"WINDOWS");
#else
    DefineSymbol(U"LINUX");
#endif
    std::ifstream definesFile(definesFilePath);
    if (definesFile)
    {
        std::string line;
        while (std::getline(definesFile, line))
        {
            DefineSymbol(ToUtf32(line));
        }
    }
}

void BuildProject(Project* project)
{
    if (project->GetTarget() == Target::unitTest)
    {
        throw std::runtime_error("cannot build unit test project '" + ToUtf8(project->Name()) + "' using cmc, use cmunit.");
    }
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
    SetCurrentToolName(U"cmc");
    boost::filesystem::path libraryFilePath = project->LibraryFilePath();
    boost::filesystem::path libDir = libraryFilePath.remove_filename();
    std::string definesFilePath = GetFullPath((libDir / boost::filesystem::path("defines.txt")).generic_string());
    SetDefines(definesFilePath);
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
    AttributeBinder attributeBinder;
    std::unique_ptr<ModuleBinder> moduleBinder;
    if (!compileUnits.empty())
    {
        moduleBinder.reset(new ModuleBinder(module, compileUnits[0].get(), &attributeBinder));
    }
    std::vector<ClassTypeSymbol*> classTypes;
    std::vector<ClassTemplateSpecializationSymbol*> classTemplateSpecializations;
    module.PrepareForCompilation(project->References(), FileRegistry::Instance().GetFileMap(), classTypes, classTemplateSpecializations);
    cmajor::symbols::MetaInit(module.GetSymbolTable());
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
    std::vector<std::unique_ptr<BoundCompileUnit>> boundCompileUnits = BindTypes(module, compileUnits, &attributeBinder);
    EmittingContext emittingContext;
    std::vector<std::string> objectFilePaths;
    if (GetGlobalFlag(GlobalFlags::verbose))
    {
        std::cout << "Compiling..." << std::endl;
    }
    int32_t compileUnitIndex = 0;
    for (std::unique_ptr<BoundCompileUnit>& boundCompileUnit : boundCompileUnits)
    {
        boundCompileUnit->SetCompileUnitIndex(compileUnitIndex);
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
        ++compileUnitIndex;
    }
    if (project->GetTarget() == Target::program)
    {
        CheckMainFunctionSymbol(module);
        if (!module.GetSymbolTable().JsonClasses().empty())
        {
            CreateJsonRegistrationUnit(objectFilePaths, module, emittingContext, &attributeBinder);
        }
        CreateMainUnit(objectFilePaths, module, emittingContext, &attributeBinder);
    }
    if (!objectFilePaths.empty())
    {
        GenerateLibrary(objectFilePaths, project->LibraryFilePath());
    }
    if (project->GetTarget() == Target::program)
    {
        Link(project->ExecutableFilePath(), project->LibraryFilePath(), module.LibraryFilePaths(), module);
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
    WriteUserFiledIndexCounter(config);
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
        if (project->GetTarget() == Target::unitTest)
        {
            if (GetGlobalFlag(GlobalFlags::verbose))
            {
                std::cout << "skipping unit test project '" << ToUtf8(project->Name()) << "'" << std::endl;
            }
            continue;
        }
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

void BuildMsBuildProject(const std::string& projectName, const std::string& projectDirectory, const std::string& target, 
    const std::vector<std::string>& sourceFiles, const std::vector<std::string>& referenceFiles)
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
    BuildProject(project.get());
}

} } // namespace cmajor::build
