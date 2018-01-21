// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#ifndef CMAJOR_RT_SOCKET_INCLUDED
#define CMAJOR_RT_SOCKET_INCLUDED
#include <cmajor/rt/RtApi.hpp>
#include <stdint.h>

enum class ShutdownMode : int32_t
{
    receive = 0, send = 1, both = 2
};

extern "C" RT_API int32_t RtCreateSocket();
extern "C" RT_API int32_t RtBindSocket(int32_t socketHandle, int32_t port);
extern "C" RT_API int32_t RtListenSocket(int32_t socketHandle, int32_t backLog);
extern "C" RT_API int32_t RtAcceptSocket(int32_t socketHandle);
extern "C" RT_API int32_t RtCloseSocket(int32_t socketHandle);
extern "C" RT_API int32_t RtShutdownSocket(int32_t socketHandle, ShutdownMode mode);
extern "C" RT_API int32_t RtConnectSocket(const char* node, const char* service);
extern "C" RT_API int32_t RtSendSocket(int32_t socketHandle, uint8_t* buf, int32_t len, int32_t flags);
extern "C" RT_API int32_t RtReceiveSocket(int32_t socketHandle, uint8_t* buf, int32_t len, int32_t flags);

namespace cmajor { namespace rt {

void InitSocket();
void DoneSocket();

} }  // namespace cmajor::rt

#endif // CMAJOR_RT_SOCKET_INCLUDED
