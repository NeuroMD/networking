#include <algorithm>
#include <cassert>
#include "win_socket.h"

#ifdef WIN_OS

namespace Net {

static void addrinfo_empty_deleter(PADDRINFOA){

}

static decltype(&freeaddrinfo) deleter_ptr = reinterpret_cast<decltype(&freeaddrinfo)>(addrinfo_empty_deleter);

bool init_sockets() noexcept {
    WSADATA wsaData;
    auto wVersionRequested = MAKEWORD(2, 2);
    auto error = WSAStartup(wVersionRequested, &wsaData);
    if (error != 0) {
        return false;
    }
    if (LOBYTE(wsaData.wVersion) != 2 || HIBYTE(wsaData.wVersion) != 2) {
        WSACleanup();
        return false;
    }
    return true;
}

void sockets_cleanup() noexcept {
    WSACleanup();
}

int get_last_error() noexcept {
    return WSAGetLastError();
}

socket_t get_default_socket() noexcept {
    return INVALID_SOCKET;
}

addr_info_ptr get_addr_info(SocketType socket_type, PortNumberType port, std::string address) noexcept {
    int protoType;
    int socketType;
    switch(socket_type){
    case SocketType::TCP:{
        protoType = IPPROTO_TCP;
        socketType = SOCK_STREAM;
        break;
    }
    case SocketType::UDP:{
        protoType = 0;
        socketType = SOCK_DGRAM;
        break;
    }
    default:
        return addr_info_ptr(nullptr, deleter_ptr);
    }
    addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = socketType;
    hints.ai_protocol = protoType;
    hints.ai_flags = AI_PASSIVE;

    auto portString = std::to_string(port);
    addrinfo *result = nullptr;
    auto errorCode = getaddrinfo(address.empty() ? nullptr : address.c_str(), portString.c_str(), &hints, &result);
    if (errorCode != 0) {
        return addr_info_ptr(nullptr, deleter_ptr);
    }
    auto res = addr_info_ptr(result, freeaddrinfo);
    return res;
}

socket_t create_socket(const addr_info_ptr &address_info) noexcept {
    assert(address_info);
    return socket(address_info->ai_family, address_info->ai_socktype, address_info->ai_protocol);
}

bool socket_valid(const socket_t &socket) noexcept {
    return socket != INVALID_SOCKET;
}

bool socket_bind(socket_t &socket, const addr_info_ptr &addres_info) noexcept {
    assert(socket != INVALID_SOCKET);
    if (bind(socket, addres_info->ai_addr, static_cast<int>(addres_info->ai_addrlen)) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

bool socket_listen(socket_t &socket) noexcept {
    assert(socket != INVALID_SOCKET);
    if (listen(socket, SOMAXCONN) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

socket_t socket_accept(socket_t &socket) noexcept {
    return accept(socket, nullptr, nullptr);
}

bool socket_connect(socket_t &socket, const addr_info_ptr &addr_info) noexcept {
    if (connect(socket, addr_info.get()->ai_addr, static_cast<int>(addr_info.get()->ai_addrlen)) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

ByteBuffer socket_receive(socket_t &socket){
    assert(socket != INVALID_SOCKET);
    char recvbuf[RECEIVE_BUFFER_LEN];
    auto messageLength = recv(socket, recvbuf, RECEIVE_BUFFER_LEN, 0);
    if (messageLength == 0)
        return ByteBuffer();
    if (messageLength<0){
        auto err = get_last_error();
        if (err == WSAECONNRESET)
            return ByteBuffer();
        throw std::runtime_error(std::string("Receive error code: ")+std::to_string(err));
    }


    ByteBuffer message(messageLength);
    auto byteBuffer = reinterpret_cast<Byte *>(recvbuf);
    std::copy(byteBuffer, byteBuffer+messageLength, message.begin());

    return message;
}

ByteBuffer socket_receive_from(socket_t &socket, addr_info_ptr &addr_info){
    assert(socket != INVALID_SOCKET);
    char recvbuf[RECEIVE_BUFFER_LEN];
    int len = sizeof(SOCKADDR);
    auto messageLength = recvfrom(socket, recvbuf, RECEIVE_BUFFER_LEN, 0, addr_info.get()->ai_addr, &len);
    addr_info.get()->ai_addrlen = len;
    if (messageLength == 0)
        return ByteBuffer();
    if (messageLength<0){
        auto err = get_last_error();
        if (err == WSAECONNRESET)
            return ByteBuffer();
        throw std::runtime_error(std::string("Receive form error code: ")+std::to_string(err));
    }

    ByteBuffer message(messageLength);
    auto byteBuffer = reinterpret_cast<Byte *>(recvbuf);
    std::copy(byteBuffer, byteBuffer+messageLength, message.begin());

    return message;
}

bool socket_send(socket_t &socket, const ByteBuffer &message) noexcept {
    assert(socket != INVALID_SOCKET);
    auto sendBuffer = reinterpret_cast<const char *>(message.data());
    auto bufferLength = static_cast<int>(message.size());
    if (send(socket, sendBuffer, bufferLength, 0) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

bool socket_send_to(socket_t &socket, const addr_info_ptr &addr_info, const ByteBuffer &message) noexcept {
    assert(socket != INVALID_SOCKET);
    auto sendBuffer = reinterpret_cast<const char *>(message.data());
    auto bufferLength = static_cast<int>(message.size());
    if (sendto(socket, sendBuffer, bufferLength, 0, addr_info.get()->ai_addr, static_cast<int>(addr_info.get()->ai_addrlen)) == SOCKET_ERROR) {
        return false;
    }
    return true;
}

bool socket_shutdown(socket_t &socket) noexcept {
    assert(socket != INVALID_SOCKET);
    if (shutdown(socket, SD_BOTH) == SOCKET_ERROR)
        return false;
    return true;
}

void socket_close(socket_t &socket) noexcept {
    closesocket(socket);
}

}

#endif
