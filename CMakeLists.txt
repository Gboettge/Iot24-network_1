cmake_minimum_required(VERSION 3.18.4)

set(CMAKE_CXX_STANDARD 11) #sätter standard till 11
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Aktivera varningar
add_compile_options(-Wall -Wextra -pedantic)

project(server)

add_executable(server

robust_server.cpp

)
add_executable(client
client.cpp
)

find_package(Threads REQUIRED)
target_link_libraries(server PRIVATE Threads::Threads)

# Cmake instucions https://cmake.org/cmake/help/latest/guide/tutorial/A%20Basic%20Starting%20Point.html


# COMMANDS:
# mkdir server_build
# cmake --build .
# make
# ./server
# ./client
# chmod +x ./server