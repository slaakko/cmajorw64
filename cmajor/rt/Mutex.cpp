// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Mutex.hpp>
#include <cmajor/util/Error.hpp>
#include <atomic>
#include <vector>
#include <unordered_map>
#include <mutex>

namespace cmajor { namespace rt {

class MutexTable
{
public:
    static void Init();
    static void Done();
    static MutexTable& Instance() { Assert(instance, "mutex table not initialized"); return *instance; }
    int32_t AllocateMutex();
    void FreeMutex(int32_t mutexId);
    void LockMutex(int32_t mutexId);
    void UnlockMutex(int32_t mutexId);
private:
    MutexTable();
    static std::unique_ptr<MutexTable> instance;
    const int32_t numNoLockMutexes = 256;
    std::atomic<int32_t> nextMutexId;
    std::vector<std::unique_ptr<std::mutex>> noLockMutexes;
    std::unordered_map<int32_t, std::unique_ptr<std::mutex>> mutexMap;
    std::mutex mtx;
};

std::unique_ptr<MutexTable> MutexTable::instance;

void MutexTable::Init()
{
    instance.reset(new MutexTable());
}

void MutexTable::Done()
{
    instance.reset();
}

MutexTable::MutexTable() : nextMutexId(1), noLockMutexes()
{
    noLockMutexes.resize(256);
}

int32_t MutexTable::AllocateMutex()
{
    int32_t mutexId = nextMutexId++;
    if (mutexId < numNoLockMutexes)
    {
        noLockMutexes[mutexId].reset(new std::mutex());
        return mutexId;
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        mutexMap[mutexId].reset(new std::mutex());
        return mutexId;
    }
}

void MutexTable::FreeMutex(int32_t mutexId)
{
    if (mutexId < numNoLockMutexes)
    {
        noLockMutexes[mutexId].reset();
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        mutexMap[mutexId].reset();
    }
}

void MutexTable::LockMutex(int32_t mutexId)
{
    if (mutexId < numNoLockMutexes)
    {
        noLockMutexes[mutexId]->lock();
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        mutexMap[mutexId]->lock();
    }
}

void MutexTable::UnlockMutex(int32_t mutexId)
{
    if (mutexId < numNoLockMutexes)
    {
        noLockMutexes[mutexId]->unlock();
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        mutexMap[mutexId]->unlock();;
    }
}

void InitMutex()
{
    MutexTable::Init();
}

void DoneMutex()
{
    MutexTable::Done();
}

} } // namespace cmajor::rt

extern "C" RT_API int32_t RtAllocateMutex()
{
    return cmajor::rt::MutexTable::Instance().AllocateMutex();
}

extern "C" RT_API void RtFreeMutex(int32_t mutexId)
{
    return cmajor::rt::MutexTable::Instance().FreeMutex(mutexId);
}

extern "C" RT_API void RtLockMutex(int32_t mutexId)
{
    cmajor::rt::MutexTable::Instance().LockMutex(mutexId);
}

extern "C" RT_API void RtUnlockMutex(int32_t mutexId)
{
    cmajor::rt::MutexTable::Instance().UnlockMutex(mutexId);
}
