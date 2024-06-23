#pragma once

#include <vector>
#include <queue>
#include <atomic>
#include <functional>
#include <future>
#include <condition_variable>
#include <mutex>
#include <list>
#include <memory>
#include <thread>
#include <stdexcept>

#include "logger.hpp"

namespace utility
{
    class ThreadPool
    {
        using Task = std::function<void()>;

        public:
            // 构造
            ThreadPool(int numThreads);
            ThreadPool(const ThreadPool &) = delete;
            ThreadPool(ThreadPool &&) = delete;
            ThreadPool & operator = (const ThreadPool &) = delete;

            // 析构
            ~ThreadPool();
            // 关闭线程池
            void shutdown();
            // 添加任务
            template<class F, class... Args>
            auto submit(F &&f, Args&&... args) -> std::future<decltype(f(args...))>
            {
                // 包装任务
                auto ptask = std::make_shared<std::packaged_task<decltype(f(args...))()> >(
                        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );

                // 将任务添加到队列中
                {
                    std::unique_lock<std::mutex> lock(m_lock);

                    if(f_shutdown)
                    {
                        log_error("submit task to a stopped threadpool");
                        throw std::runtime_error("submit task to a stopped threadpool");
                    }

                    m_taskqueue.emplace(
                        [ptask] () 
                        {
                            (*ptask)();
                        }
                    );
                }

                // 通知线程执行
                m_cond.notify_one();

                return ptask->get_future();
            }

        private:
            std::vector<std::thread> m_threadpool;  // 线程池
            std::queue<Task>         m_taskqueue;   // 任务队列
            std::mutex               m_lock;        // 保护资源锁
            std::condition_variable  m_cond;        // 线程同步的条件变量

            std::atomic<bool> f_shutdown;               // 线程池是否关闭

    };

}
