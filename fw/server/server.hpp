#ifndef __SERVER_HPP__
#define __SERVER_HPP__

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

    public:
        Server();

        int easyInit(void);

        ~Server();
    };
};

#endif