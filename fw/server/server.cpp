#include "server_in.hpp"
#include "server.hpp"
#include "global.hpp"

#include <iostream>
#include <iomanip>
#include <thread>

using namespace net;

int quickServerEasyInit(void)
{
    Socket socket;

    int serverSocketFD = socket.createSocketTCP();

    if (serverSocketFD == -1)
    {
        std::cerr << std::setw(5) << " "
                  << "--> Error: Socket creation failed!\n";

        return EXIT_FAILURE;
    }

    server = new Server<char>();

    char *IPv4 = socket.getIPv4();

    if (!IPv4)
    {
        std::cerr << std::setw(5) << " "
                  << "--> Error: Null pointer encountered for machine's IPv4 address.\n";

        socket.closeSocket(serverSocketFD);

        shutdown(serverSocketFD, SHUT_RDWR);

        delete server;

        return EXIT_FAILURE;
    }

    struct sockaddr_in *serverAddress = socket.createIPv4SocketAddress(IPv4, 8080);

    if (server->bindServer(serverSocketFD, serverAddress) != 0)
    {
        std::cerr << "\n"
                  << std::setw(5) << " "
                  << "--> Server encountered an error during the binding process.\n"
                  << std::setw(5) << " "
                  << "--> ";

        perror("Bind");

        std::cout << "\n";

        socket.closeSocket(serverSocketFD);
        shutdown(serverSocketFD, SHUT_RDWR);
        free(serverAddress);
        delete server;
        delete[] IPv4;

        return EXIT_FAILURE;
    }

    if (listen(serverSocketFD, 10) == -1)
    {
        std::cerr << std::setw(5) << " "
                  << "--> Error: Failed to initiate listening on the server socket.\n";

        socket.closeSocket(serverSocketFD);
        shutdown(serverSocketFD, SHUT_RDWR);
        free(serverAddress);
        delete server;

        return EXIT_FAILURE;
    }
}

template <typename T>
void Server<T>::handleClientConnections(int serverSocketFileDescriptor)
{
    while (Server::SERVER_RUNNING)
    {
        //server::acceptedSocket *newAcceptedSocket = new server::acceptedSocket();

        //acceptConnection(serverSocketFileDescriptor, newAcceptedSocket);

        //connectedSockets.push_back(*newAcceptedSocket);

        //receivedDataHandlerThread(newAcceptedSocket);
    }
}

template <typename T>
void Server<T>::__SERVER_INIT__(const int serverSocketFD)
{
    SERVER_RUNNING = true;

    std::thread workerThread(&Server::handleClientConnections, this, serverSocketFD);
    workerThread.detach();
}

template <typename T>
int Server<T>::bindServer(const int __serverSocketFileDescriptor, const struct sockaddr_in *__serverAddress)
{
    return bind(__serverSocketFileDescriptor, (struct sockaddr *)__serverAddress, sizeof(struct sockaddr_in));
}