// Imports [NOTE ONLY SUPPORTS WINDOWS]
#include <string>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <asio.hpp>
#include <cctype>
#include <windows.h>
using asio::ip::tcp;

class TcpClientConnection {

public:
    TcpClientConnection(const std::string& server, const std::string& port) : ioContext(), socket(ioContext) {
        tcp::resolver resolver(ioContext);
        endPoints = resolver.resolve(server, port);
    }

    void connect() {
        asio::connect(socket, endPoints);
    }

    void sendMsg(const std::string& msg) {
        std::cout << msg << std::endl;
        asio::write(socket, asio::buffer(msg));
    }

    void sendPacket(char data[], int CHUNK_SIZE) {
        asio::write(socket, asio::buffer(data, CHUNK_SIZE));
    }

    std::string recieveMsg() {
        char resp[1024];
        size_t len = socket.read_some(asio::buffer(resp));
        return std::string(resp, len);
    }

    void close() {
        socket.close();
    }

private:
    asio::io_context ioContext;
    tcp::socket socket;
    tcp::resolver::results_type endPoints;
};

class ThreadManager {

public:
    int numThreads;
    int durThreads;
    int amountOfUsersSimulate;
    std::vector<std::thread> threads;
    std::string ip;
    std::string port;
    std::string msg;

    void startThreads() {
        for (int n = 0; n < numThreads; ++n) {
            threads.emplace_back(std::thread(&ThreadManager::tcpConnection, this, n));
        }
        for (int n = 0; n < numThreads; ++n) {
            threads.emplace_back(std::thread(&ThreadManager::counter, this));
        }

        // Join threads
        for (auto& thread : threads) {
            thread.join();
        }
    }

    void tcpConnection(int threadId) {
        std::cout << "\nThread [" << threadId << "] is executing.\n" << std::endl;
        std::this_thread::sleep_for(std::chrono::seconds(5));
        int totalAmountOfClients = amountOfUsersSimulate / numThreads;

        try {
            if (msg == "") {
                startStressTestPacket(totalAmountOfClients);
            }
            else {
                startStressTestMsg(totalAmountOfClients);
            }
        }
        catch (const std::exception& e) {
            std::cerr << e.what();
        }
    }

private:
    void counter() {
        while (durThreads != 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            durThreads--;
        }
    }

    void startStressTestPacket(int totalAmountOfClients) {
        auto client = std::make_unique<TcpClientConnection>(ip, port);
        const int CHUNK_SIZE = 64 * 1024;
        char buffer[CHUNK_SIZE];
        memset(buffer, 'A', CHUNK_SIZE);

        for (int y = 0; y < totalAmountOfClients; y++) {
            client->connect();
            client->sendPacket(buffer, CHUNK_SIZE);
            client->close();
            std::cout << "SENT PACKET TO " << ip << std::endl;
            if (durThreads == 0) {
                break;
            }
        }
    }

    void startStressTestMsg(int totalAmountOfClients) {
        auto client = std::make_unique<TcpClientConnection>(ip, port);

        for (int y = 0; y < totalAmountOfClients; y++) {
            client->connect();
            client->sendMsg(msg);
            client->close();
            std::cout << "SENT MESSAGE TO " << ip << std::endl;
            if (durThreads == 0) {
                break;
            }
        }
    }
};

void errHandler(int var1, int var2) {
    std::cerr << "Setting to default settings...";
    var1 = 1000;
    var2 = 1;
}

// Convert to lower case
std::string toLowerCase(const std::string& input) {
    std::string result = input;
    for (char& ch : result) {
        ch = std::tolower(static_cast<unsigned char>(ch));
    }
    return result;
}

void getRamInfo() {
    MEMORYSTATUSEX STATEX;
    STATEX.dwLength = sizeof(STATEX);

    if (GlobalMemoryStatusEx(&STATEX)) {
        std::cout << "Total RAM Available: " << STATEX.ullTotalPhys / (1024 * 1024 * 1024) << " GB" << std::endl;
    }
}

void getCpuCores() {
    std::cout << "Number of CPU cores: " << std::thread::hardware_concurrency() << " Cores" << std::endl;
}

int main() {
    // Essential Variables
    std::string connectionType;
    std::string ip;
    std::string port;
    std::string typeOfContentToSend;
    std::string msg;
    int packetSize;
    int amountOfFakeUsers;
    int amountOfThreads = 1;
    int durationOfThreads;
    
    const WORD PURPLE = FOREGROUND_RED | FOREGROUND_BLUE;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), PURPLE);
    
    std::cout << "StressTester V1.0 Beta" << std::endl;
    std::cout << "Welcome back user.\n" << std::endl;
    getRamInfo();
    getCpuCores();
    std::cout << "Connection type [TCP, UDP, etc] >> ";

    // Setting Variables /w input
    try {
        std::cin >> connectionType;
        std::cout << "Amount of Users to simulate >> ";
        std::cin >> amountOfFakeUsers;
        std::cout << "Duration (s) >> ";
        std::cin >> durationOfThreads;

        if (toLowerCase(connectionType) == "tcp" || toLowerCase(connectionType) == "udp") {
            std::cout << "IP address >> ";
            std::cin >> ip;
            std::cout << "Port >> ";
            std::cin >> port;
        }

        std::cout << "Message or packet >> ";
        std::cin >> typeOfContentToSend;

        if (toLowerCase(typeOfContentToSend) == "message") {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Message to send >> ";
            std::getline(std::cin, msg);
        }
        else if (toLowerCase(typeOfContentToSend) == "packet") {
            std::cout << "Size of packet [KB] >> ";
            std::cin >> packetSize;
            packetSize = packetSize * 1024;
        }

        if (amountOfThreads > std::thread::hardware_concurrency()) {
            throw std::runtime_error("Exceeded thread limit");
        }

        // Attempting to create socket
        auto client = std::make_unique<TcpClientConnection>(ip, port);
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        errHandler(amountOfFakeUsers, amountOfThreads);
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
    }
    catch (...) {
        errHandler(amountOfFakeUsers, amountOfThreads);
    }

    // Create threads
    ThreadManager threads;
    threads.numThreads = 1;
    threads.durThreads = durationOfThreads;
    threads.amountOfUsersSimulate = amountOfFakeUsers;
    threads.ip = ip;
    threads.port = port;
    threads.msg = msg;
    threads.startThreads();

    std::cout << "Done";

    return 0;
}
