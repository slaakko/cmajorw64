// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/CommandLine.hpp>
#include <cmajor/parser/CommandLine.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/Error.hpp>
#include <cmajor/util/Path.hpp>
#include <memory>
#include <string>
#include <Windows.h>

namespace cmajor { namespace rt {

using namespace cmajor::unicode;

class CommandLineProcessor
{
public:
    static void Init();
    static void Done();
    static CommandLineProcessor& Instance() { Assert(instance, "command line processor not initialized"); return *instance; }
    int32_t Argc() const { return argc; }
    const char** Argv() const { return argv.get();  }
private:
    static std::unique_ptr<CommandLineProcessor> instance;
    cmajor::parser::CommandLineGrammar* grammar;
    std::u32string commandLine;
    std::vector<std::string> args;
    int32_t argc;
    std::unique_ptr<const char*[]> argv;
    CommandLineProcessor();
    std::vector<std::string> ExpandArguments();
    std::vector<std::string> ExpandArgument(const std::string& arg);
};

std::unique_ptr<CommandLineProcessor> CommandLineProcessor::instance;

void CommandLineProcessor::Init()
{
    instance.reset(new CommandLineProcessor());
}

void CommandLineProcessor::Done()
{
    instance.reset();
}

bool ContainsWildCard(const std::string& filePath)
{
    return filePath.find('*') != std::string::npos || filePath.find('?') != std::string::npos;
}

CommandLineProcessor::CommandLineProcessor() : grammar(cmajor::parser::CommandLineGrammar::Create()), commandLine(ToUtf32(GetCommandLine())), argc(0), argv(nullptr)
{
    args = grammar->Parse(&commandLine[0], &commandLine[0] + commandLine.length(), 0, "");
    argc = args.size();
    bool wildCards = false;
    for (int i = 0; i < argc; ++i)
    {
        if (i > 0 && ContainsWildCard(args[i]))
        {
            wildCards = true;
            break;
        }
    }
    if (wildCards)
    {
        args = ExpandArguments();
        argc = args.size();
    }
    argv.reset(new const char*[argc + 1]);
    for (int i = 0; i < argc; ++i)
    {
        argv[i] = args[i].c_str();
    }
    argv[argc] = "";
}

std::vector<std::string> CommandLineProcessor::ExpandArguments()
{
    std::vector<std::string> expandedArgs;
    for (int i = 0; i < argc; ++i)
    {
        if (i > 0 && ContainsWildCard(args[i]))
        {
            std::vector<std::string> expandedArg = ExpandArgument(args[i]);
            expandedArgs.insert(expandedArgs.end(), expandedArg.cbegin(), expandedArg.cend());
        }
        else
        {
            expandedArgs.push_back(args[i]);
        }
    }
    return expandedArgs;
}

std::vector<std::string> CommandLineProcessor::ExpandArgument(const std::string& arg)
{
    std::vector<std::string> expandedArg;
    WIN32_FIND_DATA findData;
    std::string filePath = GetFullPath(arg);
    HANDLE findHandle = FindFirstFile(filePath.c_str(), &findData);
    if (findHandle == INVALID_HANDLE_VALUE)
    {
        expandedArg.push_back(arg);
    }
    else
    {
        std::string directory = Path::GetDirectoryName(filePath);
        expandedArg.push_back(Path::Combine(directory, findData.cFileName));
        while (FindNextFile(findHandle, &findData))
        {
            expandedArg.push_back(Path::Combine(directory, findData.cFileName));
        }
        FindClose(findHandle);
    }
    return expandedArg;
}

extern "C" RT_API int32_t RtArgc()
{
    return cmajor::rt::CommandLineProcessor::Instance().Argc();
}

extern "C" RT_API const char** RtArgv()
{
    return cmajor::rt::CommandLineProcessor::Instance().Argv();
}

void InitCommandLine()
{
    CommandLineProcessor::Init();
}

void DoneCommandLine()
{
    CommandLineProcessor::Done();
}

} } // namespace cmajor::rt
