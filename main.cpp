#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>

constexpr int PORT = 8080;
constexpr int MAX_EVENTS = 10;

const std::string htmlResponse = R"(
<!DOCTYPE html>
<html>
<head>
    <style>
        body {
            background-color: #f0f0f0;
            font-family: Arial, sans-serif;
        }
        h1 {
            color: #333;
        }
    </style>
</head>
<body>
    <h1>Hello from my C++ web server!</h1>
</body>
</html>
)";

int main() {
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("Error creating socket");
        return 1;
    }

    sockaddr_in serverAddr{};
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(PORT);

    if (bind(serverSocket, reinterpret_cast<sockaddr*>(&serverAddr), sizeof(serverAddr)) == -1) {
        perror("Error binding");
        close(serverSocket);
        return 1;
    }

    listen(serverSocket, 5);

    int epollFd = epoll_create1(0);
    epoll_event event{};
    event.events = EPOLLIN;
    event.data.fd = serverSocket;
    epoll_ctl(epollFd, EPOLL_CTL_ADD, serverSocket, &event);

    while (true) {
        epoll_event events[MAX_EVENTS];
        int numEvents = epoll_wait(epollFd, events, MAX_EVENTS, -1);
        for (int i = 0; i < numEvents; ++i) {
            if (events[i].data.fd == serverSocket) {
                sockaddr_in clientAddr{};
                socklen_t clientAddrLen = sizeof(clientAddr);
                int clientSocket = accept(serverSocket, reinterpret_cast<sockaddr*>(&clientAddr), &clientAddrLen);
                if (clientSocket != -1) {
                    send(clientSocket, htmlResponse.c_str(), htmlResponse.size(), 0);
                    close(clientSocket);
                }
            }
        }
    }

    close(serverSocket);
    return 0;
}
