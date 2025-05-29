#include <iostream>
#include <cstring>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#define PORT 8080
#define BUFFER_SIZE 1024
#define IP_ADDRESS "127.0.0.1"

// int main()
// {
//     int sock = socket(AF_INET, SOCK_STREAM, 0);
//     if (sock < 0)
//     {
//         std::cerr << "Socket creation error" << std::endl;
//         return 1;
//     }
//     struct sockaddr_in server_addr;
//     memset(&server_addr, 0, sizeof(server_addr));
//     server_addr.sin_family = AF_INET;
//     server_addr.sin_port = htons(PORT);
//     if (inet_pton(AF_INET, "127.0.0.1", &server_addr.sin_addr) <=
//         0)
//     {
//         std::cerr << "Invalid address or address not supported" << std::endl;
//         close(sock);
//         return 1;
//     }
//     std::cout << "Connecting to server..." << std::endl;
//     if (connect(sock, (struct sockaddr *)&server_addr,
//                 sizeof(server_addr)) < 0)
//     {
//         std::cerr << "Connection failed" << std::endl;
//         close(sock);
//         return 1;
//     }
//     char buffer[BUFFER_SIZE] = {0};
//     int valread = read(sock, buffer, BUFFER_SIZE);
//     std::cout << "Message from server: " << buffer << std::endl;
//     close(sock);
//     return 0;
// }

int main()
{
    int sock = 0;
    struct sockaddr_in serv_addr;
    const char *normal_data = "NORMAL_DATA:Hello";
    const char *urgent_request = "SEND_URGENT_REQUEST";
    char urgent_data = 'U'; // Urgent data
    char buffer[1024] = {0};

    // Create socket
    if ((sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
    {
        std::cerr << "Socket creation error" << std::endl;
        return 0;
    }

    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(PORT);

    if (inet_pton(AF_INET, "127.0.0.1", &serv_addr.sin_addr) <= 0)
    {
        std::cerr << "Address not supported" << std::endl;
        close(sock);
        return 0;
    }

    // Connect to the server
    if (connect(sock, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        std::cerr << "Connection Failed" << std::endl;
        close(sock);
        return 0;
    }

    int scenarios;
    std::cout << "\n Choose scenarios (1-4, other number to quit): ";
    std::cin >> scenarios;

    memset(buffer, 0, BUFFER_SIZE);

    if (scenarios == 1) // scenario one check, normal data
    {
        send(sock, normal_data, strlen(normal_data), 0);
        std::cout << "Sent: " << normal_data << std::endl;
    }
    else if (scenarios == 2)
    {
        char urgent_data = 'U';
        send(sock, urgent_request, strlen(urgent_request), 0);
        std::cout << "Sent: " << urgent_request << std::endl;
        usleep(200000); // sleep 200ms in order to guarantee the urgent bit
        send(sock, &urgent_data, 1, MSG_OOB);
        std::cout << "Sent urgent data: " << urgent_data << std::endl;

        send(sock, "Trailing", 8, 0); // send "trailing"
        std::cout << "Sent: 'Trailing' " << std::endl;
    }
    else if (scenarios == 3)
    {
        send(sock, urgent_request, strlen(urgent_request), 0);
        std::cout << "Sent: " << urgent_request << std::endl;

        shutdown(sock, SHUT_WR); // Disconnect client
    }
    else if (scenarios == 4)
    {
        const char *garbage = "TEST_GARBAGE";
        send(sock, garbage, strlen(garbage), 0);
        std::cout << "Sent: " << garbage << std::endl;
    }
    else if (scenarios == 5)
    {
        int u = 3;
        for (int i = 0; i < u; i++)
        {
            const char *garbage = "TEST_GARBAGE\n";
            send(sock, garbage, strlen(garbage), 0);
            std::cout << "Sent: " << garbage << std::endl;
            sleep(3);
        }
    }
    else if (scenarios == 6)
    {
        // const char *garbage = "TEST_GARBAGEEEEEEEEEE";
        std::string test_message(1023, 'A');
        send(sock, test_message.c_str(), test_message.size(), 0);
        std::cout << "Sent: " << test_message << std::endl;
        int u = 200;
        // for (int i = 0; i < u; i++)
        // {
        //     for (int j = 0; j < 10; j++)
        //     {
        //     }

        // }
        sleep(3);
        const char *garbage = "TEST_GARBAGE\n";
        send(sock, garbage, strlen(garbage), 0);
    }
    else
    {
        std::cout << "Quitting." << std::endl;
    }

    int bytes_received = recv(sock, buffer, BUFFER_SIZE - 1, 0);
    if (bytes_received > 0)
    {
        buffer[bytes_received] = '\0';
        std::cout << "Received: " << buffer << std::endl;
    }
    else if (bytes_received == 0)
    {
        std::cout << "Server closed connection" << std::endl;
    }
    else
    {
        perror("recv failed");
    }

    close(sock); // Close the socket
    return 1;
}

/*
g++ -o server server.cpp -std=c++11


g++ -o client client.cpp -std=c++11

*/