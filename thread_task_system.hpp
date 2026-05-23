//
// Created by Kumazuma on 26. 5. 23..
//

#ifndef THREAD_TASK_SYSTEM_HPP
#define THREAD_TASK_SYSTEM_HPP

#include <windows.h>
#include <fibersapi.h>
#include <list>
#include <functional>
#include <mutex>

class ThreadTaskSystem {
    struct Task {
        ThreadTaskSystem* owner;
        void* pFiber;
        std::function<void()> task;
        uint32_t index;
    };

public:
    ThreadTaskSystem();

    ThreadTaskSystem(const ThreadTaskSystem&) = delete;

    ThreadTaskSystem& operator=(const ThreadTaskSystem&) = delete;

    ~ThreadTaskSystem();

    bool AddTask(std::function<void()> task);

    void Join();

    static void YieldTask();

private:
    void YieldTask(Task* pTask);

    void Run();

    void DeleteTask(Task* pTask);

    static unsigned WINAPI ThreadProc(LPVOID lpParameter);

    static void WINAPI FiberProc(LPVOID lpParameter);

private:
    bool m_bThreadRunning;
    HANDLE m_hThread;
    DWORD m_threadId;
    Task* m_pIdleTask;
    void* m_pFiber;
    std::mutex m_mutex;
    std::vector<Task*> m_taskQueue;
    uint32_t m_currentTaskIndex;
    uint32_t m_reservedDeleteFiberIndex;
};

#endif //THREAD_TASK_SYSTEM_HPP
