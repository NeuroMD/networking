#ifndef POSIX_SOCKET_H
#define POSIX_SOCKET_H

#include "../net_types.h"

#ifdef POSIX_OS

#include <sys/socket.h>
#include <sys/types.h>
#include <netdb.h>
#include <netinet/in.h>
#include <memory>

namespace Net
{

    using socket_t = int;
    using addr_info_t = addrinfo;
    using addr_info_ptr = std::unique_ptr<addr_info_t, decltype(&freeaddrinfo)>;

    bool init_sockets() noexcept;
    void sockets_cleanup() noexcept;

    int get_last_error() noexcept;
    socket_t get_default_socket() noexcept;
    addr_info_ptr get_addr_info(SocketType, PortNumberType, std::string address = std::string()) noexcept;
    socket_t create_socket(const addr_info_ptr &) noexcept;
    bool socket_valid(const socket_t &) noexcept;
    bool socket_bind(socket_t &, const addr_info_ptr &) noexcept;
    bool socket_listen(socket_t &) noexcept;
    socket_t socket_accept(socket_t &) noexcept;
    bool socket_connect(socket_t &, const addr_info_ptr &) noexcept;
    ByteBuffer socket_receive(socket_t &);
    ByteBuffer socket_receive_from(socket_t &, addr_info_ptr &);
    bool socket_send(socket_t &, const ByteBuffer &) noexcept;
    bool socket_send_to(socket_t &, const addr_info_ptr &, const ByteBuffer &) noexcept;
    bool socket_shutdown(socket_t &) noexcept;
    void socket_close(socket_t &) noexcept;


}

#endif
#endif // POSIX_SOCKET_H
