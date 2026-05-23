cmake_minimum_required(VERSION 3.21)
project(task_system)

set(CMAKE_CXX_STANDARD 20)

add_executable(task_system main.cpp thread_task_system.cpp)
