#include "serverUtils.hpp"
#include "declarations.hpp"
#include "interface/interface.hpp"
// #include "database/database.hpp"

#include <iostream>
#include <iomanip>
#include <fstream>
#include <string.h>
#include <sstream>
#include <vector>
#include <thread>
#include <netinet/in.h>
#include <cppconn/resultset.h>
#include <cppconn/prepared_statement.h>

using namespace net;
using namespace net::interface;

template class server<char>;

template <typename T>
volatile bool server<T>::SERVER_RUNNING = false;

/* DB */

template <typename T>
server<T>::Database::~Database()
{
    this->con->close();
    delete this->con;
}

template <typename T>
sql::Connection *server<T>::Database::getCon(void) const noexcept
{
    return con;
}

template <typename T>
sql::Driver *server<T>::Database::getDriver(void) const noexcept
{
    return driver;
}

template <typename T>
std::vector<std::string> server<T>::Database::getTableNames(void) const
{
    if (this->con == nullptr || this->driver == nullptr)
        return std::vector<std::string>();

    std::string query = "SELECT table_name FROM information_schema.tables WHERE table_schema = " + std::string(this->database);

    std::vector<std::string> tableNames;

    sql::Statement *stmt = nullptr;
    sql::ResultSet *res = nullptr;

    stmt = this->con->createStatement();
    res = stmt->executeQuery(query);

    while (res->next())
    {
        sql::SQLString sqlstr;

        sqlstr __tn = res->getString("table_name");
        tableNames.pop_back(std::string(__tn));
    }

    delete stmt;
    delete res;

    return tableNames;
}

template <typename T>
std::vector<typename server<T>::Database::Table> server<T>::Database::fetchTableData(void)
{
    if (this->con == nullptr || this->driver == nullptr)
        return std::vector<Table>();

    sql::Statement *stmt = nullptr;
    sql::ResultSet *res = nullptr;

    std::vector<Table> table;

    std::vector<std::string> tableNames = getTableNames();

    for (const std::string &__tn : tableNames)
    {
        std::unordered_map<std::string, std::string> rowData;
        
        std::string query = "SELECT * FROM " + __tn;

        stmt = this->con->createStatement();
        res = stmt->executeQuery(query);

        while (res->next())
            for (unsigned int i = 1; i <= numColumns; i++)
            {
                std::string columnName = metaData->getColumnLabel(i);
                std::string columnValue = res->getString(i);
                rowData[columnName] = columnValue;
            }

        delete res;
        delete stmt;

        table.emplace_back(Table(rowData));
    }
}

/* server */

template <typename T>
server<T>::server()
{
    this->db = nullptr;
    this->__user = nullptr;
}

template <typename T>
void server<T>::acceptConnection(const int serverSocketFileDescriptor, class acceptedSocket *__acceptedSocket)
{
    struct sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);

    int clientSocketFD = accept(serverSocketFileDescriptor, (struct sockaddr *)&clientAddress, (socklen_t *)&clientAddressSize);

    __acceptedSocket->getAcceptedSocket(clientAddress, clientSocketFD, clientSocketFD > 0);
}

char *route = nullptr;
bool changeRoute = false;

template <typename T>
int server<T>::POSTrequestsHandler(T *buffer, int acceptedSocketFileDescriptor, ssize_t __bytesReceived)
{
    uint8_t *byteBuffer = reinterpret_cast<uint8_t *>(buffer);
    char *charBuffer = reinterpret_cast<char *>(buffer);

    // Upon getting the initial buffer, it establishes the pathway for incoming data until 'changeRoute' is reset again.
    if (changeRoute)
    {
        if (route != nullptr)
            delete[] route;

        char *ptr = new char[strlen(charBuffer) + 1];
        strcpy(ptr, charBuffer);

        for (unsigned int i = 0, n = strlen(ptr); i < n; i++)
            if (ptr[i] == '/')
            {
                route = new char[strlen(ptr + i) + 1];
                strcpy(route, ptr + i);

                break;
            }

        for (unsigned int i = 0, n = strlen(route); i < n; i++)
            if (route[i] == ' ')
                route[i] = '\0';

        delete[] ptr;

        changeRoute = false;
    }

    /**
     * @brief Add routes which you implement in the @file interface.hpp
     * 
     * @example
     * 
     * if (findString(route, "/userlogin"))
     *  if (__user->loginRoute(charBuffer, acceptedSocketFileDescriptor) == EXIT_FAILURE)
     *      return EXIT_FAILURE;
     * 
    */

    return EXIT_SUCCESS;
}

template <typename T>
int server<T>::GETrequestsHandler(T *buffer, int acceptedSocketFileDescriptor)
{
    bool USE_DEFAULT_ROUTE = false;

    char *path = nullptr,
         *allocatedBuffer = reinterpret_cast<char *>(buffer);

    if (allocatedBuffer == nullptr)
        return EXIT_FAILURE;

    for (int i = 0, n = strlen(allocatedBuffer); i < n; i++)
        if (allocatedBuffer[i] == '/')
        {
            path = &allocatedBuffer[i];
            break;
        }

    if (path == nullptr)
        USE_DEFAULT_ROUTE = true;

    std::ifstream file;

    if (!USE_DEFAULT_ROUTE)
    {
        for (int i = 0, n = strlen(path); i < n; i++)
            if (path[i] == ' ')
                path[i] = '\0';

        if ((strlen(path) == 1 && path[0] == '/'))
            USE_DEFAULT_ROUTE = true;

        if (strcmp(path, "/apology.html") != 0 && strcmp(path, "/login.html") != 0 && strcmp(path, "/changePassword.html") != 0 &&
            strcmp(path, "/createAccount.html") != 0 && !findString(path, ".css") && !findString(path, ".png") && !__user->getAuthStatus())
            USE_DEFAULT_ROUTE = true;

        if (!USE_DEFAULT_ROUTE)
        {
            if (strcmp(path, "/login.html") == 0)
            {
                __user->resetAuthStatus();
                __user->resetSessionID();
            }

            char fullPath[strlen(ROOT) + strlen(path) + 1] = "";

            if (!findString(path, ROOT))
                strcpy(fullPath, ROOT);

            strcat(fullPath, path);

            file.open(fullPath, std::ios::binary);
        }
    }

    if (USE_DEFAULT_ROUTE)
        file.open(DEFAULT_ROUTE, std::ios::binary);

    if (!file.is_open())
    {
        if (DEBUG_FLAG)
            std::cerr << std::setw(5) << " "
                      << "--> Encountered an error while attempting to open the file: " << path << '\n';
        return EXIT_FAILURE;
    }

    std::ostringstream response;
    response << "HTTP/1.1 200 OK\r\nContent-Length: ";

    file.seekg(0, std::ios::end);
    int size = file.tellg();
    response << size << "\r\n\r\n";

    file.seekg(0, std::ios::beg);
    response << file.rdbuf();

    if (send(acceptedSocketFileDescriptor, response.str().c_str(), response.str().size(), 0) == -1)
    {
        std::cerr << std::setw(5) << " "
                  << "--> Error: Failed to send HTTP response.\n";
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

char *requestType = nullptr;

template <typename T>
int server<T>::HTTPrequestsHandler(T *buffer, int acceptedSocketFileDescriptor, ssize_t __bytesReceived)
{
    char *charBuffer = reinterpret_cast<char *>(buffer),
         *copyBuffer = new char[strlen(charBuffer) + 1];

    strcpy(copyBuffer, charBuffer);

    char *ptr = strstr(copyBuffer, "GET");

    if (ptr == NULL)
    {
        strcpy(copyBuffer, charBuffer);
        ptr = strstr(copyBuffer, "POST");
    }

    if (ptr != NULL)
    {
        if (requestType != nullptr)
            delete[] requestType;

        changeRoute = true;

        for (unsigned int i = 0, n = strlen(ptr); i < n; i++)
            if (ptr[i] == ' ')
                ptr[i] = '\0';

        requestType = new char[strlen(ptr) + 1];

        strcpy(requestType, ptr);
    }

    if (strcmp(requestType, "GET") == 0)
        if (GETrequestsHandler(buffer, acceptedSocketFileDescriptor) == EXIT_FAILURE)
        {
            delete[] copyBuffer;
            copyBuffer = nullptr;

            return EXIT_FAILURE;
        }

    if (strcmp(requestType, "POST") == 0)
        if (POSTrequestsHandler(buffer, acceptedSocketFileDescriptor, __bytesReceived) == EXIT_FAILURE)
        {
            delete[] copyBuffer;
            copyBuffer = nullptr;

            return EXIT_FAILURE;
        }

    if (copyBuffer != nullptr)
    {
        delete[] copyBuffer;
        copyBuffer = nullptr;
    }

    return EXIT_SUCCESS;
}

template <typename T>
void server<T>::postRecv(const int acceptedSocketFileDescriptor)
{
    // post recv
}

template <typename T>
void server<T>::receivedDataHandler(class acceptedSocket *socket)
{
    int acceptedSocketFD = socket->getAcceptedSocketFileDescriptor();

    T buffer[1025];

    while (true)
    {
        ssize_t bytesReceived = recv(acceptedSocketFD, buffer, sizeof(buffer), 0);

        if (bytesReceived <= 0)
        {
            if (DEBUG_FLAG)
                std::cerr << "\n" << std::setw(5) << " "
                          << "--> Receive failed: "
                          << socket->getError() << "\n";

            postRecv(acceptedSocketFD);

            break;
        }

        if (DEBUG_FLAG)
        {
            std::cout << "\n";
            underline(75);
        }

        buffer[bytesReceived] = '\0';

        if (DEBUG_FLAG)
            std::cout << buffer;

        HTTPrequestsHandler(buffer, acceptedSocketFD, bytesReceived);
    }

    close(socket->getAcceptedSocketFileDescriptor());
    delete socket;
}

template <typename T>
void server<T>::receivedDataHandlerThread(class acceptedSocket *socket)
{
    std::thread printThread(&server::receivedDataHandler, this, socket);
    printThread.detach();
}

template <typename T>
void server<T>::handleClientConnections(int serverSocketFileDescriptor)
{
    while (server::SERVER_RUNNING)
    {
        server::acceptedSocket *newAcceptedSocket = new server::acceptedSocket();

        acceptConnection(serverSocketFileDescriptor, newAcceptedSocket);

        connectedSockets.push_back(*newAcceptedSocket);

        receivedDataHandlerThread(newAcceptedSocket);
    }
}

template <typename T>
void server<T>::consoleListener(void)
{
    underline(75);

    char input[101] = "";

    while (SERVER_RUNNING)
    {
        std::cout << std::setw(5) << " "
                  << "--> ";
        std::cin >> input;

        if (strcasecmp(input, "exit") == 0)
        {
            SERVER_RUNNING = false;

            if (!DEBUG_FLAG)
                system("clear");
        }
    }
}

template <typename T>
void server<T>::__SERVER_INIT__(int serverSocketFileDescriptor)
{
    SERVER_RUNNING = true;

    std::thread workerThread(&server::handleClientConnections, this, serverSocketFileDescriptor);
    workerThread.join();

    // std::thread consoleListenerThread(&consoleListener);
    // consoleListenerThread.join();
}

template <typename T>
int server<T>::bindServer(int serverSocketFileDescriptor, struct sockaddr_in *serverAddress)
{
    return bind(serverSocketFileDescriptor, (struct sockaddr *)serverAddress, sizeof(struct sockaddr_in));
}

template <typename T>
std::vector<class server<T>::acceptedSocket> server<T>::getConnectedSockets(void) const noexcept
{
    return connectedSockets;
}

template <typename T>
bool server<T>::getServerStatus(void) const noexcept
{
    return SERVER_RUNNING;
}

template <typename T>
class server<T>::database *server<T>::getSQLdatabase(void) const noexcept
{
    return db;
}

template <typename T>
class interface::user *server<T>::getUser(void) const noexcept
{
    return __user;
}

template <typename T>
void server<T>::SQLfetchUserTable(void)
{
    __user->clearUserCredentials();

    sql::Statement *stmt = nullptr;
    sql::ResultSet *res = nullptr;

    stmt = db->getCon()->createStatement();
    res = stmt->executeQuery("SELECT * FROM user");

    while (res->next())
    {
        int id = res->getInt("id");

        sql::SQLString sqlstr;

        sqlstr = res->getString("username");
        char *username = (char *)malloc(sqlstr.asStdString().length() + 1);
        strcpy(username, sqlstr.asStdString().c_str());

        sqlstr = res->getString("password");
        char *password = (char *)malloc(sqlstr.asStdString().length() + 1);
        strcpy(password, sqlstr.asStdString().c_str());

        user::userCredentials t_uc(username, password, id); // create an obj which we are pushing into the vector

        __user->addToUserCredentials(t_uc);

        free(username);
        free(password);
    }

    res->close();
    stmt->close();

    delete res;
    delete stmt;
}

template <typename T>
void server<T>::SQLfetchFileTable(void)
{
    __user->clearUserFiles();

    sql::Statement *stmt = nullptr;
    sql::ResultSet *res = nullptr;

    std::string SQLquery = "SELECT * FROM file WHERE user_id=" + std::to_string(__user->getSessionID());

    stmt = db->getCon()->createStatement();
    res = stmt->executeQuery(SQLquery);

    while (res->next())
    {
        int userID = res->getInt("user_id");
        int fileID = res->getInt("file_id");
        int fileSize = res->getInt("size");
        int downloads = res->getInt("no_of_downloads");

        sql::SQLString sqlstr;
        sqlstr = res->getString("name");
        char *fileName = (char *)malloc(sqlstr.asStdString().length() + 1);
        strcpy(fileName, sqlstr.asStdString().c_str());

        user::userFiles t_uf(fileName, userID, fileID, fileSize, downloads);

        __user->addToUserFiles(t_uf);

        free(fileName);
    }

    res->close();
    stmt->close();

    delete res;
    delete stmt;
}

template <typename T>
int server<T>::addToFileTable(const char *fileName, const int fileSize)
{
    bool found = false;
    std::vector<class user::userFiles> __userFiles = __user->getUserFiles();

    for (const auto &__uf : __userFiles)
        if (strcmp(__uf.getFileName(), fileName) == 0)
        {
            found = true;
            break;
        }

    if (found)
        return EXIT_SUCCESS;

    int maxID = 0;

    sql::Statement *stmt = nullptr;
    sql::ResultSet *res = nullptr;

    stmt = db->getCon()->createStatement();
    res = stmt->executeQuery("SELECT file_id FROM file");

    while (res->next())
    {
        int fileID = res->getInt("file_id");
        if (fileID > maxID)
            maxID = fileID;
    }

    delete stmt;
    delete res;

    try
    {
        std::string tableName = "file";
        std::string query = "INSERT INTO " + tableName + " (user_id, file_id, name, size, no_of_downloads) VALUES (?, ?, ?, ?, ?)";

        sql::PreparedStatement *prepStmt = db->getCon()->prepareStatement(query);

        prepStmt->setInt(1, __user->getSessionID());
        prepStmt->setInt(2, maxID + 1);
        prepStmt->setString(3, std::string(fileName));
        prepStmt->setInt(4, fileSize);
        prepStmt->setInt(5, 0);

        prepStmt->executeUpdate();

        delete prepStmt;
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "\n"
                  << "Error code: " << e.getErrorCode() << "\n"
                  << "Error message: " << e.what() << "\n"
                  << "SQLState: " << e.getSQLState() << "\n";

        return EXIT_FAILURE;
    }

    SQLfetchFileTable();

    __user->buildIndexHTML();

    return EXIT_SUCCESS;
}

template <typename T>
int server<T>::__database_init__(void)
{ 
    if (db != nullptr)
    {
        std::cout << std::setw(5) << " "
                  << "--> Database is already running.\n";
        return EXIT_SUCCESS;
    }

    char *hostname = (char *)malloc(LENGHT * sizeof(char) + 1);

    if (hostname == NULL)
    {
        std::cerr << std::setw(5) << " "
                  << "--> Error: Failed to allocate hostname memory.\n";

        return EXIT_FAILURE;
    }

    char *username = (char *)malloc(LENGHT * sizeof(char) + 1);

    if (username == NULL)
    {
        std::cerr << std::setw(5) << " "
                  << "--> Error: Failed to allocate username memory.\n";
        free(hostname);

        return EXIT_FAILURE;
    }

    char *password = (char *)malloc(LENGHT * sizeof(char) + 1);

    if (password == NULL)
    {
        std::cerr << std::setw(5) << " "
                  << "--> Error: Failed to allocate password memory.\n";
        free(hostname);
        free(username);

        return EXIT_FAILURE;
    }

    char *database = (char *)malloc(LENGHT * sizeof(char) + 1);

    if (database == NULL)
    {
        std::cerr << std::setw(5) << " "
                  << "--> Error: Failed to allocate database name memory.\n";
        free(hostname);
        free(username);
        free(password);

        return EXIT_FAILURE;
    }

    if (server<T>::database::dbCredentials::getCredentials(hostname, username, password, database) == EXIT_FAILURE)
    {
        std::cerr << std::setw(5) << " "
                  << "--> Error: Failed to fetch MySQL schema credentails.\n";

        free(hostname);
        free(username);
        free(password);
        free(database);

        return EXIT_FAILURE;
    }

    try
    {
        sql::Driver *driver = nullptr;
        sql::Connection *con = nullptr;

        driver = sql::mysql::get_mysql_driver_instance();
        con = driver->connect("tcp://" + std::string(hostname), std::string(username), std::string(password));

        if (con == nullptr)
        {
            std::cerr << std::setw(5) << " "
                      << "--> Error: Failed to establish a connection to the database.\n";

            free(hostname);
            free(username);
            free(password);
            free(database);

            return EXIT_FAILURE;
        }

        con->setSchema(database);

        db = new server<T>::database(driver, con, hostname, username, password, database);
        __user = new interface::user;
    }
    catch (sql::SQLException &e)
    {
        std::cerr << "\n\n"
                  << std::setw(5) << " "
                  << "Error code: " << e.getErrorCode() << "\n"
                  << std::setw(5) << " "
                  << "Error message: " << e.what() << "\n"
                  << std::setw(5) << " "
                  << "SQLState: " << e.getSQLState() << "\n\n";

        free(hostname);
        free(username);
        free(password);
        free(database);

        return EXIT_FAILURE;
    }

    free(hostname);
    free(username);
    free(password);
    free(database);

    SQLfetchUserTable();

    return EXIT_SUCCESS;
}

template <typename T>
server<T>::~server()
{
    if (this->db != nullptr)
    {
        delete this->db;
        this->db = nullptr;
    }

    if (this->__user != nullptr)
    {
        delete this->__user;
        this->__user = nullptr;
    }

    connectedSockets.clear();
}

/* acceptedSocket */

template <typename T>
void server<T>::acceptedSocket::getAcceptedSocket(const struct sockaddr_in ipAddress, const int acceptedSocketFileDescriptor, const int error)
{
    this->ipAddress = ipAddress;
    this->acceptedSocketFileDescriptor = acceptedSocketFileDescriptor;
    this->error = error;
}

template <typename T>
struct sockaddr_in server<T>::acceptedSocket::getIpAddress(void) const noexcept
{
    return ipAddress;
}

template <typename T>
int server<T>::acceptedSocket::getError(void) const noexcept
{
    return error;
}

template <typename T>
int server<T>::acceptedSocket::getAcceptedSocketFileDescriptor(void) const noexcept
{
    return acceptedSocketFileDescriptor;
}