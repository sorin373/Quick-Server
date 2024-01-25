#include "server_in.hpp"

#include <malloc.h>
#include <sys/socket.h>
#include <cstring>
#include <arpa/inet.h>

using namespace net;

int Socket::createSocketTCP(void) __THROW
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

struct sockaddr_in *Socket::createIPv4SocketAddress(const char *__IPv4, const int __port)
{
    struct sockaddr_in *__address = (struct sockaddr_in *)malloc(sizeof(struct sockaddr_in));

    if (__address != NULL)
    {
        __address->sin_family = AF_INET;
        __address->sin_port = htons(__port);

        if (strlen(__IPv4) <= 0 || strcasecmp(__IPv4, "INADDR_ANY") == 0 || __IPv4 == nullptr)
            __address->sin_addr.s_addr = INADDR_ANY;
        else
            inet_pton(AF_INET, __IPv4, &__address->sin_addr.s_addr);
    }

    return __address;
}

int Socket::connectToServer(int __socketFileDescriptor, struct sockaddr_in *__address) __THROW
{
    return connect(__socketFileDescriptor, (struct sockaddr *)__address, sizeof(struct sockaddr));
}