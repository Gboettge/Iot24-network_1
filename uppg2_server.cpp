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

#define PORT 8080
#define BUFFER_SIZE 1024
#define IP_ADDRESS "127.0.0.1"

class ThreadPool
{
public:
    ThreadPool(size_t num_threads) : stop(false)
    {
        for (size_t i = 0; i < num_threads; ++i)
        {
            workers.emplace_back([this]
                                 {
                while (true) {
                    std::function<void()> task;

                    {
                        std::unique_lock<std::mutex> lock(queue_mutex);
                        condition.wait(lock, [this] {
                            return stop || !tasks.empty();
                        });

                        if (stop && tasks.empty())
                            return;

                        task = std::move(tasks.front());
                        tasks.pop();
                    }

                    task(); // Kör uppgiften
                } });
        }
    }

    // Lägg till en uppgift i kön
    void enqueue(std::function<void()> task)
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            tasks.push(std::move(task));
        }
        condition.notify_one();
    }

    // Stäng ner trådpoolen
    ~ThreadPool()
    {
        {
            std::unique_lock<std::mutex> lock(queue_mutex);
            stop = true;
        }
        condition.notify_all();
        for (auto &thread : workers)
            thread.join();
    }

private:
    std::vector<std::thread> workers;
    std::queue<std::function<void()>> tasks;

    std::mutex queue_mutex;
    std::condition_variable condition;
    bool stop;
};

int main()
{
    ThreadPool pool(5);
    const char *welcome_message = "Hello from IoT Network Programming Server!";
    int server_fd;
    if ((server_fd = socket(AF_INET, SOCK_STREAM, 0)) == 0)
    {
        std::cerr << "Socket creation failed" << std::endl;
        return 1;
    }
    int opt = 1;
    if (setsockopt(server_fd, SOL_SOCKET, SO_REUSEADDR, &opt,
                   sizeof(opt)))
    {
        std::cerr << "setsockopt failed" << std::endl;
        close(server_fd);
        return 1;
    }
    struct sockaddr_in address;

    address.sin_family = AF_INET;                      // set to IPv4
    inet_pton(AF_INET, IP_ADDRESS, &address.sin_addr); // set server adress to define (127.0.0.1)
    address.sin_port = htons(PORT);                    // set port (define (8080))
    if (bind(server_fd, (struct sockaddr *)&address,
             sizeof(address)) < 0)
    {
        std::cerr << "Bind failed" << std::endl;
        close(server_fd);
        return 1;
    }
    if (listen(server_fd, 20) < 0)
    { // Allow 1 connection
        std::cerr << "Listen failed" << std::endl;

        close(server_fd);
        return 1;
    }
    std::cout << "Server listening on 127.0.0.1:" << PORT << std::endl;

    //
    int addrlen = sizeof(address);
    int client_sock;

    while (true) // loop for accepting new clients
    {
        // Accept a client connection
        if ((client_sock = accept(server_fd, (struct sockaddr *)&address, (socklen_t *)&addrlen)) < 0)
        {
            std::cerr << "Accept fail" << std::endl;
        }
        else
        {
            std::cout << "Client connected" << std::endl;
        }

        pool.enqueue([client_sock]()
                     {
                         std::cout << "Running in thread: " << std::this_thread::get_id() << std::endl;
                         char buffer[BUFFER_SIZE] = {0};
                         while (true)
                         {
                             memset(buffer, 0, BUFFER_SIZE);

                             int bytes_read = recv(client_sock, buffer, BUFFER_SIZE - 1, 0);
                             if (bytes_read <= 0)
                             {
                                 std::cout << "Client disconnected" << std::endl;
                                 break;
                             }

                             buffer[bytes_read] = '\0';
                             std::cout << "Received: " << buffer << std::endl;

                             if (strcmp(buffer, "NORMAL_DATA:Hello") == 0)
                             {
                                 const char *response = "SERVER_ACK:Hello";
                                 if (send(client_sock, response, strlen(response), 0) < 0)
                                 {
                                     perror("Failed to send normal ack");
                                 }
                                 else
                                 {
                                     std::cout << "Sent: " << response << std::endl;
                                 }
                             }

                             else if (strcmp(buffer, "SEND_URGENT_REQUEST") == 0)
                             {
                                 char oob_byte;
                                 usleep(200000); // wait, I'm trying to give the server time to read the urgent byte

                                 int oob_res = recv(client_sock, &oob_byte, 1, MSG_OOB);
                                 if (oob_res == 1)
                                 {
                                     std::string response = "SERVER_URGENT_ACK:";

                                     response += oob_byte + CLOCK_THREAD_CPUTIME_ID;

                                     if (send(client_sock, response.c_str(), response.length(), 0) < 0)
                                     {
                                         perror("Failed to send urgent ack");
                                     }
                                     else
                                     {
                                         std::cout << "Sent: " << response << std::endl;
                                     }
                                 }
                                 else
                                 {
                                     const char *no_urgent = "SERVER_NO_URGENT_DATA";
                                     send(client_sock, no_urgent, strlen(no_urgent), 0);
                                     std::cout << "Sent: " << no_urgent << std::endl;
                                 }
                             }
                             else if (strcmp(buffer, "Trailing") == 0)
                             {
                                 memset(buffer, 0, BUFFER_SIZE);
                             }

                             else
                             {
                                 const char *unknown = "SERVER_UNKNOWN_COMMAND";
                                 send(client_sock, unknown, strlen(unknown), 0);
                                 std::cout << "Sent: " << unknown << std::endl;
                             }
                         }

                         close(client_sock); // Close the client socket
                     });
    }
    close(server_fd); // Close the server socket
    return 0;
}