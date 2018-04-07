cmake_minimum_required(VERSION 3.9)
project(server)

set(CMAKE_CXX_STANDARD 11)

add_executable(server main.cpp app.cpp app.h APP_PROTOCOL.h Database.cpp Database.h Appt.h Appt.cpp)