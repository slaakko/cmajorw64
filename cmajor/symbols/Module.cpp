// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/symbols/Module.hpp>
#include <cmajor/symbols/SymbolWriter.hpp>
#include <cmajor/symbols/SymbolReader.hpp>
#include <cmajor/symbols/GlobalFlags.hpp>
#include <cmajor/ast/Project.hpp>
#include <cmajor/util/Path.hpp>
#include <cmajor/util/Unicode.hpp>
#include <boost/filesystem.hpp>

namespace cmajor { namespace symbols {

using namespace cmajor::unicode;

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

ModuleDependency::ModuleDependency(Module* module_) : module(module_)
{
}

void ModuleDependency::AddReferencedModule(Module* referencedModule)
{
    referencedModules.push_back(referencedModule);
}

void ModuleDependency::Dump(CodeFormatter& formatter)
{
    // todo
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

Module::Module(const std::u32string& name_, const std::string& filePath_) : 
    format(currentModuleFormat), flags(ModuleFlags::none), name(name_), originalFilePath(filePath_), filePathReadFrom(), referenceFilePaths(), moduleDependency(this), symbolTablePos(0), symbolTable(), 
    directoryPath(), libraryFilePaths()
{
    if (SystemModuleSet::Instance().IsSystemModule(name))
    {
        SetSystemModule();
    }
}

void Module::PrepareForCompilation(const std::vector<std::string>& references, const std::vector<std::string>& sourceFilePaths)
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
    const Module* rootModule = this;
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
        libraryFilePath = GetFullPath(boost::filesystem::path(originalFilePath).replace_extension(".lib").generic_string());
    }
    for (Module* module : finishReadOrder)
    {
        if (!module->LibraryFilePath().empty())
        {
            libraryFilePaths.push_back(module->LibraryFilePath());
        }
    }
    FinishReads(rootModule, finishReadOrder, finishReadOrder.size() - 2);
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
    for (uint32_t i = 0; i < ns; ++i)
    {
        writer.GetBinaryWriter().Write(sourceFilePaths[i]);
    }
    symbolTable.Write(writer);
}

void Module::ReadHeader(SymbolReader& reader, const Module* rootModule, std::unordered_set<std::string>& importSet, std::vector<Module*>& modules,
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
    filePathReadFrom = reader.GetBinaryReader().FileName();
    uint32_t nr = reader.GetBinaryReader().ReadEncodedUInt();
    for (uint32_t i = 0; i < nr; ++i)
    {
        referenceFilePaths.push_back(reader.GetBinaryReader().ReadUtf8String());
    }
    uint32_t ns = reader.GetBinaryReader().ReadEncodedUInt();
    for (uint32_t i = 0; i < ns; ++i)
    {
        sourceFilePaths.push_back(reader.GetBinaryReader().ReadUtf8String());
    }
    if (!sourceFilePaths.empty())
    {
        libraryFilePath = GetFullPath(boost::filesystem::path(filePathReadFrom).replace_extension(".lib").generic_string());
    }
    symbolTablePos = reader.GetBinaryReader().Pos();
    ImportModules(rootModule, importSet, modules, dependencyMap, readMap);
}

void Module::FinishReads(const Module* rootModule, std::vector<Module*>& finishReadOrder, int prevModuleIndex)
{
    Module* prevModule = nullptr;
    if (prevModuleIndex >= 0)
    {
        prevModule = finishReadOrder[prevModuleIndex];
    }
    if (prevModule)
    {
        prevModule->FinishReads(rootModule, finishReadOrder, prevModuleIndex - 1);
        symbolTable.Import(prevModule->symbolTable);
    }
    if (this != rootModule)
    {
        SymbolReader reader(filePathReadFrom);
        reader.GetBinaryReader().Skip(symbolTablePos);
        symbolTable.Read(reader);
    }
}

void Module::SetDirectoryPath(const std::string& directoryPath_)
{
    directoryPath = directoryPath_;
}

void Module::ImportModules(const Module* rootModule, std::unordered_set<std::string>& importSet, std::vector<Module*>& modules,
    std::unordered_map<std::string, ModuleDependency*>& dependencyMap, std::unordered_map<std::string, Module*>& readMap)
{
    ImportModules(referenceFilePaths, importSet, rootModule, modules, dependencyMap, readMap);
}

void Module::ImportModules(const std::vector<std::string>& references, std::unordered_set<std::string>& importSet, const Module* rootModule, std::vector<Module*>& modules,
    std::unordered_map<std::string, ModuleDependency*>& moduleDependencyMap, std::unordered_map<std::string, Module*>& readMap)
{
    std::vector<std::string> allReferences = references;
    if (!IsSystemModule())
    {
        allReferences.push_back(CmajorSystemModuleFilePath(GetConfig()));
    }
    Import(allReferences, importSet, rootModule, modules, moduleDependencyMap, readMap);
}

void Module::Import(const std::vector<std::string>& references, std::unordered_set<std::string>& importSet, const Module* rootModule, std::vector<Module*>& modules, 
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

void InitModule()
{
    SystemModuleSet::Init();
}

void DoneModule()
{
    SystemModuleSet::Done();
}

} } // namespace cmajor::symbols
