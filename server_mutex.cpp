#include <iostream>
#include <thread>
#include <vector>

int shared_counter = 0;

void increment_counter(int iterations)
{
    for (int i = 0; i < iterations; ++i)
    {
        shared_counter++; // This is NOT atomic! (read-modify-write)
    }
}

int main()
{
    const int num_threads = 10;
    const int iterations_per_thread = 100000;
    std::vector<std::thread> threads;

    for (int i = 0; i < num_threads; ++i)
    {
        threads.emplace_back(increment_counter, iterations_per_thread);
    }

    for (auto &t : threads)
    {
        t.join();
    }
    // Expected: num_threads * iterations_per_thread
    // Actual: Likely less due to race conditions!
    std::cout << "Expected counter: " << num_threads * iterations_per_thread << std::endl;
    std::cout << "Actual counter:   " << shared_counter << std::endl;
    return 0;
}
