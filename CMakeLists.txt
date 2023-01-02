cmake_minimum_required(VERSION 3.24)
project(OS3 C)

set(CMAKE_C_STANDARD 99)

add_executable(OS3 queue.c queue.h request.c request.h segel.c segel.h server.c stat_thread.h stat_thread.c)
