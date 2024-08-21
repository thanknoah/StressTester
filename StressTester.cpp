// Imports [NOTE ONLY SUPPORTS WINDOWS]
#include <string>
#include <cstring>
#include <iostream>
#include <thread>
#include <vector>
#include <asio.hpp>
#include <cctype>
#include <windows.h>
#include <chrono>

using asio::ip::tcp;

class TcpClientConnection : public std::enable_shared_from_this<TcpClientConnection> {
public:
    TcpClientConnection(asio::io_context& ioContext, const tcp::endpoint& endPoint, const std::string& message, bool& responseEnabled, int& duration)
        : socket(ioContext), endPoints(endPoint), message(message), responseEnabled(responseEnabled), duration(duration) {}

    void connect() {
        auto self(shared_from_this());
        socket.async_connect(endPoints, [this, self](asio::error_code ec) {
             if (!ec) {
                 doRequestWork();
             }
        });
    }

    void doRequestWork() {
        auto self(shared_from_this());
        if (responseEnabled) { sendMsgResponse(); }
        if (!responseEnabled) { sendMsg(); }
    }

private:
    void sendMsgResponse() {
        auto self(shared_from_this());
        asio::async_write(socket, asio::buffer(message),
            [this, self](const asio::error_code& ec, std::size_t len) {
                if (!ec) {
                    //e
                }
            });
    }

    void sendMsg() {
        auto self(shared_from_this());
        asio::async_write(socket, asio::buffer(message),
            [this, self](const asio::error_code& ec, std::size_t len) {
                if (!ec) {
                    //e
                }
            });
    }


    void sendPacket(char data[], int CHUNK_SIZE) {
        auto self(shared_from_this());
        asio::async_write(socket, asio::buffer(data, CHUNK_SIZE),
            [this, self](const asio::error_code& ec, std::size_t len) {
                if (!ec) {
                    // std::cout << "[TCP_CLIENT]: SENT PACKET TO SERVER\n";
                }
            });
    }

    void recieveMsg() {
        auto self(shared_from_this());
        asio::async_read_until(socket, responseBuffer, '\n',
            [this, self](const asio::error_code& ec, std::size_t len) {
                if (!ec) {
                    std::istream resp_stream(&responseBuffer);
                    std::string resp;
                    std::getline(resp_stream, resp);
                    std::cout << "[TCP_RESPONSE]: " << resp << "\n";
                }
                if (ec) {
                    std::cout << ec.message();
                }
            });
    }

    asio::io_context ioContext;
    asio::streambuf responseBuffer;
    tcp::socket socket;
    tcp::endpoint endPoints;
    std::array<char, 1024> data;
    std::string message;
    bool responseEnabled;
    int duration;
};

class ThreadManager {
public:
    int numThreads;
    int durThreads;
    int amountOfUsersSimulate;
    bool responseEnabled;
    std::vector<std::thread> threads;
    std::string ip;
    std::string port;
    std::string msg;

    void startThreads() {
        asio::io_context ioContext;
        tcp::resolver resolver(ioContext);
        auto endPoints = resolver.resolve(ip, port);
        auto start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < amountOfUsersSimulate; i++) {
            auto client = std::make_shared<TcpClientConnection>(ioContext, *endPoints.begin(), msg, responseEnabled, durThreads);
            client->connect();
        }

        for (int n = 0; n < 1; n++) {
            threads.emplace_back([&ioContext]() {
                ioContext.run();
                });
        }

        for (auto& thread : threads) {
            thread.join();
        }

        auto end = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> duration = end - start;
        std::cout << "Time taken: " << duration.count() << " seconds" << std::endl << "\n";
    }
private:
    void counter() {
        while (durThreads != 0) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
            durThreads--;
        }
    }
};

void errHandler(int var1, int var2) {
    std::cerr << "Setting to default settings...";
    var1 = 1000;
    var2 = 1;
}

std::string toLowerCase(const std::string& input) {
    std::string result = input;
    for (char& ch : result) {
        ch = std::tolower(static_cast<unsigned char>(ch));
    }
    return result;
}

void getRamInfo() {
    MEMORYSTATUSEX STATEX_;
    STATEX_.dwLength = sizeof(STATEX_);

    if (GlobalMemoryStatusEx(&STATEX_)) {
        std::cout << "Total RAM Available: " << STATEX_.ullTotalPhys / (1024 * 1024 * 1024) << " GB" << std::endl;
    }
}

void getCpuCores() {
    std::cout << "Number of CPU cores [AVAILABLE TO USE]: " << std::thread::hardware_concurrency() - 1 << " Cores" << std::endl << "\n";
}

int main() {
    std::string typeOfContentToSend;
    std::string responseType;
    std::string connectionType;
    std::string port;
    std::string msg;
    std::string ip;
    int packetSize;
    int amountOfFakeUsers;
    int amountOfThreads;
    int durationOfThreads;
    bool responseEnabled = false;

    const WORD PURPLE = FOREGROUND_RED | FOREGROUND_BLUE;
    SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), PURPLE);

    std::cout << "StressTester V1.4 Beta" << std::endl;
    std::cout << "Welcome back user.\n" << std::endl;

    // Setting Variables /w input
    try {
        getRamInfo();
        getCpuCores();

        std::cout << "Enter Request Type [TCP only supported] >> ";
        std::cin >> connectionType;
        std::cout << "Amount of Requests [Total] >> ";
        std::cin >> amountOfFakeUsers;
        std::cout << "\nDuration (s) [NOT WORKING RN]>> ";
        std::cin >> durationOfThreads;
        std::cout << "Amount of Cores to use >> ";
        std::cin >> amountOfThreads;
        std::cout << "\nEnable Response [On, Off] >> ";
        std::cin >> responseType;

        if (amountOfThreads > std::thread::hardware_concurrency() - 1) {
            throw std::runtime_error("Exceeded thread limit");
        }
        if (toLowerCase(connectionType) == "tcp" || toLowerCase(connectionType) == "udp") {
            std::cout << "IP address >> ";
            std::cin >> ip;
            std::cout << "Port >> ";
            std::cin >> port;
        }
        if (toLowerCase(responseType) == "On" || toLowerCase(responseType) == "on") {
            responseEnabled = true;
        }

        std::cout << "\nMessage or packet >> ";
        std::cin >> typeOfContentToSend;

        if (toLowerCase(typeOfContentToSend) == "message") {
            std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
            std::cout << "Message to send >> ";
            std::getline(std::cin, msg);
            std::cout << "\n";
        }
        else if (toLowerCase(typeOfContentToSend) == "packet") {
            //std::cout << "Size of packet [KB] >> ";
            //std::cin >> packetSize;
            //packetSize = packetSize * 1024;
        }

        // Attempting to create socket
        //auto client = std::make_unique<TcpClientConnection>(ip, port);
        //client->connect();
    }
    catch (const std::runtime_error& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        errHandler(amountOfFakeUsers, amountOfThreads);
        return 1;
    }
    catch (const std::exception& e) {
        std::cerr << "Error: " << e.what() << std::endl;
        return 1;
    }
    catch (...) {
        errHandler(amountOfFakeUsers, amountOfThreads);
        return 1;
    }

    // Create threads
    ThreadManager threads;
    threads.numThreads = amountOfThreads;
    threads.durThreads = durationOfThreads;
    threads.amountOfUsersSimulate = amountOfFakeUsers;
    threads.ip = ip;
    threads.port = port;
    threads.msg = msg;
    threads.responseEnabled = responseEnabled;
    threads.startThreads();

    std::cout << "Done";

    return 0;
}
