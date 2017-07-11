// =================================
// Copyright (c) 2017 Seppo Laakko
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
#include <cmajor/util/Util.hpp>
#include <cmajor/util/Path.hpp>
#include <boost/filesystem.hpp>
#include <iostream>
#include <string>
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
    std::cout << "Cmajor compiler version " << version << " for Windows x64" << std::endl;
}

using namespace cmajor::util;
using namespace cmajor::symbols;
using namespace cmajor::build;

class Alpha
{

};

class Beta : public Alpha
{

};

int main(int argc, const char** argv)
{
    try
    {
        InitDone initDone;
        std::vector<std::string> projectsAndSolutions;
        if (argc < 2)
        {
            PrintHelp();
        }
        else
        {
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
                    else if (arg == "--debug-parse" || arg == "-p")
                    {
                        SetGlobalFlag(GlobalFlags::debugParsing);
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
        }
    }
    catch (const Exception& ex)
    {
        if (!GetGlobalFlag(GlobalFlags::quiet))
        {
            std::cerr << ex.What() << std::endl;
        }
        return 1;
    }
    catch (const std::exception& ex)
    {
        std::cerr << ex.what() << std::endl;
        return 1;
    }
    return 0;
}
