#ifndef NET_TYPES_H
#define NET_TYPES_H

#if defined(__linux__) || defined(__ANDROID) || defined(__APPLE__)
#define POSIX_OS
#elif defined(_WIN64) || defined (_WIN32)
#define WIN_OS
#else
static_assert(false, "Unsupported platform");
#endif

#include <vector>
#include <string>

namespace Net
{

using PortNumberType = unsigned short;
using Byte = unsigned char;
using ByteBuffer = std::vector<Byte>;

inline ByteBuffer to_byte_buffer(std::string text){
    auto nullTerminatedSize = text.size()+1;
    ByteBuffer result(nullTerminatedSize);
    auto cStr = reinterpret_cast<const Byte *>(text.c_str());
    std::copy(cStr, cStr+nullTerminatedSize, result.begin());
    return result;
}

inline std::string convert_to_string(const ByteBuffer &buffer){
    auto stringArray = reinterpret_cast<const char *>(buffer.data());
    return std::string(stringArray);
}

constexpr std::size_t RECEIVE_BUFFER_LEN = 512;

enum class SocketSide{
    Server,
    Client
};

enum class SocketType: int {
    TCP = 0,
    UDP = 1
};

}

#endif // NET_TYPES_H
