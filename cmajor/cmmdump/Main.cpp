// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/ast/InitDone.hpp>
#include <cmajor/parsing/InitDone.hpp>
#include <cmajor/util/InitDone.hpp>
#include <cmajor/binder/AttributeBinder.hpp>
#include <cmajor/binder/ModuleBinder.hpp>
#include <cmajor/symbols/InitDone.hpp>
#include <cmajor/symbols/Exception.hpp>
#include <cmajor/symbols/Module.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
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

const char* version = "2.1.0";

void PrintHelp()
{
    std::cout << "Cmajor Module Dump " << version << " for Windows x64" << std::endl;
}

using namespace cmajor::symbols;
using namespace cmajor::binder;

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
        SetGlobalFlag(GlobalFlags::info);
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
            std::vector<ClassTypeSymbol*> classTypes;
            std::vector<ClassTemplateSpecializationSymbol*> classTemplateSpecializations;
            Module module(moduleFilePath, classTypes, classTemplateSpecializations);
            if (module.Name() == U"System.Base")
            {
                cmajor::symbols::MetaInit(module.GetSymbolTable());
            }
            std::unique_ptr<ModuleBinder> moduleBinder;
            CompileUnitNode compileUnit(Span(), "foo");
            AttributeBinder attributeBinder;
            moduleBinder.reset(new ModuleBinder(module, &compileUnit, &attributeBinder));
            moduleBinder->SetBindingTypes();
            module.GetSymbolTable().AddClassTemplateSpecializationsToClassTemplateSpecializationMap(classTemplateSpecializations);
            for (ClassTemplateSpecializationSymbol* classTemplateSpecialization : classTemplateSpecializations)
            {
                moduleBinder->BindClassTemplateSpecialization(classTemplateSpecialization);
            }
            for (ClassTypeSymbol* classType : classTypes)
            {
                classType->SetSpecialMemberFunctions();
                classType->CreateLayouts();
            }
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