// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Classes.hpp>
#include <cmajor/rt/Statics.hpp>
#include <cmajor/rt/Error.hpp>
#include <cmajor/rt/Io.hpp>
#include <cmajor/util/System.hpp>
#include <cmajor/util/BinaryReader.hpp>
#include <cmajor/util/Path.hpp>
#include <cmajor/util/Prime.hpp>
#include <boost/filesystem.hpp>
#include <boost/uuid/uuid_io.hpp>
#include <boost/functional/hash.hpp>
#include <unordered_map>
#include <sstream>
#ifdef _WIN32
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#else
#include <unistd.h>
#include <dlfcn.h>
#endif

namespace cmajor { namespace rt {

using namespace cmajor::util;

class ClassIdMap
{
public:
    static void Init();
    static void Done();
    static ClassIdMap& Instance() { return *instance; }
    void SetClassId(const boost::uuids::uuid& typeId, uint64_t classId);
    uint64_t GetClassId(const boost::uuids::uuid& typeId) const;
private:
    static std::unique_ptr<ClassIdMap> instance;
    std::unordered_map<boost::uuids::uuid, uint64_t, boost::hash<boost::uuids::uuid>> classIdMap;
};

std::unique_ptr<ClassIdMap> ClassIdMap::instance;

void ClassIdMap::Init()
{
    instance.reset(new ClassIdMap());
}

void ClassIdMap::Done()
{
    instance.reset();
}


void ClassIdMap::SetClassId(const boost::uuids::uuid& typeId, uint64_t classId)
{
    classIdMap[typeId] = classId;
}

uint64_t ClassIdMap::GetClassId(const boost::uuids::uuid& typeId) const
{
    auto it = classIdMap.find(typeId);
    if (it != classIdMap.cend())
    {
        return it->second;
    }
    else
    {
        std::stringstream s;
        s << "internal error : class id for type id " << typeId << " not found.\n";
        std::string str = s.str();
        RtWrite(stdErrFileHandle, reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
        exit(exitCodeInternalError);
    }
}

struct ClassInfo
{
    ClassInfo(const boost::uuids::uuid& typeId_, const std::string& vmtObjectName_, const boost::uuids::uuid& baseTypeId_) :
        typeId(typeId_), vmtObjectName(vmtObjectName_), baseTypeId(baseTypeId_), baseClass(nullptr), level(0), key(0), id(0) {}
    boost::uuids::uuid typeId;
    std::string vmtObjectName;
    boost::uuids::uuid  baseTypeId;
    ClassInfo* baseClass;
    int level;
    uint64_t key;
    uint64_t id;
};

void ReadClasses(const std::string& classFilePath, std::vector<std::unique_ptr<ClassInfo>>& classInfos, std::vector<boost::uuids::uuid>& staticClassIds)
{
    BinaryReader reader(classFilePath);
    uint32_t n = reader.ReadULEB128UInt();
    for (uint32_t i = 0; i < n; ++i)
    {
        boost::uuids::uuid typeId;
        reader.ReadUuid(typeId);
        std::string vmtObjectName = reader.ReadUtf8String();
        boost::uuids::uuid baseTypeId;
        reader.ReadUuid(baseTypeId);
        classInfos.push_back(std::unique_ptr<ClassInfo>(new ClassInfo(typeId, vmtObjectName, baseTypeId)));
    }
    uint32_t ns = reader.ReadULEB128UInt();
    for (uint32_t i = 0; i < ns; ++i)
    {
        boost::uuids::uuid typeId;
        reader.ReadUuid(typeId);
        staticClassIds.push_back(typeId);
    }
}

void ResolveBaseClasses(const std::vector<std::unique_ptr<ClassInfo>>& classes)
{
    std::unordered_map<boost::uuids::uuid, ClassInfo*, boost::hash<boost::uuids::uuid>> classMap;
    for (const std::unique_ptr<ClassInfo>& cls : classes)
    {
        classMap[cls->typeId] = cls.get();
    }
    for (const std::unique_ptr<ClassInfo>& cls : classes)
    {
        if (!cls->baseTypeId.is_nil())
        {
            auto it = classMap.find(cls->baseTypeId);
            if (it != classMap.cend())
            {
                cls->baseClass = it->second;
            }
            else
            {
                throw std::runtime_error("error assigning class id's: class with id " + boost::uuids::to_string(cls->baseTypeId) + " not found");
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
        throw std::runtime_error("error assigning class id's: could not resolve address of a symbol '" + symbolName + "'");
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
/*
    if (!symbolAddress)
    {
        throw std::runtime_error("error assigning class id's: could not resolve address of a symbol '" + symbolName);
    }
*/
    return symbolAddress;
}

#endif 

void SetClassIdsToVmts(const std::vector<ClassInfo*>& classesByPriority)
{
    for (ClassInfo* cls : classesByPriority)
    {
        void* vmtObjectAddress = GetSymbolAddess(cls->vmtObjectName);
        if (vmtObjectAddress)
        {
            *reinterpret_cast<uint64_t*>(vmtObjectAddress) = cls->id;   // class id field is the first field of vmt so it's address is the same as the address of the vmt
        }
    }
}

void SetClassIdsToClassIdMap(const std::vector<ClassInfo*>& classesByPriority)
{
    for (ClassInfo* cls : classesByPriority)
    {
        ClassIdMap::Instance().SetClassId(cls->typeId, cls->id);
    }
}

uint64_t GetClassId(const boost::uuids::uuid& typeId)
{
    return ClassIdMap::Instance().GetClassId(typeId);
}

void InitClasses()
{
    try
    {
        ClassIdMap::Init();
        std::string executablePath = GetPathToExecutable();
        boost::filesystem::path classFilePath = boost::filesystem::path(executablePath).replace_extension(".cls");
        if (!exists(classFilePath))
        {
            throw std::runtime_error("error assigning class id's: class file '" + GetFullPath(classFilePath.generic_string()) + "' does not exist");
        }
        std::vector<std::unique_ptr<ClassInfo>> polyMorphicClasses;
        std::vector<boost::uuids::uuid> staticClassIds;
        ReadClasses(GetFullPath(classFilePath.generic_string()), polyMorphicClasses, staticClassIds);
        ResolveBaseClasses(polyMorphicClasses);
        AssignLevels(polyMorphicClasses);
        std::vector<ClassInfo*> classesByPriority = GetClassesByPriority(polyMorphicClasses);
        AssignKeys(classesByPriority);
        AssignClassIds(classesByPriority);
        SetClassIdsToVmts(classesByPriority);
        SetClassIdsToClassIdMap(classesByPriority);
        AllocateMutexes(staticClassIds);
    }
    catch (const std::exception& ex)
    {
        std::stringstream s;
        s << "internal error in program initialization: " << ex.what() << "\n";
        std::string str = s.str();
        RtWrite(stdErrFileHandle, reinterpret_cast<const uint8_t*>(str.c_str()), str.length());
        exit(exitCodeInternalError);
    }
}

void DoneClasses()
{
    ClassIdMap::Done();
}

} } // namespace cmajor::rt
