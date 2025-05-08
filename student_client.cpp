#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string>
#include <vector>

#define STUDENT_SERVER_PORT 15500
#define INSTRUCTOR_REGISTRY_IP "172.16.219.46"
#define INSTRUCTOR_REGISTRY_PORT 12345
#define MY_SERVER_IP_STRING "172.17.0.2"
#define Richard_PORT 16000
#define RICHARD_IP_STRING "172.16.219.155"
int main()
{
    int mysocket = socket(AF_INET, SOCK_STREAM, 0);
    if (mysocket < 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }

    int optval = 1;
    if (setsockopt(mysocket, SOL_SOCKET, SO_REUSEADDR, &optval,
                   sizeof(optval)) < 0)
    {
        std::cerr << "setsockopt failed: " << strerror(errno) << std::endl;
        close(mysocket);
        return 1;
    }
    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;

    // address.sin_port = htons(INSTRUCTOR_REGISTRY_PORT);
    // int bajs = inet_pton(AF_INET, INSTRUCTOR_REGISTRY_IP, &address.sin_addr);

    address.sin_port = htons(STUDENT_SERVER_PORT);
    int bajs = inet_pton(AF_INET, MY_SERVER_IP_STRING, &address.sin_addr);

    // address.sin_port = htons(Richard_PORT);
    // int bajs = inet_pton(AF_INET, RICHARD_IP_STRING, &address.sin_addr);

    if (bajs < 0)
    {
        std::cerr << "inet_pton failed: " << strerror(errno) << std::endl;
        close(mysocket);
        return 1;
    }
    if (connect(mysocket, (struct sockaddr *)&address,
                sizeof(address)) < 0)
    {
        std::cerr << "Connection failed" << std::endl;
        close(mysocket);
        return 1;
    }
    std::string my_server_ip = MY_SERVER_IP_STRING;
    int my_server_port = STUDENT_SERVER_PORT;
    std::string message_to_send = my_server_ip + ":" + std::to_string(my_server_port) + " Gustav";

    send(mysocket, message_to_send.c_str(), message_to_send.length(), 0);
    std::cout << "Sent to Registry Server: " << message_to_send << std::endl;
    close(mysocket);
}