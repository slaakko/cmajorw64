// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Classes.hpp>
#include <cmajor/rt/Statics.hpp>
#include <cmajor/util/System.hpp>
#include <cmajor/util/BinaryReader.hpp>
#include <cmajor/util/Path.hpp>
#include <cmajor/util/Prime.hpp>
#include <boost/filesystem.hpp>
#include <unordered_map>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <unistd.h>
#endif

namespace cmajor { namespace rt {

using namespace cmajor::util;

struct ClassInfo
{
    ClassInfo(uint32_t typeId_, const std::string& vmtObjectName_, uint32_t baseTypeId_) : 
        typeId(typeId_), vmtObjectName(vmtObjectName_), baseTypeId(baseTypeId_), baseClass(nullptr), level(0), key(0), id(0) {}
    uint32_t typeId;
    std::string vmtObjectName;
    uint32_t baseTypeId;
    ClassInfo* baseClass;
    int level;
    uint64_t key;
    uint64_t id;
};

void ReadClasses(const std::string& classFilePath, std::vector<std::unique_ptr<ClassInfo>>& classInfos, std::vector<uint32_t>& staticClassIds)
{
    BinaryReader reader(classFilePath);
    uint32_t n = reader.ReadEncodedUInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        uint32_t typeId = reader.ReadEncodedUInt();
        std::string vmtObjectName = reader.ReadUtf8String();
        uint32_t baseTypeId = reader.ReadEncodedUInt();
        classInfos.push_back(std::unique_ptr<ClassInfo>(new ClassInfo(typeId, vmtObjectName, baseTypeId)));
    }
    uint32_t ns = reader.ReadEncodedUInt();
    for (uint32_t i = 0; i < ns; ++i)
    {
        uint32_t typeId = reader.ReadEncodedUInt();
        staticClassIds.push_back(typeId);
    }
}

void ResolveBaseClasses(const std::vector<std::unique_ptr<ClassInfo>>& classes)
{
    std::unordered_map<uint32_t, ClassInfo*> classMap;
    for (const std::unique_ptr<ClassInfo>& cls : classes)
    {
        classMap[cls->typeId] = cls.get();
    }
    for (const std::unique_ptr<ClassInfo>& cls : classes)
    {
        if (cls->baseTypeId != 0)
        {
            auto it = classMap.find(cls->baseTypeId);
            if (it != classMap.cend())
            {
                cls->baseClass = it->second;
            }
            else
            {
                throw std::runtime_error("error assigning class id's: class with id " + std::to_string(cls->baseTypeId) + " not found");
            }
        }
    }
}

int NumberOfAncestors(ClassInfo* cls)
{
    int numAncestors = 0;
    ClassInfo* baseClass = cls->baseClass;
    while (baseClass)
    {
        ++numAncestors;
        baseClass = baseClass->baseClass;
    }
    return numAncestors;
}

void AssignLevels(const std::vector<std::unique_ptr<ClassInfo>>& classes)
{
    for (const std::unique_ptr<ClassInfo>& cls : classes)
    {
        cls->level = NumberOfAncestors(cls.get());
    }
}

struct PriorityGreater
{
    bool operator()(ClassInfo* left, ClassInfo* right) const
    {
        if (left->level < right->level) return true;
        return false;
    }
};

std::vector<ClassInfo*> GetClassesByPriority(const std::vector<std::unique_ptr<ClassInfo>>& classes)
{
    std::vector<ClassInfo*> classesByPriority;
    for (const std::unique_ptr<ClassInfo>& cls : classes)
    {
        classesByPriority.push_back(cls.get());
    }
    std::sort(classesByPriority.begin(), classesByPriority.end(), PriorityGreater());
    return classesByPriority;
}

void AssignKeys(std::vector<ClassInfo*>& classesByPriority)
{
    uint64_t key = 2;
    for (ClassInfo* cls : classesByPriority)
    {
        cls->key = key;
        key = cmajor::util::NextPrime(key + 1);
    }
}

uint64_t ComputeClassId(ClassInfo* cls)
{
    uint64_t classId = cls->key;
    ClassInfo* baseClass = cls->baseClass;
    while (baseClass)
    {
        classId *= baseClass->key;
        baseClass = baseClass->baseClass;
    }
    if (classId == 0)
    {
        throw std::runtime_error("error assigning class id's: invalid resulting class id 0");
    }
    return classId;
}

void AssignClassIds(std::vector<ClassInfo*>& classesByPriority)
{
    for (ClassInfo* cls : classesByPriority)
    {
        cls->id = ComputeClassId(cls);
    }
}

#ifdef _WIN32

void* GetSymbolAddess(const std::string& symbolName)
{
    HMODULE exeHandle = GetModuleHandle(NULL);
    if (!exeHandle)
    {
        throw std::runtime_error("error assigning class id's: could not get the handle of the executable");
    }
    void* symbolAddress = GetProcAddress(exeHandle, symbolName.c_str());
    if (!symbolAddress)
    {
        throw std::runtime_error("error assigning class id's: could not resolve address of a symbol '" + symbolName);
    }
    return symbolAddress;
}

#else

void* GetSymbolAddess(const std::string& symbolName)
{
    void* exeHandle = dlopen(NULL, RTLD_NOW);
    if (!exeHandle)
    {
        throw std::runtime_error("error assigning class id's: could not get the handle of the executable");
    }
    void* symbolAddress = dlsym(exeHandle, symbolName.c_str());
    dlclose(exeHandle);
    if (!symbolAddress)
    {
        throw std::runtime_error("error assigning class id's: could not resolve address of a symbol '" + symbolName);
    }
    return symbolAddress;
}

#endif 

void SetClassIdsToVmts(const std::vector<ClassInfo*>& classesByPriority)
{
    for (ClassInfo* cls : classesByPriority)
    {
        void* vmtObjectAddress = GetSymbolAddess(cls->vmtObjectName);
        *reinterpret_cast<uint64_t*>(vmtObjectAddress) = cls->id;   // class id field is the first field of vmt so it's address is the same as the address of the vmt
    }
}

void InitClasses()
{
    std::string executablePath = GetPathToExecutable();
    boost::filesystem::path classFilePath = boost::filesystem::path(executablePath).replace_extension(".cls");
    if (!exists(classFilePath))
    {
        throw std::runtime_error("error assigning class id's: class file '" + GetFullPath(classFilePath.generic_string()) + "' does not exist");
    }
    std::vector<std::unique_ptr<ClassInfo>> polyMorphicClasses;
    std::vector<uint32_t> staticClassIds;
    ReadClasses(GetFullPath(classFilePath.generic_string()), polyMorphicClasses, staticClassIds);
    ResolveBaseClasses(polyMorphicClasses);
    AssignLevels(polyMorphicClasses);
    std::vector<ClassInfo*> classesByPriority = GetClassesByPriority(polyMorphicClasses);
    AssignKeys(classesByPriority);
    AssignClassIds(classesByPriority);
    SetClassIdsToVmts(classesByPriority);
    AllocateMutexes(staticClassIds);
}

void DoneClasses()
{
}

} } // namespace cmajor::rt
