// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Profile.hpp>
#include <cmajor/rt/InitDone.hpp>
#include <cmajor/util/System.hpp>
#include <cmajor/util/Path.hpp>
#include <cmajor/util/Unicode.hpp>
#include <cmajor/util/BinaryWriter.hpp>
#include <memory>
#include <vector>
#include <chrono>
#include <mutex>
#include <fstream>

namespace cmajor {namespace rt {

using namespace cmajor::util;
using namespace cmajor::unicode;

enum class TimePointKind : uint8_t
{
    start = 0, end = 1
};

struct FunctionProfileData
{
    FunctionProfileData(TimePointKind kind_, uint32_t functionId_) : timePoint(std::chrono::high_resolution_clock::now()), functionId(functionId_), kind(kind_) {}
    std::chrono::high_resolution_clock::time_point timePoint;
    uint32_t functionId;
    TimePointKind kind;
};

class Profiler
{
public:
    static void Init();
    static void Done();
    static Profiler& Instance() { return *instance; }
    void StartFunction(uint32_t functionId);
    void EndFunction(uint32_t functionId);
    std::vector<FunctionProfileData>* CreateFunctionProfileData();
    void WriteData();
private:
    static std::unique_ptr<Profiler> instance;
    std::vector<std::unique_ptr<std::vector<FunctionProfileData>>> profileData;
    std::mutex mtx;
};

std::unique_ptr<Profiler> Profiler::instance;

void Profiler::Init()
{
    instance.reset(new Profiler());
}

void Profiler::Done()
{
    instance.reset();
}

std::vector<FunctionProfileData>* Profiler::CreateFunctionProfileData()
{
    std::lock_guard<std::mutex> lock(mtx);
    profileData.push_back(std::unique_ptr<std::vector<FunctionProfileData>>(new std::vector<FunctionProfileData>));
    return profileData.back().get();
}

void Profiler::WriteData()
{
    std::string executablePath = GetFullPath(GetPathToExecutable());
    std::string profileDataFilePath = Path::Combine(Path::GetDirectoryName(executablePath), "cmprof.bin");
    BinaryWriter writer(profileDataFilePath);
    uint64_t n = 0;
    for (const std::unique_ptr<std::vector<FunctionProfileData>>& profileDataVec : profileData)
    {
        n += profileDataVec->size();
    }
    writer.Write(n);
    for (const std::unique_ptr<std::vector<FunctionProfileData>>& profileDataVec : profileData)
    {
        for (const FunctionProfileData& functionProfileData : *profileDataVec)
        {
            writer.Write(functionProfileData.functionId);
            writer.Write(functionProfileData.timePoint.time_since_epoch().count());
            writer.Write(static_cast<uint8_t>(functionProfileData.kind));
        }
    }
}

#ifdef _WIN32
__declspec(thread) std::vector<FunctionProfileData>* functionProfileData = nullptr;
#else
__thread std::vector<FunctionProfileData>* functionProfileData = nullptr;
#endif

void Profiler::StartFunction(uint32_t functionId)
{
    if (!functionProfileData)
    {
        functionProfileData = CreateFunctionProfileData();
    }
    functionProfileData->push_back(FunctionProfileData(TimePointKind::start, functionId));
}

void Profiler::EndFunction(uint32_t functionId)
{
    if (!functionProfileData)
    {
        functionProfileData = CreateFunctionProfileData();
    }
    functionProfileData->push_back(FunctionProfileData(TimePointKind::end, functionId));
}

void InitProfiler()
{
    Profiler::Init();
}

void DoneProfiler()
{
    Profiler::Instance().WriteData();
    Profiler::Done();
}

} }  // namespace cmajor::rt

extern "C" RT_API void RtStartProfiling()
{
    cmajor::rt::InitProfiler();
    RtInit();
}

extern "C" RT_API void RtEndProfiling()
{
    RtDone();
    cmajor::rt::DoneProfiler();
}

extern "C" RT_API void RtProfileStartFunction(uint32_t functionId)
{
    cmajor::rt::Profiler::Instance().StartFunction(functionId);
}

extern "C" RT_API void RtProfileEndFunction(uint32_t functionId)
{
    cmajor::rt::Profiler::Instance().EndFunction(functionId);
}