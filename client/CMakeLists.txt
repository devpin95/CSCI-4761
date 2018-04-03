# cmake_minimum_required(VERSION <specify CMake version here>)
project(client)

set(CMAKE_CXX_STANDARD 11)

add_executable(client main.cpp app.cpp app.h APP_PROTOCOL.h)