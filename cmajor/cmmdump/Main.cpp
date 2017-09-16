// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/InitDone.hpp>
#include <cmajor/parsing/InitDone.hpp>
#include <cmajor/util/InitDone.hpp>
#include <cmajor/symbols/InitDone.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/Module.hpp>
#include <cmajor/parser/FileRegistry.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <stdexcept>

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

const char* version = "2.0.0";

void PrintHelp()
{
    std::cout << "Cmajor Module Dump " << version << " for Windows x64" << std::endl;
}

using namespace cmajor::symbols;

int main(int argc, const char** argv)
{
    try
    {
        InitDone initDone;
        if (argc < 2)
        {
            PrintHelp();
            return 0;
        }
        std::vector<std::string> moduleFilePaths;
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
                else
                {
                    throw std::runtime_error("unknown option '" + arg + "'");
                }

            }
            else
            {
                moduleFilePaths.push_back(arg);
            }
        }
        if (moduleFilePaths.empty())
        {
            PrintHelp();
            return 0;
        }
        for (const std::string& moduleFilePath : moduleFilePaths)
        {
            boost::filesystem::path cmmPath(moduleFilePath);
            if (cmmPath.extension() != ".cmm")
            {
                throw std::runtime_error("Argument '" + cmmPath.generic_string() + "' has invalid extension. Not Cmajor module file (.cmm).");
            }
            if (!boost::filesystem::exists(cmmPath))
            {
                throw std::runtime_error("Cmajor module file '" + moduleFilePath + "' not found.");
            }
            Module module(moduleFilePath);
            module.Dump();
        }
    }
    catch (const Exception& ex)
    {
        std::cerr << ex.What() << std::endl;
        return 1;
    }
    catch (std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}