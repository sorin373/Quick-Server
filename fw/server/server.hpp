#ifndef __SERVER_HPP__
#define __SERVER_HPP__

#include "server_in.hpp"

namespace net
{
    class Database
    {

    };

    template <typename T> class Server
    {
    public:
        static volatile bool SERVER_RUNNING;

        class AcceptedSocket
        {
        public:
            AcceptedSocket() = default;

            ~AcceptedSocket() = default;
        };

    private:
        class AcceptedSocket *acceptedSocket;

        /**
         * @brief This function handles client connections. It creates a new 'acceptedSocket' object for every incoming connection using the new operator.
         * @param acceptedSocketFileDescriptor The file descriptor for the accepted socket connection used when seding the HTTP response.
         */
        void handleClientConnections(int serverSocketFileDescriptor);

    public:
        Server();

        void __SERVER_INIT__(const int serverSocketFD);

        /*
         * This function binds a socket to a specific address and port. Return 0 for success, -1 for errors:
         *   - Default address: 127.0.0.1
         *   - Default port:    8080
         */
        int bindServer(const int __serverSocketFileDescriptor, const struct sockaddr_in *__serverAddress);

        ~Server();
    };

    int quickServerEasyInit(void);
};

#endif