#ifndef POSIX_SOCKET_H
#define POSIX_SOCKET_H

#include "../net_types.h"

#ifdef POSIX_OS

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

namespace net
{

bool init_sockets() noexcept;
void sockets_cleanup() noexcept;
bool shutdown();

}

#endif
#endif // POSIX_SOCKET_H
