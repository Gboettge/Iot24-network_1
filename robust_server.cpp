#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <thread>
#include <fcntl.h>
#include <set>
#include <chrono>

#define PORT 8080
#define BUFFER_SIZE 1024
#define IP_ADDRESS "127.0.0.1"

void make_non_blocking(int fd) // set sockets to non blocking
{
    int flags = fcntl(fd, F_GETFL, 0);
    fcntl(fd, F_SETFL, flags | O_NONBLOCK);
}

void client_handler(int client_sock) // handles, different cases from connected clients
{
    char buffer[BUFFER_SIZE] = {0};
    ssize_t bytes_read;
    const char *response = "SERVER_ACK:";

    while (true)
    {
        bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
        if (bytes_read < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }
            else
            {
                std::cerr << "recv failed" << std::endl;
                break;
            }
        }
        // if 0 bytes recv the client has disconnected
        else if (bytes_read == 0)
        {
            std::cout << "Client disconnected" << std::endl;
            break;
        }
        // check if message is too big
        if (bytes_read >= BUFFER_SIZE - 1)
        {
            const char *response = "Message reciaved is too big, disconnecting client";
            std::cout << "Message reciaved is too big" << std::endl;
            memset(buffer, 0, BUFFER_SIZE);
            if (send(client_sock, response, strlen(response), 0) < 0)
            {
                std::cerr << "Failed to send response" << std::endl;
            }
            break;
        }
        else
        {
            // Check if the last byte is a new line, ( if so then it's a complete message)
            if (buffer[bytes_read - 1] == '\0' || buffer[bytes_read - 1] == '\n')
            {

                std::cout << "Received: " << buffer << std::endl;
                if (send(client_sock, buffer, strlen(buffer), 0) < 0)
                {
                    std::cerr << "Failed to send ACK" << std::endl;
                }
            }
            // this is how i handle a not complete message without newline, wait 10s if no "complete message recv then disconnect"
            else
            {
                std::cout << "Received partial message: " << buffer << std::endl;
                std::this_thread::sleep_for(std::chrono::seconds(10)); // sleep thread, if a new message isnt found within 10s disconnect client by (break)
                buffer[bytes_read] = '\0';
                const char *response = "Waited to long, disconnecting client";
                if (send(client_sock, response, strlen(response), 0) < 0)
                {
                    std::cerr << "Failed to send response" << std::endl;
                }
                std::cout << "waited too long closing client" << std::endl;
                break;
            }
        }
    }
    close(client_sock);
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

    fd_set read_fds;
    std::set<int> client_sockets;

    while (true)
    {
        FD_ZERO(&read_fds);
        FD_SET(server_sock, &read_fds);
        int max_fd = server_sock;

        // add all clients to the FD_SET
        for (int sock : client_sockets) // iterate all client sockets
        {
            FD_SET(sock, &read_fds);
            if (sock > max_fd)
            {
                max_fd = sock;
            }
        }

        if (select(max_fd + 1, &read_fds, nullptr, nullptr, nullptr) < 0) // monitor sockets
        {
            std::cerr << "select failed" << std::endl;
            break;
        }

        if (FD_ISSET(server_sock, &read_fds))
        {
            int client_sock = accept(server_sock, nullptr, nullptr);
            if (client_sock < 0)
            {
                std::cerr << "Accept failed" << std::endl;
                continue;
            }
            std::cout << "Client connected" << std::endl;
            make_non_blocking(client_sock); // using my make non-blocking function
            client_sockets.insert(client_sock);
        }
        // iterate over all the client socket so check for data
        auto it = client_sockets.begin();
        while (it != client_sockets.end())
        {
            int sock = *it;
            if (FD_ISSET(sock, &read_fds))
            {
                // create a new thread to handle the new client connection
                std::thread(client_handler, sock).detach();
                it = client_sockets.erase(it); // remove the socket from the set
            }
            else
            {
                ++it;
            }
        }
    }

    close(server_sock); // close server socket when done
    return 0;
}