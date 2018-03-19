#include "socket.h"
#include <system_error>
#include <cassert>
#include <iostream>

namespace Net
{

TcpServerSocket::TcpServerSocket(PortNumberType port){
    auto addresInfo = get_addr_info(SocketType::TCP, port);
    if(!addresInfo)
        throw std::runtime_error("Unable to create address info");

    this->mSocket = create_socket(addresInfo);
    if (!socket_valid(this->mSocket))
        throw std::runtime_error("Unable to create socket");


    if (!socket_bind(this->mSocket, addresInfo)){
        socket_close(mSocket);
        throw std::runtime_error("Unable to bind socket");
    }
}

TcpServerSocket::~TcpServerSocket(){
    try    {
        if (listening){
            isAccepting.store(false);
            socket_shutdown(mSocket);
            if (acceptLoop.joinable())
                acceptLoop.join();
        }
    }
    catch (std::system_error &)    {
        assert(false);
    }
    socket_close(mSocket);
}

void TcpServerSocket::startListen(){
    if (listening)
        return;

    if(!socket_listen(this->mSocket))
        throw std::runtime_error("Failed start listen on socket");

    listening = true;
    startAcceptLoop();
}

void TcpServerSocket::setClientConnectedCallback(std::function<void(std::unique_ptr<TcpClientSocket>)> callback){
    clientConnectedCallback = callback;
}

void TcpServerSocket::startAcceptLoop(){
    acceptLoop = std::thread([=](){
        while(isAccepting.load()){
            auto client = socket_accept(mSocket);
            if (socket_valid(client)){
                try{
                    std::unique_ptr<TcpClientSocket> acceptedClient(new TcpClientSocket(std::move(client)));
                    if (clientConnectedCallback) clientConnectedCallback(std::move(acceptedClient));
                }
                catch (std::runtime_error &e){
                    std::cerr<< "Failed to create client socket: " << e.what() <<std::endl;
                    assert(false);
                }
            }
        }
    });
}

TcpClientSocket::TcpClientSocket(socket_t &&soc):
    BaseSocket(std::move(soc)),
    mAddressInfo(get_addr_info(SocketType::TCP, 0)){
    mConnected = true;
    startReceiveLoop();
}

TcpClientSocket::TcpClientSocket(std::string address, PortNumberType port):
    mAddressInfo(get_addr_info(SocketType::TCP, port, address)){

    if(!mAddressInfo)
        throw std::runtime_error("Unable to create address info");

    mSocket = create_socket(mAddressInfo);
    if (!socket_valid(mSocket))
        throw std::runtime_error("Unable to create socket");

    startReceiveLoop();
}

TcpClientSocket::~TcpClientSocket(){
    try{
        isReceiving.store(false);
        socket_shutdown(mSocket);
        if (receiveThread.joinable()){
            try{
                receiveThread.join();
            }
            catch(std::system_error &e){
                e.what();
            }
        }
    }
    catch (std::system_error &)    {
        assert(false);
    }
    socket_close(mSocket);
}

bool TcpClientSocket::connectRemote(){
    if (!mAddressInfo)
        throw std::runtime_error("Socket is not connactable");

    if (!socket_valid(mSocket))
        throw std::runtime_error("Socket is in invalid state");

    return mConnected = socket_connect(this->mSocket, mAddressInfo);
}

void TcpClientSocket::send(const ByteBuffer &data){
    if (!socket_valid(mSocket))
        throw std::runtime_error("Socket is in invalid state");
    if (!socket_send(this->mSocket, data))
        throw std::runtime_error(std::string("Send error. Error code: ") + std::to_string(get_last_error()));
}

void TcpClientSocket::setDataReceivedCallback(std::function<void (ByteBuffer)> callback){
    dataReceivedCallback = callback;
}

void TcpClientSocket::setDisconnectedCallback(std::function<void ()> callback){
    disconnectedCallback = callback;
}

void TcpClientSocket::startReceiveLoop(){
    receiveThread = std::thread([=](){
        while(isReceiving.load()){
            if (!socket_valid(mSocket)){
                if (disconnectedCallback) disconnectedCallback();
                return;
            }
            if (!mConnected) {
                std::this_thread::sleep_for(std::chrono::microseconds(1));
                continue;
            }
            try{
                auto data = socket_receive(mSocket);
                if(data.size()==0){
                    if (disconnectedCallback) disconnectedCallback();
                    return;
                }
                if (dataReceivedCallback) dataReceivedCallback(data);
            }
            catch (std::runtime_error &e){
                std::cerr<< "Failed to recive data: " << e.what() <<std::endl;
                assert(false);
            }
        }
    });
}

}
