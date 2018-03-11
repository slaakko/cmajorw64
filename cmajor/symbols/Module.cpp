// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/Module.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/symbols/SymbolCollector.hpp>
#include <cmajor/symbols/Warning.hpp>
#include <cmajor/parser/FileRegistry.hpp>
#include <cmajor/ast/Project.hpp>
#include <cmajor/util/CodeFormatter.hpp>
#include <cmajor/util/Path.hpp>
#include <cmajor/util/Unicode.hpp>
#include <boost/filesystem.hpp>
#include <iostream>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;
using namespace cmajor::parser;

class SystemModuleSet
{
public:
    static void Init();
    static void Done();
    static SystemModuleSet& Instance() { Assert(instance, "system module set not initialized"); return *instance; }
    bool IsSystemModule(const std::u32string& moduleName) const;
private:
    static std::unique_ptr<SystemModuleSet> instance;
    std::unordered_set<std::u32string> systemModuleNames;
    SystemModuleSet();
};

std::unique_ptr<SystemModuleSet> SystemModuleSet::instance;

void SystemModuleSet::Init()
{
    instance.reset(new SystemModuleSet());
}

void SystemModuleSet::Done()
{
    instance.reset();
}

SystemModuleSet::SystemModuleSet()
{
    systemModuleNames.insert(U"System.Core");
    systemModuleNames.insert(U"System.Runtime");
    systemModuleNames.insert(U"System.Base");
    systemModuleNames.insert(U"System.Text.Parsing.CodeDom");
    systemModuleNames.insert(U"System.Text.Parsing");
    systemModuleNames.insert(U"System.Net.Sockets");
    systemModuleNames.insert(U"System.Net.Http");
    systemModuleNames.insert(U"System.Json");
    systemModuleNames.insert(U"System.Xml");
    systemModuleNames.insert(U"System.Dom");
    systemModuleNames.insert(U"System.Numerics.Multiprecision");
    systemModuleNames.insert(U"System.IO.Compression");
    systemModuleNames.insert(U"System.Text.RegularExpressions");
    systemModuleNames.insert(U"System.XPath");
    systemModuleNames.insert(U"System");
}

bool SystemModuleSet::IsSystemModule(const std::u32string& moduleName) const
{
    auto it = systemModuleNames.find(moduleName);
    if (it != systemModuleNames.cend())
    {
        return true;
    }
    return false;
}
bool IsSystemModule(const std::u32string& moduleName) 
{
    return SystemModuleSet::Instance().IsSystemModule(moduleName);
}

const char* cmajorModuleTag = "CMM";

ModuleTag::ModuleTag()
{
    bytes[0] = cmajorModuleTag[0];
    bytes[1] = cmajorModuleTag[1];
    bytes[2] = cmajorModuleTag[2];
    bytes[3] = currentModuleFormat;
}

void ModuleTag::Write(SymbolWriter& writer)
{
    writer.GetBinaryWriter().Write(bytes[0]);
    writer.GetBinaryWriter().Write(bytes[1]);
    writer.GetBinaryWriter().Write(bytes[2]);
    writer.GetBinaryWriter().Write(bytes[3]);
}

void ModuleTag::Read(SymbolReader& reader)
{
    bytes[0] = reader.GetBinaryReader().ReadByte();
    bytes[1] = reader.GetBinaryReader().ReadByte();
    bytes[2] = reader.GetBinaryReader().ReadByte();
    bytes[3] = reader.GetBinaryReader().ReadByte();
}

std::string ModuleFlagStr(ModuleFlags flags)
{
    std::string s;
    if ((flags & ModuleFlags::system) != ModuleFlags::none)
    {
        if (!s.empty())
        {
            s.append(1, ' ');
        }
        s.append("system");
    }
    return s;
}

ModuleDependency::ModuleDependency(Module* module_) : module(module_)
{
}

void ModuleDependency::AddReferencedModule(Module* referencedModule)
{
    referencedModules.push_back(referencedModule);
}

void ModuleDependency::Dump(CodeFormatter& formatter)
{
    formatter.IncIndent();
    int n = referencedModules.size();
    for (int i = 0; i < n; ++i)
    {
        Module* referencedModule = referencedModules[i];
        formatter.WriteLine(ToUtf8(referencedModule->Name()));
        referencedModule->GetModuleDependency().Dump(formatter);
    }
    formatter.DecIndent();
}

void Visit(std::vector<Module*>& finishReadOrder, Module* module, std::unordered_set<Module*>& visited, std::unordered_set<Module*>& tempVisit,
    std::unordered_map<Module*, ModuleDependency*>& dependencyMap, const Module* rootModule)
{
    if (tempVisit.find(module) == tempVisit.cend())
    {
        if (visited.find(module) == visited.cend())
        {
            tempVisit.insert(module);
            auto i = dependencyMap.find(module);
            if (i != dependencyMap.cend())
            {
                ModuleDependency* dependency = i->second;
                for (Module* dependentAssembly : dependency->ReferencedModules())
                {
                    Visit(finishReadOrder, dependentAssembly, visited, tempVisit, dependencyMap, rootModule);
                }
                tempVisit.erase(module);
                visited.insert(module);
                finishReadOrder.push_back(module);
            }
            else
            {
                throw std::runtime_error("module '" + ToUtf8(module->Name()) + "' not found in dependencies of module '" + ToUtf8(rootModule->Name()) + "'");
            }
        }
    }
    else
    {
        throw std::runtime_error("circular module dependency '" + ToUtf8(module->Name()) + "' detected in dependencies of module '" + ToUtf8(rootModule->Name()) + "'");
    }
}

std::vector<Module*> CreateFinishReadOrder(std::vector<Module*>& modules, std::unordered_map<Module*, ModuleDependency*>& dependencyMap, const Module* rootModule)
{
    std::vector<Module*> finishReadOrder;
    std::unordered_set<Module*> visited;
    std::unordered_set<Module*> tempVisit;
    for (Module* module : modules)
    {
        if (visited.find(module) == visited.cend())
        {
            Visit(finishReadOrder, module, visited, tempVisit, dependencyMap, rootModule);
        }
    }
    return finishReadOrder;
}

Module::Module() : 
    format(currentModuleFormat), flags(ModuleFlags::none), name(), originalFilePath(), filePathReadFrom(), referenceFilePaths(), moduleDependency(this), symbolTablePos(0), symbolTable(), 
    directoryPath(), libraryFilePaths()
{
}

Module::Module(const std::string& filePath, std::vector<ClassTypeSymbol*>& classTypes, std::vector<ClassTemplateSpecializationSymbol*>& classTemplateSpecializations)  :
    format(currentModuleFormat), flags(ModuleFlags::none), name(), originalFilePath(), filePathReadFrom(), referenceFilePaths(), moduleDependency(this), symbolTablePos(0), symbolTable(),
    directoryPath(), libraryFilePaths()
{
    SymbolReader reader(filePath);
    ModuleTag expectedTag;
    ModuleTag tag;
    tag.Read(reader);
    for (int i = 0; i < 3; ++i)
    {
        if (tag.bytes[i] != expectedTag.bytes[i])
        {
            throw std::runtime_error("Invalid Cmajor module tag read from file '" + reader.GetBinaryReader().FileName() + "', please rebuild module from sources");
        }
    }
    if (tag.bytes[3] != expectedTag.bytes[3])
    {
        throw std::runtime_error("Cmajor module format version mismatch reading from file '" + reader.GetBinaryReader().FileName() +
            "': format " + std::string(1, expectedTag.bytes[3]) + " expected, format " + std::string(1, tag.bytes[3]) + " read, please rebuild module from sources");
    }
    flags = ModuleFlags(reader.GetBinaryReader().ReadByte());
    name = reader.GetBinaryReader().ReadUtf32String();
    std::unordered_set<std::string> importSet;
    Module* rootModule = this;
    std::vector<Module*> modules;
    std::unordered_map<std::string, ModuleDependency*> moduleDependencyMap;
    std::unordered_map<std::string, Module*> readMap;
    if (SystemModuleSet::Instance().IsSystemModule(name)) SetSystemModule();
    SymbolReader reader2(filePath);
    ReadHeader(reader2, rootModule, importSet, modules, moduleDependencyMap, readMap);
    moduleDependencyMap[originalFilePath] = &moduleDependency;
    std::unordered_map<Module*, ModuleDependency*> dependencyMap;
    for (const auto& p : moduleDependencyMap)
    {
        dependencyMap[p.second->GetModule()] = p.second;
    }
    std::vector<Module*> finishReadOrder = CreateFinishReadOrder(modules, dependencyMap, rootModule);
    if (!sourceFilePaths.empty())
    {
#ifdef _WIN32
        libraryFilePath = GetFullPath(boost::filesystem::path(originalFilePath).replace_extension(".lib").generic_string());
#else
        libraryFilePath = GetFullPath(boost::filesystem::path(originalFilePath).replace_extension(".a").generic_string());
#endif
    }
    for (Module* module : finishReadOrder)
    {
        if (!module->LibraryFilePath().empty())
        {
            libraryFilePaths.push_back(module->LibraryFilePath());
        }
    }
    FinishReads(rootModule, finishReadOrder, finishReadOrder.size() - 2, classTypes, classTemplateSpecializations);
    SymbolReader reader3(filePathReadFrom);
    reader3.SetProjectBitForSymbols();
    reader3.SetModule(this);
    reader3.GetBinaryReader().Skip(symbolTablePos);
    SymbolTable st;
    st.Copy(symbolTable);
    st.Read(reader3);
    symbolTable.Import(st);
    classTypes.insert(classTypes.end(), reader3.ClassTypes().begin(), reader3.ClassTypes().end());
    classTemplateSpecializations.insert(classTemplateSpecializations.end(), reader3.ClassTemplateSpecializations().begin(), reader3.ClassTemplateSpecializations().end());
}

Module::Module(const std::u32string& name_, const std::string& filePath_) : 
    format(currentModuleFormat), flags(ModuleFlags::none), name(name_), originalFilePath(filePath_), filePathReadFrom(), referenceFilePaths(), moduleDependency(this), symbolTablePos(0), symbolTable(), 
    directoryPath(), libraryFilePaths()
{
    if (SystemModuleSet::Instance().IsSystemModule(name))
    {
        SetSystemModule();
    }
}

void Module::PrepareForCompilation(const std::vector<std::string>& references, const std::vector<std::string>& sourceFilePaths, 
    std::vector<ClassTypeSymbol*>& classTypes, std::vector<ClassTemplateSpecializationSymbol*>& classTemplateSpecializations)
{
    boost::filesystem::path mfd = originalFilePath;
    mfd.remove_filename();
    boost::filesystem::create_directories(mfd);
    SetDirectoryPath(GetFullPath(mfd.generic_string()));
    if (name == U"System.Core")
    {
        InitCoreSymbolTable(symbolTable);
    }
    this->sourceFilePaths = sourceFilePaths;
    std::unordered_set<std::string> importSet;
    Module* rootModule = this;
    std::vector<Module*> modules;
    std::unordered_map<std::string, ModuleDependency*> moduleDependencyMap;
    std::unordered_map<std::string, Module*> readMap;
    ImportModules(references, importSet, rootModule, modules, moduleDependencyMap, readMap);
    modules.push_back(this);
    moduleDependencyMap[originalFilePath] = &moduleDependency;
    std::unordered_map<Module*, ModuleDependency*> dependencyMap;
    for (const auto& p : moduleDependencyMap)
    {
        dependencyMap[p.second->GetModule()] = p.second;
    }
    std::vector<Module*> finishReadOrder = CreateFinishReadOrder(modules, dependencyMap, rootModule);
    if (!sourceFilePaths.empty())
    {
#ifdef _WIN32
        libraryFilePath = GetFullPath(boost::filesystem::path(originalFilePath).replace_extension(".lib").generic_string());
#else
        libraryFilePath = GetFullPath(boost::filesystem::path(originalFilePath).replace_extension(".a").generic_string());
#endif
    }
    for (Module* module : finishReadOrder)
    {
        if (!module->LibraryFilePath().empty())
        {
            libraryFilePaths.push_back(module->LibraryFilePath());
        }
    }
    FinishReads(rootModule, finishReadOrder, finishReadOrder.size() - 2, classTypes, classTemplateSpecializations);
}

void Module::Write(SymbolWriter& writer)
{
    ModuleTag tag;
    tag.Write(writer);
    writer.GetBinaryWriter().Write(static_cast<uint8_t>(flags));
    writer.GetBinaryWriter().Write(name);
    writer.GetBinaryWriter().Write(originalFilePath);
    uint32_t nr = referencedModules.size();
    writer.GetBinaryWriter().WriteEncodedUInt(nr);
    for (uint32_t i = 0; i < nr; ++i)
    {
        const std::unique_ptr<Module>& referencedModule = referencedModules[i];
        writer.GetBinaryWriter().Write(referencedModule->OriginalFilePath());
    }
    uint32_t ns = sourceFilePaths.size();
    writer.GetBinaryWriter().WriteEncodedUInt(ns);
    std::string cmajorRoot;
    if (IsSystemModule())
    {
        cmajorRoot = GetFullPath(CmajorRootDir());
    }
    for (uint32_t i = 0; i < ns; ++i)
    {
        std::string sfp = sourceFilePaths[i];
        if (IsSystemModule())
        {
            sfp = GetFullPath(sfp);
            if (sfp.find(cmajorRoot, 0) == 0)
            {
                sfp = sfp.substr(cmajorRoot.size());
            }
        }
        writer.GetBinaryWriter().Write(sfp);
    }
    uint32_t efn = exportedFunctions.size();
    writer.GetBinaryWriter().WriteEncodedUInt(efn);
    for (uint32_t i = 0; i < efn; ++i)
    {
        writer.GetBinaryWriter().Write(exportedFunctions[i]);
    }
    uint32_t edn = exportedData.size();
    writer.GetBinaryWriter().WriteEncodedUInt(edn);
    for (uint32_t i = 0; i < edn; ++i)
    {
        writer.GetBinaryWriter().Write(exportedData[i]);
    }
    symbolTable.Write(writer);
}

void Module::ReadHeader(SymbolReader& reader, Module* rootModule, std::unordered_set<std::string>& importSet, std::vector<Module*>& modules,
    std::unordered_map<std::string, ModuleDependency*>& dependencyMap, std::unordered_map<std::string, Module*>& readMap)
{
    ModuleTag expectedTag;
    ModuleTag tag;
    tag.Read(reader);
    for (int i = 0; i < 3; ++i)
    {
        if (tag.bytes[i] != expectedTag.bytes[i])
        {
            throw std::runtime_error("Invalid Cmajor module tag read from file '" + reader.GetBinaryReader().FileName() + "', please rebuild module from sources");
        }
    }
    if (tag.bytes[3] != expectedTag.bytes[3])
    {
        throw std::runtime_error("Cmajor module format version mismatch reading from file '" + reader.GetBinaryReader().FileName() + 
            "': format " + std::string(1, expectedTag.bytes[3]) + " expected, format " + std::string(1, tag.bytes[3]) + " read, please rebuild module from sources");
    }
    flags = ModuleFlags(reader.GetBinaryReader().ReadByte());
    name = reader.GetBinaryReader().ReadUtf32String();
    originalFilePath = reader.GetBinaryReader().ReadUtf8String();
    if (dependencyMap.find(originalFilePath) == dependencyMap.cend())
    {
        modules.push_back(this);
        dependencyMap[originalFilePath] = &moduleDependency;
    }
    filePathReadFrom = GetFullPath(reader.GetBinaryReader().FileName());
    uint32_t nr = reader.GetBinaryReader().ReadEncodedUInt();
    for (uint32_t i = 0; i < nr; ++i)
    {
        referenceFilePaths.push_back(reader.GetBinaryReader().ReadUtf8String());
    }
    uint32_t ns = reader.GetBinaryReader().ReadEncodedUInt();
    for (uint32_t i = 0; i < ns; ++i)
    {
        std::string sourceFilePath = reader.GetBinaryReader().ReadUtf8String();
        sourceFilePaths.push_back(sourceFilePath);
    }
    if (!sourceFilePaths.empty())
    {
#ifdef _WIN32
        libraryFilePath = GetFullPath(boost::filesystem::path(filePathReadFrom).replace_extension(".lib").generic_string());
#else
        libraryFilePath = GetFullPath(boost::filesystem::path(filePathReadFrom).replace_extension(".a").generic_string());
#endif
    }
    uint32_t efn = reader.GetBinaryReader().ReadEncodedUInt();
    for (uint32_t i = 0; i < efn; ++i)
    {
        exportedFunctions.push_back(reader.GetBinaryReader().ReadUtf8String());
    }
    for (const std::string& exportedFunction : exportedFunctions)
    {
        rootModule->allExportedFunctions.push_back(exportedFunction);
    }
    uint32_t edn = reader.GetBinaryReader().ReadEncodedUInt();
    for (uint32_t i = 0; i < edn; ++i)
    {
        exportedData.push_back(reader.GetBinaryReader().ReadUtf8String());
    }
    for (const std::string& data : exportedData)
    {
        rootModule->allExportedData.push_back(data);
    }
    CheckUpToDate();
    symbolTablePos = reader.GetBinaryReader().Pos();
    ImportModules(rootModule, importSet, modules, dependencyMap, readMap);
}

void Module::FinishReads(Module* rootModule, std::vector<Module*>& finishReadOrder, int prevModuleIndex,
    std::vector<ClassTypeSymbol*>& classTypes, std::vector<ClassTemplateSpecializationSymbol*>& classTemplateSpecializations)
{
    Module* prevModule = nullptr;
    if (prevModuleIndex >= 0)
    {
        prevModule = finishReadOrder[prevModuleIndex];
    }
    if (prevModule)
    {
        prevModule->FinishReads(rootModule, finishReadOrder, prevModuleIndex - 1, classTypes, classTemplateSpecializations);
        symbolTable.Import(prevModule->symbolTable);
    }
    if (this != rootModule)
    {
        SymbolReader reader(filePathReadFrom);
        reader.SetModule(this);
        reader.GetBinaryReader().Skip(symbolTablePos);
        symbolTable.Read(reader);
        classTypes.insert(classTypes.end(), reader.ClassTypes().begin(), reader.ClassTypes().end());
        classTemplateSpecializations.insert(classTemplateSpecializations.end(), reader.ClassTemplateSpecializations().begin(), reader.ClassTemplateSpecializations().end());
    }
}

void Module::SetDirectoryPath(const std::string& directoryPath_)
{
    directoryPath = directoryPath_;
}

void Module::ImportModules(Module* rootModule, std::unordered_set<std::string>& importSet, std::vector<Module*>& modules,
    std::unordered_map<std::string, ModuleDependency*>& dependencyMap, std::unordered_map<std::string, Module*>& readMap)
{
    ImportModules(referenceFilePaths, importSet, rootModule, modules, dependencyMap, readMap);
}

void Module::ImportModules(const std::vector<std::string>& references, std::unordered_set<std::string>& importSet, Module* rootModule, std::vector<Module*>& modules,
    std::unordered_map<std::string, ModuleDependency*>& moduleDependencyMap, std::unordered_map<std::string, Module*>& readMap)
{
    std::vector<std::string> allReferences = references;
    if (!IsSystemModule() && !GetGlobalFlag(GlobalFlags::profile))
    {
        allReferences.push_back(CmajorSystemModuleFilePath(GetConfig()));
    }
    Import(allReferences, importSet, rootModule, modules, moduleDependencyMap, readMap);
}

void Module::Import(const std::vector<std::string>& references, std::unordered_set<std::string>& importSet, Module* rootModule, std::vector<Module*>& modules, 
    std::unordered_map<std::string, ModuleDependency*>& moduleDependencyMap, std::unordered_map<std::string, Module*>& readMap)
{
    for (const std::string& reference : references)
    {
        if (importSet.find(reference) == importSet.cend())
        {
            importSet.insert(reference);
            std::unique_ptr<Module> referencedModule(new Module());
            std::string config = GetConfig();
            boost::filesystem::path mfn = boost::filesystem::path(reference).filename();
            boost::filesystem::path mfp;
            std::string searchedDirectories;
            if (!rootModule->IsSystemModule())
            {
                mfp = CmajorSystemLibDir(config);
                searchedDirectories.append("\n").append(mfp.generic_string());
                mfp /= mfn;
                if (!boost::filesystem::exists(mfp))
                {
                    mfp = reference;
                    if (!boost::filesystem::exists(mfp))
                    {
                        boost::filesystem::path mrd = mfp;
                        mrd.remove_filename();
                        searchedDirectories.append("\n").append(mrd.generic_string());
                        throw std::runtime_error("Could not find module reference '" + mfn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                    }
                }
            }
            else
            {
                mfp = reference;
                if (!boost::filesystem::exists(mfp))
                {
                    boost::filesystem::path mrd = mfp;
                    mrd.remove_filename();
                    searchedDirectories.append("\n").append(mrd.generic_string());
                    throw std::runtime_error("Could not find module reference '" + mfn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                }
            }
            std::string moduleFilePath = GetFullPath(mfp.generic_string());
            readMap[moduleFilePath] = referencedModule.get();
            importSet.insert(moduleFilePath);
            SymbolReader reader(moduleFilePath);
            referencedModule->ReadHeader(reader, rootModule, importSet, modules, moduleDependencyMap, readMap);
            Module* refModule = referencedModule.get();
            moduleDependency.AddReferencedModule(refModule);
            referencedModules.push_back(std::move(referencedModule));
            Import(refModule->referenceFilePaths, importSet, rootModule, modules, moduleDependencyMap, readMap);
        }
        else
        {
            std::string config = GetConfig();
            boost::filesystem::path mfn = boost::filesystem::path(reference).filename();
            boost::filesystem::path mfp;
            std::string searchedDirectories;
            if (!rootModule->IsSystemModule())
            {
                mfp = CmajorSystemLibDir(config);
                mfp /= mfn;
                if (!boost::filesystem::exists(mfp))
                {
                    mfp = reference;
                    if (!boost::filesystem::exists(mfp))
                    {
                        boost::filesystem::path mrd = mfp;
                        mrd.remove_filename();
                        searchedDirectories.append("\n").append(mrd.generic_string());
                        throw std::runtime_error("Could not find module reference '" + mfn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                    }
                }
            }
            else
            {
                mfp = reference;
                if (!boost::filesystem::exists(mfp))
                {
                    boost::filesystem::path mrd = mfp;
                    mrd.remove_filename();
                    searchedDirectories.append("\n").append(mrd.generic_string());
                    throw std::runtime_error("Could not find module reference '" + mfn.generic_string() + "'.\nDirectories searched:\n" + searchedDirectories);
                }
            }
            std::string moduleFilePath = GetFullPath(mfp.generic_string());
            auto it = readMap.find(moduleFilePath);
            if (it != readMap.cend())
            {
                Module* referencedModule = it->second;
                moduleDependency.AddReferencedModule(referencedModule);
            }
            else
            {
                throw std::runtime_error("module file path '" + moduleFilePath + "' not found from module read map for module '" + ToUtf8(name) + "'");
            }
        }
    }
}

void Module::AddExportedFunction(const std::string& exportedFunction)
{
    exportedFunctions.push_back(exportedFunction);
}

void Module::AddExportedData(const std::string& data)
{
    exportedData.push_back(data);
}

void Module::Dump()
{
    CodeFormatter formatter(std::cout);
    formatter.WriteLine("========================");
    formatter.WriteLine("MODULE " + ToUtf8(name));
    formatter.WriteLine("========================");
    formatter.WriteLine();
    formatter.WriteLine("format: " + std::string(1, format));
    formatter.WriteLine("flags: " + ModuleFlagStr(flags));
    formatter.WriteLine("original file path: " + originalFilePath);
    formatter.WriteLine("file path read from: " + filePathReadFrom);
    if (!libraryFilePath.empty())
    {
        formatter.WriteLine("library file path: " + libraryFilePath);
    }
    int n = referenceFilePaths.size();
    if (n > 0)
    {
        formatter.WriteLine("reference file paths:");
        formatter.IncIndent();
        for (int i = 0; i < n; ++i)
        {
            formatter.WriteLine(referenceFilePaths[i]);
        }
        formatter.DecIndent();
    }
    n = sourceFilePaths.size();
    if (n > 0)
    {
        formatter.WriteLine("source file paths:");
        formatter.IncIndent();
        for (int i = 0; i < n; ++i)
        {
            formatter.WriteLine(sourceFilePaths[i]);
        }
        formatter.DecIndent();
    }
    formatter.WriteLine("module dependencies:");
    formatter.IncIndent();
    formatter.WriteLine(ToUtf8(Name()));
    moduleDependency.Dump(formatter);
    formatter.DecIndent();
    SymbolCollector collector;
    symbolTable.GlobalNs().Accept(&collector);
    collector.SortByFullName();
    if (!collector.BasicTypes().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("BASIC TYPES");
        for (BasicTypeSymbol* basicType : collector.BasicTypes())
        {
            formatter.WriteLine();
            basicType->Dump(formatter);
        }
    }
    if (!collector.Classes().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("CLASSES");
        for (ClassTypeSymbol* class_ : collector.Classes())
        {
            formatter.WriteLine();
            class_->Dump(formatter);
        }
    }
    if (!collector.Interfaces().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("INTERFACES");
        for (InterfaceTypeSymbol* interface : collector.Interfaces())
        {
            formatter.WriteLine();
            interface->Dump(formatter);
        }
    }
    if (!collector.Functions().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("FUNCTIONS");
        for (FunctionSymbol* function : collector.Functions())
        {
            formatter.WriteLine();
            function->Dump(formatter);
        }
    }
    if (!collector.Typedefs().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("TYPEDEFS");
        for (TypedefSymbol* typedef_ : collector.Typedefs())
        {
            formatter.WriteLine();
            typedef_->Dump(formatter);
        }
    }
    if (!collector.Concepts().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("CONCEPTS");
        for (ConceptSymbol* concept_ : collector.Concepts())
        {
            formatter.WriteLine();
            concept_->Dump(formatter);
        }
    }
    if (!collector.Delegates().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("DELEGATES");
        for (DelegateTypeSymbol* delegate_ : collector.Delegates())
        {
            formatter.WriteLine();
            delegate_->Dump(formatter);
        }
    }
    if (!collector.ClassDelegates().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("CLASS DELEGATES");
        for (ClassDelegateTypeSymbol* classDelegate : collector.ClassDelegates())
        {
            formatter.WriteLine();
            classDelegate->Dump(formatter);
        }
    }
    if (!collector.Constants().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("CONSTANTS");
        for (ConstantSymbol* constant : collector.Constants())
        {
            formatter.WriteLine();
            constant->Dump(formatter);
        }
    }
    if (!collector.EnumeratedTypes().empty())
    {
        formatter.WriteLine();
        formatter.WriteLine("ENUMERATED TYPES");
        for (EnumTypeSymbol* enumeratedType : collector.EnumeratedTypes())
        {
            formatter.WriteLine();
            enumeratedType->Dump(formatter);
        }
    }
}

void Module::CheckUpToDate()
{
    if (sourceFilePaths.empty()) return;
    std::string cmajorRootDir = GetFullPath(CmajorRootDir());
    boost::filesystem::path libDirPath = boost::filesystem::path(originalFilePath).parent_path();
    for (const std::string& sourceFilePath : sourceFilePaths)
    {
        boost::filesystem::path sfp(sourceFilePath);
        if (IsSystemModule())
        {
            sfp = cmajorRootDir / sfp;
        }
        if (boost::filesystem::exists(sfp))
        {
#ifdef _WIN32
            boost::filesystem::path objectFilePath = libDirPath / sfp.filename().replace_extension(".obj");
#else
            boost::filesystem::path objectFilePath = libDirPath / sfp.filename().replace_extension(".o");
#endif
            if (boost::filesystem::exists(objectFilePath))
            {
                if (boost::filesystem::last_write_time(sfp) > boost::filesystem::last_write_time(objectFilePath))
                {
                    Warning warning(name, "source file '" + GetFullPath(sfp.generic_string()) + "' is more recent than object file '" +
                        GetFullPath(objectFilePath.generic_string()) + "'");
                    bool found = false;
                    for (const Warning& prev : CompileWarningCollection::Instance().Warnings())
                    {
                        if (prev.Message() == warning.Message())
                        {
                            found = true;
                            break;
                        }
                    }
                    if (!found)
                    {
                        CompileWarningCollection::Instance().AddWarning(warning);
                    }
                }
            }
        }
    }
}

void InitModule()
{
    SystemModuleSet::Init();
}

void DoneModule()
{
    SystemModuleSet::Done();
}

} } // namespace cmajor::symbols
