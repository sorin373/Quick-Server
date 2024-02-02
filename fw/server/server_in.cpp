#include "server_in.hpp"

#include <iostream>
#include <iomanip>
#include <cstring>
#include <unistd.h>
#include <malloc.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define __BUFFER__   128
#define __IFCONFIG__ "ifconfig"

using namespace net;

char *Socket::exec(const char *__prompt)
{
    FILE *pipe = popen(__prompt, "r");

    if (!pipe)
    {
        std::cerr << std::setw(5) << " "
                  << "Failed to execute prompt: " << __prompt << "\n";

        return nullptr;
    }

    char buffer[__BUFFER__];
    char *res = new char[4096];

    size_t totalSize = 0;

    while (!feof(pipe))
        if (fgets(buffer, __BUFFER__, pipe) != nullptr)
        {
            size_t size = strlen(buffer);
            strncpy(res + totalSize, buffer, size);
            totalSize += size;
        }

    res[strlen(res)] = NULL;

    pclose(pipe);

    return res;
}

char *Socket::getIPv4(void)
{
    char *res = exec(__IFCONFIG__);

    if (!res)
        return nullptr;

    char *pos = strstr(res, "inet ");

    if (pos)
    {
        pos = strstr(pos + 1, "inet ");

        if (pos)
        {
            pos += 5;

            char *end = strchr(pos, ' ');

            if (end)
            {
                size_t len = end - pos;
                char *address = new char[len + 1];
                strncpy(address, pos, len);

                delete[] res;

                return address;
            }
        }
    }

    delete[] res;

    return nullptr;
}

int Socket::createSocketTCP(void) __THROW
{
    return socket(AF_INET, SOCK_STREAM, 0);
}

int Socket::connectToServer(int __socketFileDescriptor, struct sockaddr_in *__address) __THROW
{
    return connect(__socketFileDescriptor, (struct sockaddr *)__address, sizeof(struct sockaddr));
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

void Socket::closeSocket(const int __socketFileDescriptor)
{
    close(__socketFileDescriptor);
}