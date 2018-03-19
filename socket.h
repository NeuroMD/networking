#ifndef BASE_SOCKET_H
#define BASE_SOCKET_H

#include "net_types.h"
#include <functional>
#include <atomic>
#include <thread>

#if defined(WIN_OS)
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include "windows/win_socket.h"
#elif  defined(POSIX_OS)
#include "posix/posix_socket.h"
#endif

namespace Net
{

class BaseSocket{
public:
    BaseSocket(const BaseSocket &) = delete;
    BaseSocket& operator=(const BaseSocket &) = delete;
    virtual ~BaseSocket();

protected:
    socket_t mSocket;

    BaseSocket();
    BaseSocket(socket_t &&);
};

class TcpClientSocket : public BaseSocket{
public:
    TcpClientSocket(socket_t &&);
    TcpClientSocket(std::string address, PortNumberType port);
    ~TcpClientSocket();
    bool connectRemote();
    void send(const ByteBuffer &);
    void setDataReceivedCallback(std::function<void(ByteBuffer)>);
    void setDisconnectedCallback(std::function<void()>);

private:
    bool mConnected{false};
    std::function<void(ByteBuffer)> dataReceivedCallback;
    std::function<void()> disconnectedCallback;
    addr_info_ptr mAddressInfo;
    std::atomic<bool> isReceiving{true};
    std::thread receiveThread;

    void startReceiveLoop();
};

class TcpServerSocket : public BaseSocket{
public:
    TcpServerSocket(PortNumberType port);
    ~TcpServerSocket();
    void startListen();
    void setClientConnectedCallback(std::function<void(std::unique_ptr<TcpClientSocket>)>);

private:
    std::function<void(std::unique_ptr<TcpClientSocket>)> clientConnectedCallback;
    bool listening{false};
    std::atomic<bool> isAccepting{true};
    std::thread acceptLoop;

    void startAcceptLoop();
};

class UdpSocket : public BaseSocket{
public:
    UdpSocket(PortNumberType port);
    ~UdpSocket();
    bool sendTo(std::string address, PortNumberType port, const ByteBuffer &);
    void setDataReceivedCallback(std::function<void(ByteBuffer, std::string, PortNumberType)>);

private:
    std::function<void(ByteBuffer, std::string, PortNumberType)> dataReceivedCallback;
    std::atomic<bool> isReceiving{true};
    std::thread receiveThread;

    void startReceiveLoop();
};

}

#endif // BASE_SOCKET_H

