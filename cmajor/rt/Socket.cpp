// =================================
// Copyright (c) 2018 Seppo Laakko
// Distributed under the MIT license
// =================================

#include <cmajor/rt/Socket.hpp>
#include <cmajor/rt/Io.hpp>
#include <cmajor/rt/Error.hpp>
#include <cmajor/rt/InitDone.hpp>
#include <cmajor/util/Error.hpp>
#include <memory>
#include <vector>
#include <atomic>
#include <mutex>
#include <unordered_map>
#include <winsock2.h>
#include <ws2tcpip.h>    
#include <Windows.h>

namespace cmajor { namespace rt {

class SocketTable
{
public:
    static void Init();
    static void Done();
    static SocketTable& Instance() { Assert(instance, "socket table not initialized"); return *instance; }
    ~SocketTable();
    int32_t CreateSocket();
    int32_t BindSocket(int32_t socketHandle, int32_t port);
    int32_t ListenSocket(int32_t socketHandle, int32_t backlog);
    int32_t AcceptSocket(int32_t socketHandle);
    int32_t CloseSocket(int32_t socketHandle);
    int32_t ShutdownSocket(int32_t socketHandle, ShutdownMode mode);
    int32_t ConnectSocket(const std::string& node, const std::string& service);
    int32_t SendSocket(int32_t socketHandle, uint8_t* buf, int32_t len, int32_t flags);
    int32_t ReceiveSocket(int32_t socketHandle, uint8_t* buf, int32_t len, int32_t flags);
private:
    static std::unique_ptr<SocketTable> instance;
    const int32_t maxNoLockSocketHandles = 256;
    std::vector<SOCKET> sockets;
    std::unordered_map<int32_t, SOCKET> socketMap;
    std::atomic<int32_t> nextSocketHandle;
    std::mutex mtx;
    SocketTable();
};

std::unique_ptr<SocketTable> SocketTable::instance;

void SocketTable::Init()
{
    instance.reset(new SocketTable());
}

void SocketTable::Done()
{
    instance.reset();
}

std::string GetSocketErrorMessage(int errorCode)
{
    char buf[1024];
    FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, errorCode, 0, buf, sizeof(buf), NULL);
    return std::string(buf);
}

int GetLastSocketError()
{
    return WSAGetLastError();
}

SocketTable::SocketTable() : nextSocketHandle(1)
{
    sockets.resize(maxNoLockSocketHandles);
    WORD ver = MAKEWORD(2, 2);
    WSADATA wsaData;
    if (WSAStartup(ver, &wsaData) != 0)
    {
        int errorCode = GetLastSocketError();
        std::string errorMessage = "socket initialization failed with error code " + std::to_string(errorCode) + ": " + GetSocketErrorMessage(errorCode);
        RtWrite(2, (const uint8_t*)errorMessage.c_str(), errorMessage.length());
        RtExit(exitCodeSocketInitializationFailed);
    }
}

SocketTable::~SocketTable()
{
    WSACleanup();
}

int32_t SocketTable::CreateSocket()
{
    SOCKET s = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (s == INVALID_SOCKET)
    {
        int errorCode = GetLastSocketError();
        std::string errorMessage = GetSocketErrorMessage(errorCode);
        return InstallError(errorMessage);
    }
    int32_t socketHandle = nextSocketHandle++;
    if (socketHandle < maxNoLockSocketHandles)
    {
        sockets[socketHandle] = s;
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        socketMap[socketHandle] = s;
    }
    return socketHandle;
}

int32_t SocketTable::BindSocket(int32_t socketHandle, int32_t port)
{
    int result = 0;
    if (socketHandle <= 0)
    {
        return InstallError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        struct sockaddr_in addr;
        addr.sin_family = AF_INET;
        addr.sin_addr.s_addr = inet_addr("127.0.0.1");
        addr.sin_port = htons(port);
        result = bind(s, (struct sockaddr*) &addr, sizeof(addr));
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            struct sockaddr_in addr;
            addr.sin_family = AF_INET;
            addr.sin_addr.s_addr = inet_addr("127.0.0.1");
            addr.sin_port = htons(port);
            result = bind(s, (struct sockaddr*) &addr, sizeof(addr));
        }
        else
        {
            return InstallError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result != 0)
    {
        int errorCode = GetLastSocketError();
        std::string errorMessage = GetSocketErrorMessage(errorCode);
        return InstallError(errorMessage);
    }
    return 0;
}

int32_t SocketTable::ListenSocket(int32_t socketHandle, int32_t backlog)
{
    int result = 0;
    if (socketHandle <= 0)
    {
        return InstallError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        result = listen(s, backlog);
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            result = listen(s, backlog);
        }
        else
        {
            return InstallError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result != 0)
    {
        int errorCode = GetLastSocketError();
        std::string errorMessage = GetSocketErrorMessage(errorCode);
        return InstallError(errorMessage);
    }
    return 0;
}

int32_t SocketTable::AcceptSocket(int32_t socketHandle)
{
    SOCKET a = 0;
    if (socketHandle <= 0)
    {
        return InstallError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        a = accept(s, NULL, NULL);
        if (a == INVALID_SOCKET)
        {
            int errorCode = GetLastSocketError();
            std::string errorMessage = GetSocketErrorMessage(errorCode);
            return InstallError(errorMessage);
        }
        int32_t acceptedSocketHandle = nextSocketHandle++;
        if (acceptedSocketHandle < maxNoLockSocketHandles)
        {
            sockets[acceptedSocketHandle] = a;
        }
        else
        {
            std::lock_guard<std::mutex> lock(mtx);
            socketMap[acceptedSocketHandle] = a;
        }
        return acceptedSocketHandle;
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            a = accept(s, NULL, NULL);
            if (a == INVALID_SOCKET)
            {
                int errorCode = GetLastSocketError();
                std::string errorMessage = GetSocketErrorMessage(errorCode);
                return InstallError(errorMessage);
            }
            int32_t acceptedSocketHandle = nextSocketHandle++;
            if (acceptedSocketHandle < maxNoLockSocketHandles)
            {
                sockets[acceptedSocketHandle] = a;
            }
            else
            {
                socketMap[acceptedSocketHandle] = a;
            }
            return acceptedSocketHandle;
        }
        else
        {
            return InstallError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
}

int32_t SocketTable::CloseSocket(int32_t socketHandle)
{
    int result = 0;
    if (socketHandle <= 0)
    {
        return InstallError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        result = closesocket(s);
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            result = closesocket(s);
        }
        else
        {
            return InstallError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result != 0)
    {
        int errorCode = GetLastSocketError();
        std::string errorMessage = GetSocketErrorMessage(errorCode);
        return InstallError(errorMessage);
    }
    return 0;
}

int32_t SocketTable::ShutdownSocket(int32_t socketHandle, ShutdownMode mode)
{
    int result = 0;
    int how = SD_RECEIVE;
    switch (mode)
    {
        case ShutdownMode::receive: how = SD_RECEIVE; break;
        case ShutdownMode::send: how = SD_SEND; break;
        case ShutdownMode::both: how = SD_BOTH; break;
    }
    if (socketHandle <= 0)
    {
        return InstallError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        result = shutdown(s, how);
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            result = shutdown(s, how);
        }
        else
        {
            return InstallError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result != 0)
    {
        int errorCode = GetLastSocketError();
        std::string errorMessage = GetSocketErrorMessage(errorCode);
        return InstallError(errorMessage);
    }
    return 0;
}

int32_t SocketTable::ConnectSocket(const std::string& node, const std::string& service)
{
    struct addrinfo hint;
    struct addrinfo* rp;
    struct addrinfo* res;
    memset(&hint, 0, sizeof(struct addrinfo));
    hint.ai_flags = 0;
    hint.ai_family = AF_INET;
    hint.ai_socktype = SOCK_STREAM;
    hint.ai_protocol = IPPROTO_TCP;
    hint.ai_addrlen = 0;
    hint.ai_addr = 0;
    hint.ai_canonname = 0;
    hint.ai_next = 0;
    int result = getaddrinfo(node.c_str(), service.c_str(), &hint, &res);
    if (result != 0)
    {
        int errorCode = GetLastSocketError();
        std::string errorMessage = GetSocketErrorMessage(errorCode);
        return InstallError(errorMessage);
    }
    else
    {
        for (rp = res; rp != 0; rp = rp->ai_next)
        {
            SOCKET s = socket(rp->ai_family, rp->ai_socktype, rp->ai_protocol);
            if (s == -1)
            {
                continue;
            }
            int result = connect(s, rp->ai_addr, (int)rp->ai_addrlen);
            if (result == 0)
            {
                freeaddrinfo(res);
                int32_t connectedSocketHandle = nextSocketHandle++;
                if (connectedSocketHandle < maxNoLockSocketHandles)
                {
                    sockets[connectedSocketHandle] = s;
                }
                else
                {
                    std::lock_guard<std::mutex> lock(mtx);
                    socketMap[connectedSocketHandle] = s;
                }
                return connectedSocketHandle;
            }
            else
            {
                freeaddrinfo(res);
                int errorCode = GetLastSocketError();
                std::string errorMessage = GetSocketErrorMessage(errorCode);
                return InstallError(errorMessage);
            }
        }
    }
    std::string errorMessage = "could not connect";
    return InstallError(errorMessage);
}

int32_t SocketTable::SendSocket(int32_t socketHandle, uint8_t* buf, int32_t len, int32_t flags)
{
    int32_t result = 0;
    if (socketHandle <= 0)
    {
        return InstallError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        result = send(s, (const char*)buf, len, flags);
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            result = send(s, (const char*)buf, len, flags);
        }
        else
        {
            return InstallError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result < 0)
    {
        int errorCode = GetLastSocketError();
        std::string errorMessage = GetSocketErrorMessage(errorCode);
        return InstallError(errorMessage);
    }
    return result;
}

int32_t SocketTable::ReceiveSocket(int32_t socketHandle, uint8_t* buf, int32_t len, int32_t flags)
{
    int32_t result = 0;
    if (socketHandle <= 0)
    {
        return InstallError("invalid socket handle " + std::to_string(socketHandle));
    }
    else if (socketHandle < maxNoLockSocketHandles)
    {
        SOCKET s = sockets[socketHandle];
        result = recv(s, (char*)buf, len, flags);
    }
    else
    {
        std::lock_guard<std::mutex> lock(mtx);
        auto it = socketMap.find(socketHandle);
        if (it != socketMap.cend())
        {
            SOCKET s = it->second;
            result = recv(s, (char*)buf, len, flags);
        }
        else
        {
            return InstallError("invalid socket handle " + std::to_string(socketHandle));
        }
    }
    if (result < 0)
    {
        int errorCode = GetLastSocketError();
        std::string errorMessage = GetSocketErrorMessage(errorCode);
        return InstallError(errorMessage);
    }
    return result;
}

void InitSocket()
{
    SocketTable::Init();
}

void DoneSocket()
{
    SocketTable::Done();
}

} }  // namespace cmajor::rt

extern "C" RT_API int32_t RtCreateSocket()
{
    return cmajor::rt::SocketTable::Instance().CreateSocket();
}

extern "C" RT_API int32_t RtBindSocket(int32_t socketHandle, int32_t port)
{
    return cmajor::rt::SocketTable::Instance().BindSocket(socketHandle, port);
}

extern "C" RT_API int32_t RtListenSocket(int32_t socketHandle, int32_t backLog)
{
    return cmajor::rt::SocketTable::Instance().ListenSocket(socketHandle, backLog);
}

extern "C" RT_API int32_t RtAcceptSocket(int32_t socketHandle)
{
    return cmajor::rt::SocketTable::Instance().AcceptSocket(socketHandle);
}

extern "C" RT_API int32_t RtCloseSocket(int32_t socketHandle)
{
    return cmajor::rt::SocketTable::Instance().CloseSocket(socketHandle);
}

extern "C" RT_API int32_t RtShutdownSocket(int32_t socketHandle, ShutdownMode mode)
{
    return cmajor::rt::SocketTable::Instance().ShutdownSocket(socketHandle, mode);
}

extern "C" RT_API int32_t RtConnectSocket(const char* node, const char* service)
{
    return cmajor::rt::SocketTable::Instance().ConnectSocket(node, service);
}

extern "C" RT_API int32_t RtSendSocket(int32_t socketHandle, uint8_t* buf, int32_t len, int32_t flags)
{
    return cmajor::rt::SocketTable::Instance().SendSocket(socketHandle, buf, len, flags);
}

extern "C" RT_API int32_t RtReceiveSocket(int32_t socketHandle, uint8_t* buf, int32_t len, int32_t flags)
{
    return cmajor::rt::SocketTable::Instance().ReceiveSocket(socketHandle, buf, len, flags);
}
