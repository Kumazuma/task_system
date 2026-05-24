//
// Created by Kumazuma on 26. 5. 23..
//

#include "thread_task_system.hpp"

#include <process.h>
#include <thread>

ThreadTaskSystem::ThreadTaskSystem()
    : m_hThread(), m_threadId(0) {
    unsigned dwThreadId;
    m_bThreadRunning = true;
    m_reservedDeleteFiberIndex = 0;
    m_currentTaskIndex = 0;
    m_pIdleTask = new Task();
    m_pIdleTask->owner = this;
    m_pIdleTask->pFiber = nullptr;
    m_pIdleTask->index = 0;
    m_taskQueue.emplace_back(m_pIdleTask);
    auto thread = _beginthreadex(nullptr, 0, &ThreadTaskSystem::ThreadProc, this, 0, &dwThreadId);
    m_hThread = (HANDLE)thread;
    m_threadId = dwThreadId;
}

ThreadTaskSystem::~ThreadTaskSystem() {
    m_bThreadRunning = false;
    WaitForSingleObject(m_hThread, INFINITE);
    CloseHandle(m_hThread);
    delete m_pIdleTask;
}

void ThreadTaskSystem::Join()
{
    while (m_taskQueue.size() != 1)
    {
        SwitchToThread();
    }
}

void ThreadTaskSystem::YieldTask() {
    auto fiberData = GetFiberData();
    if(fiberData == nullptr)
        return;

    Task* pTask = reinterpret_cast<Task*>(fiberData);
    if(pTask->pFiber != GetCurrentFiber())
        return;

    pTask->owner->YieldTask(pTask);
}

bool ThreadTaskSystem::DoAddTask(Binder* binder) {

    std::lock_guard lock(m_mutex);
    Task* newFibertask = new Task();
    newFibertask->owner = this;
    newFibertask->callee = std::unique_ptr<Binder>(std::move(binder));
    newFibertask->pFiber = CreateFiber(0, &ThreadTaskSystem::FiberProc, newFibertask);
    newFibertask->index = m_taskQueue.size();
    if(newFibertask->pFiber == nullptr) {
        delete newFibertask;
        return false;
    }

    m_taskQueue.emplace_back(newFibertask);
    return true;
}

void ThreadTaskSystem::YieldTask(Task* pTask) {
    if (m_taskQueue.size() == 1) {
        m_currentTaskIndex = 0;
    } else {
        m_currentTaskIndex = (m_currentTaskIndex % (m_taskQueue.size() - 1)) + 1;
    }

    void* pFiber = m_taskQueue[m_currentTaskIndex]->pFiber;
    if(pFiber != nullptr)
        SwitchToFiber(pFiber);
}

void ThreadTaskSystem::Run() {
    m_pFiber = ConvertThreadToFiber(m_pIdleTask);
    m_pIdleTask->pFiber = m_pFiber;
    while(m_bThreadRunning) {
        if(m_reservedDeleteFiberIndex != 0) {
            Task* task = nullptr;
            void* pFiber = nullptr;
            {
                std::lock_guard lock(m_mutex);
                task = m_taskQueue[m_reservedDeleteFiberIndex];
                auto itTaskQueue = m_taskQueue.erase(m_taskQueue.begin() + m_reservedDeleteFiberIndex);
                for(auto it = itTaskQueue; it != m_taskQueue.end(); ++it) {
                    (*it)->index -= 1;
                }

                pFiber = task->pFiber;
                delete task;
                m_currentTaskIndex = m_currentTaskIndex % m_taskQueue.size();
                task = m_taskQueue[m_currentTaskIndex];
            }

            DeleteFiber(pFiber);
            m_reservedDeleteFiberIndex = 0;
            SwitchToFiber(task->pFiber);
            continue;
        }

        if(m_taskQueue.size() == 1) {
            SwitchToThread();
            continue;
        }

        YieldTask(nullptr);
    }
}

void ThreadTaskSystem::DeleteTask(Task* pTask) {
    m_reservedDeleteFiberIndex = pTask->index;
    SwitchToFiber(m_pFiber);
}

unsigned WINAPI ThreadTaskSystem::ThreadProc(LPVOID lpParameter) {
    auto pThis = reinterpret_cast<ThreadTaskSystem*>(lpParameter);
    pThis->Run();
    return 0;
}

void WINAPI ThreadTaskSystem::FiberProc(LPVOID lpParameter) {
    Task* pTask = reinterpret_cast<Task*>(lpParameter);
    ThreadTaskSystem* pOwner = pTask->owner;
    pTask->callee->Invoke();
    pOwner->DeleteTask(pTask);
}
