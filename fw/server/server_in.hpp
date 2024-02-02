#ifndef __SERVER_IN_HPP__
#define __SERVER_IN_HPP__

#include <netinet/in.h>

namespace net
{
    constexpr char localHostAddress[] = "127.0.0.1";
    constexpr int DEFAULT_PORT = 8080;

    class Socket
    {
    private:
        char *exec(const char *__prompt);
        
    public:
        Socket() = default;

        char *getIPv4(void);

        int createSocketTCP(void) __THROW;

        int connectToServer(int __socketFileDescriptor, struct sockaddr_in *__address) __THROW;

        struct sockaddr_in *createIPv4SocketAddress(const char *__IPv4 = localHostAddress, const int __port = DEFAULT_PORT);

        void closeSocket(const int __socketFileDescriptor);

        ~Socket() = default;
    };
};

#endif