// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Statics.hpp>
#include <cmajor/util/Error.hpp>
#include <memory>
#include <mutex>
#include <unordered_map>

namespace cmajor {namespace rt {

class StaticInitTable
{
public:
    static void Init();
    static void Done();
    static StaticInitTable& Instance() { return *instance; }
    void AllocateMutexes(const std::vector<uint32_t>& staticClassIds);
    void BeginCriticalSection(uint32_t classId);
    void EndCriticalSection(uint32_t classId);
private:
    static std::unique_ptr<StaticInitTable> instance;
    std::vector<std::unique_ptr<std::recursive_mutex>> mutexes;
    std::unordered_map<uint32_t, int> mutexMap;
};

std::unique_ptr<StaticInitTable> StaticInitTable::instance;

void StaticInitTable::Init()
{
    instance.reset(new StaticInitTable());
}

void StaticInitTable::Done()
{
    instance.reset();
}

void StaticInitTable::AllocateMutexes(const std::vector<uint32_t>& staticClassIds)
{
    int n = staticClassIds.size();
    for (int i = 0; i < n; ++i)
    {
        uint32_t classId = staticClassIds[i];
        mutexMap[classId] = mutexes.size();
        mutexes.push_back(std::unique_ptr<std::recursive_mutex>(new std::recursive_mutex()));
    }
}

void StaticInitTable::BeginCriticalSection(uint32_t classId)
{
    auto it = mutexMap.find(classId);
    if (it != mutexMap.cend())
    {
        int mutexIndex = it->second;
        Assert(mutexIndex >= 0 && mutexIndex < mutexes.size(), "invalid mutex index");
        std::recursive_mutex* mutex = mutexes[mutexIndex].get();
        mutex->lock();
    }
    else
    {
        Assert(false, "invalid class id");
    }
}

void StaticInitTable::EndCriticalSection(uint32_t classId)
{
    auto it = mutexMap.find(classId);
    if (it != mutexMap.cend())
    {
        int mutexIndex = it->second;
        Assert(mutexIndex >= 0 && mutexIndex < mutexes.size(), "invalid mutex index");
        std::recursive_mutex* mutex = mutexes[mutexIndex].get();
        mutex->unlock();
    }
    else
    {
        Assert(false, "invalid class id");
    }
}

void AllocateMutexes(const std::vector<uint32_t>& staticClassIds)
{
    StaticInitTable::Instance().AllocateMutexes(staticClassIds);
}

typedef void(*destructor_ptr)(void* arg);

struct Destruction
{
    Destruction(destructor_ptr destructor_, void* arg_, Destruction* next_) : destructor(destructor_), arg(arg_), next(next_)
    {
    }
    destructor_ptr destructor;
    void* arg;
    Destruction* next;
};

Destruction* destructionList = nullptr;

void ExecuteDestructors()
{
    Destruction* destruction = destructionList;
    while (destruction)
    {
        destructionList = destructionList->next;
        destruction->destructor(destruction->arg);
        delete destruction;
        destruction = destructionList;
    }
}

void InitStatics()
{
    StaticInitTable::Init();
}

void DoneStatics()
{
    ExecuteDestructors();
    StaticInitTable::Done();
}

} }  // namespace cmajor::rt

extern "C" RT_API void RtBeginStaticInitCriticalSection(uint32_t staticClassId)
{
    cmajor::rt::StaticInitTable::Instance().BeginCriticalSection(staticClassId);
}

extern "C" RT_API void RtEndStaticInitCriticalSection(uint32_t staticClassId)
{
    cmajor::rt::StaticInitTable::Instance().EndCriticalSection(staticClassId);
}

std::mutex destructionMutex;
 
extern "C" RT_API void RtEnqueueDestruction(void* destructor, void* arg)
{
    std::lock_guard<std::mutex> lock(destructionMutex);
    cmajor::rt::destructionList = new cmajor::rt::Destruction(static_cast<cmajor::rt::destructor_ptr>(destructor), arg, cmajor::rt::destructionList);
}
