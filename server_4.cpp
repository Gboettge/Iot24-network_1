#include <cstdlib>
#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <cerrno>
#include <vector>

#define PORT 8080
#define BUFFER_SIZE 1024
#define IP_ADDR "127.0.0.1"

bool set_non_blocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0);

    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        return false;
    }

    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("fcntl F_SETFL O_NONBLOCK");
        return false;
    }

    return true;
}

int main()
{
    int server_fd;
    struct sockaddr_in server_addr, client_addr;
    socklen_t client_addr_len = sizeof(client_addr);
    char buffer[BUFFER_SIZE] = {0};

    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        perror("socket failed");
        exit(EXIT_FAILURE);
    }
    std::cout << "Server socket created." << std::endl;

    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt)))
    {
        perror("setsockopt SO_REUSEADDR failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }

    if (!set_non_blocking(server_fd))
    {
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Listening socket set to non-blocking." << std::endl;

    server_addr.sin_family = AF_INET;
    server_addr.sin_addr.s_addr = INADDR_ANY;
    server_addr.sin_port = htons(PORT);

    if (bind(server_fd, (struct sockaddr *)&server_addr, sizeof(server_addr)) < 0)
    {
        perror("bind failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Socket bound to port " << PORT << std::endl;

    if (listen(server_fd, 3) < 0)
    {
        perror("listen failed");
        close(server_fd);
        exit(EXIT_FAILURE);
    }
    std::cout << "Server listening on " << std::string(IP_ADDR) << ":" << PORT << " (non-blockingly)..." << std::endl;

    std::vector<int> client_sockets;

    while (true)
    {
        struct sockaddr_in temp_client_addr;
        socklen_t temp_client_addr_len = sizeof(temp_client_addr);

        // std::cout << "\nWaiting for a client connection..." << std::endl;
        int new_client_sock = accept(server_fd, (struct sockaddr *)&temp_client_addr, &temp_client_addr_len);

        if (new_client_sock < 0)
        {
            if (errno == EAGAIN || errno == EWOULDBLOCK)
            {
                continue;
            }
            perror("Error accepting socket!");
            break;
        }
        char client_ip_str[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &temp_client_addr.sin_addr, client_ip_str, INET_ADDRSTRLEN);
        std::cout << "Accepted connection from " << client_ip_str << ":" << ntohs(temp_client_addr.sin_port) << std::endl;

        if (!set_non_blocking(new_client_sock))
        {
            std::cerr << "Failed to set client socket to non-blocking";
            close(new_client_sock);
        }
        else
        {
            client_sockets.push_back(new_client_sock);
        }

        for (auto it = client_sockets.begin(); it != client_sockets.end();)
        {
            int current_client_sock = *it;
            memset(buffer, 0, BUFFER_SIZE);
            ssize_t bytes_read = recv(current_client_sock, buffer, BUFFER_SIZE - 1, 0);

            if (bytes_read > 0)
            {
                buffer[bytes_read] = '\0';
                std::cout << "Received from socket " << current_client_sock << ": " << buffer << std::endl;
            }
            else if (bytes_read == 0)
            {
                // Client disconnected gracefully.
                std::cout << "Client on socket " << current_client_sock << " disconnected." << std::endl;
                close(current_client_sock);
                it = client_sockets.erase(it);

                continue;
            }
            else
            {
                if (errno != EAGAIN && errno != EWOULDBLOCK)
                {
                    perror("recv filed");
                    close(current_client_sock);
                    it = client_sockets.erase(it);
                    continue;
                }
            }
            ++it;
        }
        usleep(10000);
    }

    close(server_fd);
    return 0;
}
