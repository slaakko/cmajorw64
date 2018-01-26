// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Random.hpp>
#define _CRT_RAND_S
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

namespace cmajor { namespace rt {

unsigned int get_random_seed_from_system()
{
    unsigned int seed = 0;
    errno_t retval = rand_s(&seed);
    if (retval != 0)
    {
        perror("get_random_seed_from_system() failed");
        exit(1);
    }
    return seed;
}

class MT
{
private:
    static const int32_t n = 624;
    static const int32_t m = 397;
    static const uint32_t matrixA = 0x9908b0dfu;
    static const uint32_t upperMask = 0x80000000u;
    static const uint32_t lowerMask = 0x7fffffffu;
public:
    MT();
    bool Initialized()
    {
        return initialized;
    }
    void InitWithRandomSeed()
    {
        uint32_t seed = get_random_seed_from_system();
        Init(seed);
    }
    void Init(uint32_t seed)
    {
        initialized = true;
        mt[0] = seed;
        for (mti = 1; mti < n; ++mti)
        {
            mt[mti] = 1812433253u * (mt[mti - 1] ^ (mt[mti - 1] >> 30u)) + static_cast<uint32_t>(mti);
        }
        mag[0] = 0u;
        mag[1] = matrixA;
    }
    uint32_t GenRand()
    {
        uint32_t y = 0u;
        if (mti >= n)
        {
            int32_t kk;
            for (kk = 0; kk < n - m; ++kk)
            {
                y = (mt[kk] & upperMask) | (mt[kk + 1] & lowerMask);
                mt[kk] = mt[kk + m] ^ (y >> 1u) ^ mag[static_cast<int32_t>(y & 0x01u)];
            }
            for (; kk < n - 1; ++kk)
            {
                y = (mt[kk] & upperMask) | (mt[kk + 1] & lowerMask);
                mt[kk] = mt[kk + (m - n)] ^ (y >> 1u) ^ mag[static_cast<int32_t>(y & 0x01u)];
            }
            y = (mt[n - 1] & upperMask) | (mt[0] & lowerMask);
            mt[n - 1] = mt[m - 1] ^ (y >> 1u) ^ mag[static_cast<int32_t>(y & 0x01u)];
            mti = 0;
        }
        y = mt[mti++];
        y = y ^ (y >> 11u);
        y = y ^ ((y << 7u) & 0x9d2c5680u);
        y = y ^ ((y << 15u) & 0xefc60000u);
        y = y ^ (y >> 18u);
        return y;
    }
private:
    bool initialized;
    int32_t mti;
    uint32_t mt[n];
    uint32_t mag[2];
};

MT::MT() : initialized(false), mti(0), mt(), mag()
{
}

__declspec(thread) MT* mt = nullptr;

void InitMt(uint32_t seed)
{
    if (mt == nullptr)
    {
        mt = new MT();
    }
    mt->Init(seed);
}

uint32_t Random()
{
    if (mt == nullptr)
    {
        mt = new MT();
    }
    if (!mt->Initialized())
    {
        mt->InitWithRandomSeed();
    }
    return mt->GenRand();
}

uint64_t Random64()
{
    return static_cast<uint64_t>(Random()) << 32 | static_cast<uint64_t>(Random());
}

} } // namespace cmajor::rt

extern "C" RT_API void RtInitRand(uint32_t seed)
{
    cmajor::rt::InitMt(seed);
}

extern "C" RT_API uint32_t RtRandom()
{
    return cmajor::rt::Random();
}

extern "C" RT_API uint64_t RtRandom64()
{
    return cmajor::rt::Random64();
}