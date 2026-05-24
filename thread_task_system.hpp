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
#include <memory>
#include <tuple>
#include <vector>
#include <type_traits>

class ThreadTaskSystem {
    struct Binder {
        virtual ~Binder() = default;
        virtual void Invoke() = 0;
    };

    struct Task {
        ThreadTaskSystem* owner;
        void* pFiber;
        std::unique_ptr<Binder> callee;
        uint32_t index;
    };

    template<typename TCallable, typename... TArgs>
    struct TBinder : Binder {
        std::decay_t<TCallable> callee;
        std::tuple<std::decay_t<TArgs>...> args;

        TBinder(TCallable&& callee, TArgs&&... args)
            : callee(std::forward<TCallable>(callee)),
              args(std::forward<TArgs>(args)...) {}

        void Invoke() override {
            std::apply(std::move(callee), std::move(args));
        }
    };

public:
    ThreadTaskSystem();

    ThreadTaskSystem(const ThreadTaskSystem&) = delete;

    ThreadTaskSystem& operator=(const ThreadTaskSystem&) = delete;

    ~ThreadTaskSystem();

    template<typename TCallable, typename... TArgs>
    bool AddTask(TCallable&& callee, TArgs&&... args) {
        return DoAddTask(new TBinder<TCallable, TArgs...>(
            std::forward<TCallable>(callee),
            std::forward<TArgs>(args)...
        ));
    }

    void Join();

    static void YieldTask();

private:
    bool DoAddTask(Binder* binder);

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
