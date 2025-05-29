cmake_minimum_required(VERSION 3.18.4)

#set c++ standar to c++11
set(CMAKE_CXX_STANDARD 11) 
set(CMAKE_CXX_STANDARD_REQUIRED ON)


add_compile_options(-Wall -Wextra -pedantic)

#project name
project(server)

# converts robust_server.cpp to: server when compiling to make ease
add_executable(server
robust_server.cpp
)

#same with client so i cant type only ./client
add_executable(client
client.cpp
)


find_package(Threads REQUIRED)
target_link_libraries(server PRIVATE Threads::Threads)

# Cmake instucions https://cmake.org/cmake/help/latest/guide/tutorial/A%20Basic%20Starting%20Point.html


# COMMANDS:
# cmake -S . -B build
# cd build
# mkdir server_build
# cmake --build .
# make
# ./server
# ./client
# chmod +x ./server

# chmod +x multiple_clients.sh
# ./multiple_clients.sh