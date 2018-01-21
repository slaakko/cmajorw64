// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Time.hpp>
#include <chrono>
#include <thread>

extern "C" RT_API int64_t RtNow()
{
    return std::chrono::duration_cast<std::chrono::nanoseconds>(std::chrono::steady_clock::now().time_since_epoch()).count();
}

extern "C" RT_API void RtSleep(int64_t nanoseconds)
{
    std::chrono::nanoseconds duration{ nanoseconds };
    std::this_thread::sleep_for(duration);
}
