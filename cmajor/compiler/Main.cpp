// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/InitDone.hpp>
#include <cmajor/parsing/InitDone.hpp>
#include <cmajor/util/InitDone.hpp>
#include <cmajor/build/Build.hpp>
#include <cmajor/parser/FileRegistry.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/InitDone.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/symbols/Warning.hpp>
#include <cmajor/parsing/Exception.hpp>
#include <cmajor/util/Util.hpp>
#include <cmajor/util/Path.hpp>
#include <cmajor/util/Json.hpp>
#include <cmajor/util/Unicode.hpp>
#include <boost/filesystem.hpp>
#include <boost/lexical_cast.hpp>
#include <iostream>
#include <string>
#include <stdexcept>
#include <chrono>

struct InitDone
{
    InitDone()
    {
        cmajor::ast::Init();
        cmajor::parser::FileRegistry::Init();
        cmajor::symbols::Init();
        cmajor::parsing::Init();
        cmajor::util::Init();
    }
    ~InitDone()
    {
        cmajor::util::Done();
        cmajor::parsing::Done();
        cmajor::symbols::Done();
        cmajor::ast::Done();
    }
};

const char* version = "2.0.0 beta-8";

void PrintHelp()
{
    std::cout << "Cmajor compiler version " << version << " for Windows x64" << std::endl; 
    std::cout << "Usage: cmc [options] { project.cmp | solution.cms }" << std::endl;
    std::cout << "Compiles given Cmajor solutions and projects." << std::endl;
    std::cout << "Options:\n" <<
        "--help (-h)\n" <<
        "   print this help message\n" <<
        "--config=CONFIG (c=CONFIG)\n" <<
        "   set configuration to CONFIG (debug | release | profile)\n" <<
        "   default is debug\n" <<
        "--optimization-level=LEVEL (-O=LEVEL)\n" <<
        "   set optimization level to LEVEL=0-3\n" <<
        "   defaults: debug=0, release=3\n" <<
        "--verbose (-v)\n" <<
        "   print verbose messages\n" <<
        "--quiet (-q)\n" <<
        "   print no messages\n" <<
        "--strict-nothrow (-s)\n" <<
        "   treat nothrow violation as error\n" <<
        "--time (-t)\n" <<
        "   print duration of compilation\n" <<
        "--emit-llvm (-l)\n" <<
        "   emit intermediate LLVM code to file.ll files\n" <<
        "--emit-opt-llvm (-o)\n" <<
        "   emit optimized intermediate LLVM code to file.opt.ll files\n" <<
        "--clean (-e)\n" <<
        "   clean given solutions and projects\n" <<
        "--debug-parse (-p)\n" <<
        "   debug parsing to stdout\n" <<
        "--link-with-debug-runtime (-d)\n" <<
        "   link with the debug version of the runtime library cmrt200(d).dll\n" <<
        "--link-using-ms-link (-m)\n" <<
        "   use Microsoft's link.exe as the linker\n" << 
        "--define SYMBOL (-D SYMBOL)\n" <<
        "   define a conditional compilation symbol SYMBOL.\n" <<
        std::endl;
}

using namespace cmajor::util;
using namespace cmajor::unicode;
using namespace cmajor::symbols;
using namespace cmajor::parsing;
using namespace cmajor::build;

int main(int argc, const char** argv)
{
    try
    {
        std::chrono::steady_clock::time_point start = std::chrono::steady_clock::now();
        InitDone initDone;
        std::vector<std::string> projectsAndSolutions;
        if (argc < 2)
        {
            PrintHelp();
        }
        else
        {
            bool prevWasDefine = false;
            for (int i = 1; i < argc; ++i)
            {
                std::string arg = argv[i];
                if (!arg.empty() && arg[0] == '-')
                {
                    if (arg == "--help" || arg == "-h")
                    {
                        PrintHelp();
                        return 0;
                    }
                    else if (arg == "--verbose" || arg == "-v")
                    {
                        SetGlobalFlag(GlobalFlags::verbose);
                    }
                    else if (arg == "--quiet" || arg == "-q")
                    {
                        SetGlobalFlag(GlobalFlags::quiet);
                    }
                    else if (arg == "--clean" || arg == "-e")
                    {
                        SetGlobalFlag(GlobalFlags::clean);
                    }
                    else if (arg == "--ide" || arg == "-i")
                    {
                        SetGlobalFlag(GlobalFlags::ide);
                    }
                    else if (arg == "--debug-parse" || arg == "-p")
                    {
                        SetGlobalFlag(GlobalFlags::debugParsing);
                    }
                    else if (arg == "--strict-nothrow" || arg == "-s")
                    {
                        SetGlobalFlag(GlobalFlags::strictNothrow);
                    }
                    else if (arg == "--emit-llvm" || arg == "-l")
                    {
                        SetGlobalFlag(GlobalFlags::emitLlvm);
                    }
                    else if (arg == "--emit-opt-llvm" || arg == "-o")
                    {
                        SetGlobalFlag(GlobalFlags::emitLlvm);
                        SetGlobalFlag(GlobalFlags::emitOptLlvm);
                    }
                    else if (arg == "--link-with-debug-runtime" || arg == "-d")
                    {
                        SetGlobalFlag(GlobalFlags::linkWithDebugRuntime);
                    }
                    else if (arg == "--link-using-ms-link" || arg == "-m")
                    {
                        SetGlobalFlag(GlobalFlags::linkUsingMsLink);
                    }
                    else if (arg == "--time" || arg == "-t")
                    {
                        SetGlobalFlag(GlobalFlags::time);
                    }
                    else if (arg == "--define" || arg == "-D")
                    {
                        prevWasDefine = true;
                    }
                    else if (arg.find('=') != std::string::npos)
                    {
                        std::vector<std::string> components = Split(arg, '=');
                        if (components.size() == 2)
                        {
                            if (components[0] == "--config" || components[0] == "-c")
                            {
                                if (components[1] == "release")
                                {
                                    SetGlobalFlag(GlobalFlags::release);
                                }
                                else if (components[1] != "debug")
                                {
                                    throw std::runtime_error("unknown configuration '" + components[1] + "'");
                                }
                            }
                            else if (components[0] == "--optimization-level" || components[0] == "-O")
                            {
                                int optimizationLevel = boost::lexical_cast<int>(components[1]);
                                if (optimizationLevel >= 0 && optimizationLevel <= 3)
                                {
                                    SetOptimizationLevel(optimizationLevel);
                                }
                                else
                                {
                                    throw std::runtime_error("unknown optimization level '" + components[1] + "'");
                                }
                            }
                            else
                            {
                                throw std::runtime_error("unknown option '" + arg + "'");
                            }
                        }
                        else
                        {
                            throw std::runtime_error("invalid argument '" + arg + "'");
                        }
                    }
                    else
                    {
                        throw std::runtime_error("unknown option '" + arg + "'");
                    }
                }
                else if (prevWasDefine)
                {
                    prevWasDefine = false;
                    DefineCommandLineConditionalSymbol(ToUtf32(arg));
                }
                else
                {
                    projectsAndSolutions.push_back(arg);
                }
            }
            if (projectsAndSolutions.empty())
            {
                PrintHelp();
                return 0;
            }
            if (GetGlobalFlag(GlobalFlags::verbose))
            {
                std::cout << "Cmajor compiler version " << version << " for Windows x64" << std::endl;
            }
            for (const std::string& projectOrSolution : projectsAndSolutions)
            {
                boost::filesystem::path fp(projectOrSolution);
                if (fp.extension() == ".cms")
                {
                    if (!boost::filesystem::exists(fp))
                    {
                        throw std::runtime_error("solution file '" + fp.generic_string() + " not found");
                    }
                    else
                    {
                        BuildSolution(GetFullPath(fp.generic_string()));
                    }
                }
                else if (fp.extension() == ".cmp")
                {
                    if (!boost::filesystem::exists(fp))
                    {
                        throw std::runtime_error("project file '" + fp.generic_string() + " not found");
                    }
                    else
                    {
                        BuildProject(GetFullPath(fp.generic_string()));
                    }
                }
                else
                {
                    throw std::runtime_error("Argument '" + fp.generic_string() + "' has invalid extension. Not Cmajor solution (.cms) or project (.cmp) file.");
                }
            }
            if (!CompileWarningCollection::Instance().Warnings().empty())
            {
                if (!GetGlobalFlag(GlobalFlags::quiet) && !GetGlobalFlag(GlobalFlags::ide))
                {
                    for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
                    {
                        std::string what = Expand(warning.Message(), warning.Defined(), warning.References(), "Warning");
                        std::cerr << what << std::endl;
                    }
                }
                if (GetGlobalFlag(GlobalFlags::ide))
                {
                    for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
                    {
                        std::string what = Expand(warning.Message(), warning.Defined(), warning.References(), "Warning");
                        std::cout << what << std::endl;
                    }
                    std::unique_ptr<JsonObject> compileResult(new JsonObject());
                    compileResult->AddField(U"success", std::unique_ptr<JsonValue>(new JsonBool(true)));
                    if (!CompileWarningCollection::Instance().Warnings().empty())
                    {
                        JsonArray* warningsArray = new JsonArray();
                        for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
                        {
                            warningsArray->AddItem(std::move(warning.ToJson()));
                        }
                        compileResult->AddField(U"warnings", std::unique_ptr<JsonValue>(warningsArray));
                    }
                    std::cerr << compileResult->ToString() << std::endl;
                }
            }
            else if (GetGlobalFlag(GlobalFlags::ide))
            {
                std::unique_ptr<JsonObject> compileResult(new JsonObject());
                compileResult->AddField(U"success", std::unique_ptr<JsonValue>(new JsonBool(true)));
                std::cerr << compileResult->ToString() << std::endl;
            }
            if (GetGlobalFlag(GlobalFlags::time))
            {
                std::chrono::steady_clock::time_point end = std::chrono::steady_clock::now();
                auto dur = end - start;
                long long totalSecs = std::chrono::duration_cast<std::chrono::seconds>(dur).count() + 1;
                int hours = static_cast<int>(totalSecs / 3600);
                int mins = static_cast<int>((totalSecs / 60) % 60);
                int secs = static_cast<int>(totalSecs % 60);
                std::cout <<
                    (hours > 0 ? std::to_string(hours) + " hour" + ((hours != 1) ? "s " : " ") : "") <<
                    (mins > 0 ? std::to_string(mins) + " minute" + ((mins != 1) ? "s " : " ") : "") <<
                    secs << " second" << ((secs != 1) ? "s" : "") << std::endl;
            }
        }
    }
    catch (const ParsingException& ex)
    {
        if (!GetGlobalFlag(GlobalFlags::quiet) && !GetGlobalFlag(GlobalFlags::ide))
        {
            std::cerr << ex.what() << std::endl;
            for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
            {
                std::string what = Expand(warning.Message(), warning.Defined(), warning.References(), "Warning");
                std::cerr << what << std::endl;
            }
        }
        if (GetGlobalFlag(GlobalFlags::ide))
        {
            std::cout << ex.what() << std::endl;
            for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
            {
                std::string what = Expand(warning.Message(), warning.Defined(), warning.References(), "Warning");
                std::cout << what << std::endl;
            }
            std::unique_ptr<JsonObject> compileResult(new JsonObject());
            compileResult->AddField(U"success", std::unique_ptr<JsonValue>(new JsonBool(false)));
            std::unique_ptr<JsonObject> json(new JsonObject());
            json->AddField(U"tool", std::unique_ptr<JsonValue>(new JsonString(U"cmc")));
            json->AddField(U"kind", std::unique_ptr<JsonValue>(new JsonString(U"error")));
            json->AddField(U"project", std::unique_ptr<JsonValue>(new JsonString(GetCurrentProjectName())));
            json->AddField(U"message", std::unique_ptr<JsonValue>(new JsonString(ToUtf32(ex.Message()))));
            std::unique_ptr<JsonArray> refs(new JsonArray());
            std::unique_ptr<JsonObject> ref = SpanToJson(ex.GetSpan());
            if (ref)
            {
                refs->AddItem(std::move(ref));
            }
            json->AddField(U"references", std::move(refs));
            compileResult->AddField(U"diagnostics", std::move(json));
            if (!CompileWarningCollection::Instance().Warnings().empty())
            {
                JsonArray* warningsArray = new JsonArray();
                for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
                {
                    warningsArray->AddItem(std::move(warning.ToJson()));
                }
                compileResult->AddField(U"warnings", std::unique_ptr<JsonValue>(warningsArray));
            }
            std::cerr << compileResult->ToString() << std::endl;
        }
        return 1;
    }
    catch (const Exception& ex)
    {
        if (!GetGlobalFlag(GlobalFlags::quiet) && !GetGlobalFlag(GlobalFlags::ide))
        {
            std::cerr << ex.What() << std::endl;
            for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
            {
                std::string what = Expand(warning.Message(), warning.Defined(), warning.References(), "Warning");
                std::cerr << what << std::endl;
            }
        }
        if (GetGlobalFlag(GlobalFlags::ide))
        {
            std::cout << ex.What() << std::endl;
            for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
            {
                std::string what = Expand(warning.Message(), warning.Defined(), warning.References(), "Warning");
                std::cout << what << std::endl;
            }
            std::unique_ptr<JsonObject> compileResult(new JsonObject());
            compileResult->AddField(U"success", std::unique_ptr<JsonValue>(new JsonBool(false)));
            compileResult->AddField(U"diagnostics", std::move(ex.ToJson()));
            if (!CompileWarningCollection::Instance().Warnings().empty())
            {
                JsonArray* warningsArray = new JsonArray();
                for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
                {
                    warningsArray->AddItem(std::move(warning.ToJson()));
                }
                compileResult->AddField(U"warnings", std::unique_ptr<JsonValue>(warningsArray));
            }
            std::cerr << compileResult->ToString() << std::endl;
        }
        return 1;
    }
    catch (const std::exception& ex)
    {
        if (!GetGlobalFlag(GlobalFlags::quiet) && !GetGlobalFlag(GlobalFlags::ide))
        {
            std::cerr << ex.what() << std::endl;
            for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
            {
                std::string what = Expand(warning.Message(), warning.Defined(), warning.References(), "Warning");
                std::cerr << what << std::endl;
            }
        }
        if (GetGlobalFlag(GlobalFlags::ide))
        {
            std::cout << ex.what() << std::endl;
            for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
            {
                std::string what = Expand(warning.Message(), warning.Defined(), warning.References(), "Warning");
                std::cout << what << std::endl;
            }
            std::unique_ptr<JsonObject> compileResult(new JsonObject());
            compileResult->AddField(U"success", std::unique_ptr<JsonValue>(new JsonBool(false)));
            std::unique_ptr<JsonObject> diagnostics(new JsonObject());
            diagnostics->AddField(U"tool", std::unique_ptr<JsonValue>(new JsonString(GetCurrentToolName())));
            diagnostics->AddField(U"kind", std::unique_ptr<JsonValue>(new JsonString(U"error")));
            diagnostics->AddField(U"project", std::unique_ptr<JsonValue>(new JsonString(GetCurrentProjectName())));
            diagnostics->AddField(U"message", std::unique_ptr<JsonValue>(new JsonString(ToUtf32(ex.what()))));
            compileResult->AddField(U"diagnostics", std::move(diagnostics));
            if (!CompileWarningCollection::Instance().Warnings().empty())
            {
                JsonArray* warningsArray = new JsonArray();
                for (const Warning& warning : CompileWarningCollection::Instance().Warnings())
                {
                    warningsArray->AddItem(std::move(warning.ToJson()));
                }
                compileResult->AddField(U"warnings", std::unique_ptr<JsonValue>(warningsArray));
            }
            std::cerr << compileResult->ToString() << std::endl;
        }
        return 1;
    }
    return 0;
}
