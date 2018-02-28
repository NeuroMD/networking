#include "socket.h"

namespace Net
{

BaseSocket::~BaseSocket(){
    socket_close(this->mSocket);
}

BaseSocket::BaseSocket():mSocket(get_default_socket()){

}

BaseSocket::BaseSocket(socket_t &&soc):
    mSocket(std::move(soc)){}

}
