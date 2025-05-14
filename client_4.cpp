#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 8080
#define BUFFER_SIZE 1024
#define SERVER_IP "127.0.0.1"

int main()
{
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        return 1;
    }
    struct sockaddr_in server_addr;
    memset(&server_addr, 0, sizeof(server_addr));
    server_addr.sin_family = AF_INET;
    server_addr.sin_port = htons(PORT);
    if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <=
        0)
    {
        std::cerr << "Invalid address or address not supported" << std::endl;
        close(sock);
        return 1;
    }
    std::cout << "Connecting to server..." << std::endl;
    if (connect(sock, (struct sockaddr *)&server_addr,
                sizeof(server_addr)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        close(sock);
        return 1;
    }
    char buffer[BUFFER_SIZE] = {0};
    int valread = read(sock, buffer, BUFFER_SIZE);
    std::cout << "Message from server: " << buffer << std::endl;

    std::string my_server_ip = SERVER_IP;
    int my_server_port = PORT;
    std::string message_to_send = my_server_ip + ":" + std::to_string(my_server_port) + " Gustav";

    send(sock, message_to_send.c_str(), message_to_send.length(), 0);
    std::string recieved;
    recv(sock, &recieved, recieved.length(), 0);
    std::cout << "Sent to Registry Server: " << message_to_send << std::endl;

    close(sock);
    return 0;
}

/*
g++ -o server_4 server_4.cpp -std=c++11


g++ -o client_4 client_4.cpp -std=c++11

*/