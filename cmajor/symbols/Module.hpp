// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_SYMBOLS_MODULE_INCLUDED
#define CMAJOR_SYMBOLS_MODULE_INCLUDED
#include <cmajor/symbols/SymbolTable.hpp>
#include <cmajor/util/CodeFormatter.hpp>

namespace cmajor { namespace symbols {

extern const char* cmajorModuleTag;

struct ModuleTag
{
    ModuleTag();
    void Write(SymbolWriter& writer);
    void Read(SymbolReader& reader);
    uint8_t bytes[4];
};

const uint8_t moduleFormat_1 = uint8_t('1');
const uint8_t currentModuleFormat = moduleFormat_1;

enum class ModuleFlags : uint8_t
{
    none = 0, system = 1 << 0
};

inline ModuleFlags operator|(ModuleFlags left, ModuleFlags right)
{
    return ModuleFlags(uint8_t(left) | uint8_t(right));
}

inline ModuleFlags operator&(ModuleFlags left, ModuleFlags right)
{
    return ModuleFlags(uint8_t(left) & uint8_t(right));
}

class Module;

class ModuleDependency
{
public:
    ModuleDependency(Module* module_);
    Module* GetModule() const { return module; }
    void AddReferencedModule(Module* referencedModule);
    const std::vector<Module*>& ReferencedModules() const { return referencedModules; }
    void Dump(CodeFormatter& formatter);
private:
    Module* module;
    std::vector<Module*> referencedModules;
};

class Module
{
public:
    Module();
    Module(const std::u32string& name_, const std::string& filePath_);
    uint8_t Format() const { return format; }
    ModuleFlags Flags() const { return flags; }
    const std::u32string& Name() const { return name; }
    const std::string& OriginalFilePath() const { return originalFilePath; }
    const std::string& FilePathReadFrom() const { return filePathReadFrom; }
    const std::string& LibraryFilePath() const { return libraryFilePath; }
    void PrepareForCompilation(const std::vector<std::string>& references, const std::vector<std::string>& sourceFilePaths,
        std::vector<ClassTypeSymbol*>& classTypes, std::vector<ClassTemplateSpecializationSymbol*>& classTemplateSpecializations);
    SymbolTable& GetSymbolTable() { return symbolTable; }
    void Write(SymbolWriter& writer);
    void SetDirectoryPath(const std::string& directoryPath_);
    const std::string& DirectoryPath() const { return directoryPath; }
    const std::vector<std::string>& LibraryFilePaths() const { return libraryFilePaths; }
    bool IsSystemModule () const { return GetFlag(ModuleFlags::system); }
    void SetSystemModule() { SetFlag(ModuleFlags::system); }
    bool GetFlag(ModuleFlags flag) const { return (flags & flag) != ModuleFlags::none; }
    void SetFlag(ModuleFlags flag) { flags = flags | flag; }
    void AddExportedFunction(const std::string& exportedFunction);
    void AddExportedData(const std::string& data);
    const std::vector<std::string>& ExportedFunctions() { return exportedFunctions; }
    const std::vector<std::string>& ExportedData() { return exportedData; }
    const std::vector<std::string>& AllExportedFunctions() const { return allExportedFunctions; }
    const std::vector<std::string>& AllExportedData() const { return allExportedData; }
private:
    uint8_t format;
    ModuleFlags flags;
    std::u32string name;
    std::string originalFilePath;
    std::string filePathReadFrom;
    std::string libraryFilePath;
    std::vector<std::string> referenceFilePaths;
    std::vector<std::string> sourceFilePaths;
    std::vector<std::string> exportedFunctions;
    std::vector<std::string> exportedData;
    std::vector<std::string> allExportedFunctions;
    std::vector<std::string> allExportedData;
    ModuleDependency moduleDependency;
    std::vector<std::unique_ptr<Module>> referencedModules;
    uint32_t symbolTablePos;
    SymbolTable symbolTable;
    std::string directoryPath;
    std::vector<std::string> libraryFilePaths;
    void ReadHeader(SymbolReader& reader, Module* rootModule, std::unordered_set<std::string>& importSet, std::vector<Module*>& modules,
        std::unordered_map<std::string, ModuleDependency*>& moduleDependencyMap, std::unordered_map<std::string, Module*>& readMap);
    void FinishReads(Module* rootModule, std::vector<Module*>& finishReadOrder, int prevModuleIndex,
        std::vector<ClassTypeSymbol*>& classTypes, std::vector<ClassTemplateSpecializationSymbol*>& classTemplateSpecializations);
    void ImportModules(Module* rootModule, std::unordered_set<std::string>& importSet, std::vector<Module*>& modules,
        std::unordered_map<std::string, ModuleDependency*>& dependencyMap, std::unordered_map<std::string, Module*>& readMap);
    void ImportModules(const std::vector<std::string>& references, std::unordered_set<std::string>& importSet, Module* rootModule, std::vector<Module*>& modules,
        std::unordered_map<std::string, ModuleDependency*>& moduleDependencyMap, std::unordered_map<std::string, Module*>& readMap);
    void Import(const std::vector<std::string>& references, std::unordered_set<std::string>& importSet, Module* rootModule, std::vector<Module*>& modules, 
        std::unordered_map<std::string, ModuleDependency*>& moduleDependencyMap, std::unordered_map<std::string, Module*>& readMap);
};

void InitModule();
void DoneModule();

} } // namespace cmajor::symbols

#endif // CMAJOR_SYMBOLS_MODULE_INCLUDED
