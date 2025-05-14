#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <fcntl.h>

#define STUDENT_SERVER_PORT 15500

bool set_non_blocking(int sockfd)
{
    int flags = fcntl(sockfd, F_GETFL, 0); //

    if (flags == -1)
    {
        perror("fcntl F_GETFL");
        return false;
    }
    if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) == -1)
    {
        perror("lol");
        return false;
    }

    return true;
}

int main()
{
    // int server_fd;
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

    // if(!set_non_blocking(mysocket)){
    //    }

    struct sockaddr_in address;
    memset(&address, 0, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = INADDR_ANY; // accepterar alla adresser
    address.sin_port = htons(STUDENT_SERVER_PORT);

    if (bind(mysocket, (struct sockaddr *)&address, sizeof(address)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        std::cerr << "Bind failed:  " << strerror(errno) << std::endl;
        close(mysocket);
        return 1;
    }
    if (listen(mysocket, 5))
    { // Allow up to 3 pending connections
        std::cerr << "Listen failed" << std::endl;
        close(mysocket);
        return 1;
    }
    std::cout << "Server listening on port " << STUDENT_SERVER_PORT << "..." << std::endl;
    while (true)
    {
        struct sockaddr_in instructor_addr;
        socklen_t instructor_addr_len = sizeof(instructor_addr);
        int instructor_conn_fd = accept(mysocket, (struct sockaddr *)&instructor_addr, &instructor_addr_len);
        if (instructor_conn_fd < 0)
        {
            std::cerr << "Wont accept" << std::endl;
            close(mysocket);
            return 1;
        }
        else
        {
            char instructor_ip_str[INET_ADDRSTRLEN];
            inet_ntop(AF_INET, &instructor_addr.sin_addr, instructor_ip_str, INET_ADDRSTRLEN);
            std::cout << "Connection accepted from instructor's server at " << instructor_ip_str << ":" << ntohs(instructor_addr.sin_port) << std::endl;

            char recieved[100];
            int read = recv(instructor_conn_fd, recieved, sizeof(recieved) - 1, 0);
            if (read != -1)
            {
                // recieved[read] = '\0';
                std::string recieved(recieved);
                std::cout << recieved << std::endl;
                std::cout << "recieved" << std::endl;
            }
            else
            {
                std::cout << "recv not working " << strerror(errno) << std::endl;
                // EAGAIN;
            }
        }
        // recv(mysocket, )
        close(instructor_conn_fd);
    }
    close(mysocket);
}
/*

 g++ -o student_server student_server.cpp -std=c++11

 ./student_server

 g++ -o student_client student_client.cpp -std=c++11

 ./student_client

*/