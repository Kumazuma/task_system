#include <iostream>
#include <string>
#include "thread_task_system.hpp"

void testArgs(int a, std::string b) {
    for(int i = 0; i < 5; i++) {
        std::cout << "Task with args: " << a << ", " << b << " - " << i << std::endl;
        ThreadTaskSystem::YieldTask();
    }
}

int main()
{
    ThreadTaskSystem taskSystem;
    int num = 0;
    taskSystem.AddTask([num]() {
       for(int i = 0; i < 100; i++) {
           std::cout << num << ":" << i << std::endl;
           ThreadTaskSystem::YieldTask();
       }
    });

    num = 1;
    taskSystem.AddTask([num]() {
       for(int i = 0; i < 100; i++) {
           std::cout << num << ":" << i << std::endl;
           ThreadTaskSystem::YieldTask();
       }
    });

    taskSystem.AddTask(testArgs, 42, "hello");

    taskSystem.Join();
    return 0;
}
