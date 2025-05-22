#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 8080
#define BUFFER_SIZE 1024
#define IP_ADDRESS "127.0.0.1"

int main()
{
    std::cout << "Hellos" << std::endl;
    return 0;
}