#include <iostream>
#include "thread_task_system.hpp"

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

    taskSystem.Join();
    return 0;
}
