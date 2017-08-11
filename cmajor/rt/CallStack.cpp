// =================================
// Copyright (c) 2017 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/CallStack.hpp>
#include <cmajor/util/Error.hpp>
#include <vector>

namespace cmajor { namespace rt {

struct FunctionInfo
{
    FunctionInfo(const char* functionName_, const char* sourceFilePath_);
    const char* functionName;
    const char* sourceFilePath;
    int32_t lineNumber;
};

FunctionInfo::FunctionInfo(const char* functionName_, const char* sourceFilePath_) : functionName(functionName_), sourceFilePath(sourceFilePath_), lineNumber(0)
{
}

class CallStack
{
public:
    CallStack();
    std::vector<FunctionInfo>& Calls() { return calls; }
private:
    std::vector<FunctionInfo> calls;
};

CallStack::CallStack()
{
}

#ifdef _WIN32

__declspec(thread) CallStack* callStack = nullptr;

#else

__thread CallStack* callStack = nullptr;

#endif

} }  // namespace cmajor::rt

extern "C" RT_API void RtEnterFunction(const char* functionName, const char* sourceFilePath)
{
    cmajor::rt::CallStack* callStack = cmajor::rt::callStack;
    if (!callStack)
    {
        callStack = new cmajor::rt::CallStack();
        cmajor::rt::callStack = callStack;
    }
    callStack->Calls().push_back(cmajor::rt::FunctionInfo(functionName, sourceFilePath));
}

extern "C" RT_API void RtSetLineNumber(int32_t lineNumber)
{
    try
    {
        cmajor::rt::CallStack* callStack = cmajor::rt::callStack;
        Assert(callStack && !callStack->Calls().empty(), "call stack is empty");
        callStack->Calls().back().lineNumber = lineNumber;
    }
    catch (const std::exception& ex)
    {
        int x = 0;
        // todo
    }
}

extern "C" RT_API void RtExitFunction()
{
    try
    {
        cmajor::rt::CallStack* callStack = cmajor::rt::callStack;
        Assert(callStack && !callStack->Calls().empty(), "call stack is empty");
        callStack->Calls().pop_back();
    }
    catch (const std::exception& ex)
    {
        int x = 0;
        // todo
    }
}
