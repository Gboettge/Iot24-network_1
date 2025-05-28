#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <mutex>
#include <queue>
#include <thread>
#include <condition_variable>
#include <functional>
#include <fcntl.h>

#define PORT 8080
#define BUFFER_SIZE 1024
#define IP_ADDRESS "127.0.0.1"

void make_non_blocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

int client_handler(int client_sock)
{
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read;

    const char *response = "SERVER_ACK:";

    bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_read <= 0)
    {
        std::cout << "Client disconnected" << std::endl;
    }
    else if (bytes_read > 1 && bytes_read < BUFFER_SIZE)
    {
        if (buffer[bytes_read] == '\0')
        {
            std::cout << "new line found" << std::endl;
            std::cout << "Recieved: " << buffer << std::endl;
        }
        else
        {
            std::cout << "new line not found" << std::endl;
        }

        if (send(client_sock, response, strlen(response), 0) < 0)
        {
            std::cerr << "Failed to send ACK" << std::endl;
        }
        else
        {
            std::cout << "sent: " << response << std::endl;
        }
    }
    else
    {
        std::cout << "Huge message" << std::endl;
    }
    close(client_sock);
    return 1;
}

int main()
{
    int server_sock;
    if ((server_sock = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Socket creation failed " << std::endl;
        return 1;
    }
    int opt = 1;
    if (setsockopt(server_sock, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)))
    {
        std::cerr << "setsockopt failed" << std::endl;
        close(server_sock);
        return 1;
    }

    make_non_blocking(server_sock);

    struct sockaddr_in address;

    address.sin_family = AF_INET;                      // set to IPv4
    inet_pton(AF_INET, IP_ADDRESS, &address.sin_addr); // set server adress to define (127.0.0.1)
    address.sin_port = htons(PORT);                    // set port (define (8080))
    if (bind(server_sock, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        close(server_sock);
        return 1;
    }

    if (listen(server_sock, 20) < 0) // allow up to 20 connections
    {
        std::cerr << "listen failed" << std::endl;
        close(server_sock);
        return 1;
    }

    std::cout << "Server listening on 127.0.0.1:" << PORT << std::endl;

    int addrlen = sizeof(address);
    int client_sock;
    std::cout << "pre" << std::endl;
    std::cout << "post" << std::endl;

    while (true)
    {
        if ((client_sock = accept(server_sock, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                // Ingen anslutning att acceptera just nu – inte ett fel
                continue;
            }
            else
            {
                std::cerr << "Accept fail" << std::endl;
                close(client_sock);
                return 1;
            }
        }
        else
        {
            std::cout << "client connected" << std::endl;
            make_non_blocking(client_sock);
        }
        std::thread client_thread(client_handler, client_sock);
        // creating new thread to client_handler with the current client_socket
        client_thread.detach();
    }

    return 0;
}