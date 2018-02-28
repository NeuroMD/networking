#include "socket.h"
#include <cassert>
#include <system_error>
#include <iostream>

namespace Net
{

UdpSocket::UdpSocket(PortNumberType port){
    auto addresInfo = get_addr_info(SocketType::UDP, port);
    if(!addresInfo)
        throw std::runtime_error("Unable to create address info");

    this->mSocket = create_socket(addresInfo);
    if (!this->mSocket)
        throw std::runtime_error("Unable to create socket");

    if (!socket_bind(this->mSocket, addresInfo)){        
        socket_close(mSocket);
        throw std::runtime_error("Unable to bind socket");
    }

    startReceiveLoop();
}

UdpSocket::~UdpSocket(){
    try{
    isReceiving.store(false);
    socket_shutdown(mSocket);
    if (receiveThread.joinable())
        receiveThread.join();
    }
    catch (std::system_error &)    {
        assert(false);
    }    
    socket_close(mSocket);
}

bool UdpSocket::sendTo(std::string address, PortNumberType port, const ByteBuffer &data){
    auto addressInfo = get_addr_info(SocketType::UDP, port, address);
    if (!addressInfo){
        assert(false);
        return false;
    }
    return socket_send_to(mSocket, addressInfo, data);
}

void UdpSocket::setDataReceivedCallback(std::function<void (ByteBuffer, std::string, PortNumberType)> callback){
    dataReceivedCallback = callback;
}

void UdpSocket::startReceiveLoop(){
    receiveThread = std::thread([=](){
        while(isReceiving.load()){
            if (!socket_valid(mSocket))
                return;
            auto addrInfo = get_addr_info(SocketType::UDP,0);
            try{
                auto data = socket_receive_from(mSocket, addrInfo);

                if(data.size()==0)
                    return;
                if (dataReceivedCallback) dataReceivedCallback(data, "", 123);
            }
            catch (std::runtime_error &e){
                std::cerr<< "Failed to recive data: " << e.what() <<std::endl;
                assert(false);
            }
        }
    });
}

}
