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
            // 构造和析构
            ThreadPool(int numThreads);
            ThreadPool(const ThreadPool &) = delete;
            ThreadPool(ThreadPool &&) = delete;
            ThreadPool & operator = (const ThreadPool &) = delete;
            ~ThreadPool();

            // 关闭线程池
            void Shutdown();

            // 添加任务
            template<class F, class... Args>
            auto Submit(F &&f, Args&&... args) -> std::future<decltype(f(args...))>
            {
                // 包装任务
                auto ptask = std::make_shared<std::packaged_task<decltype(f(args...))()> >(
                        std::bind(std::forward<F>(f), std::forward<Args>(args)...)
                    );

                // 将任务添加到队列中
                {
                    std::unique_lock<std::mutex> lock(lock_);

                    if(shutdown_)
                    {
                        log_error("Submit task to a stopped threadpool");
                        throw std::runtime_error("Submit task to a stopped threadpool");
                    }

                    taskQueue_.emplace(
                        [ptask] () 
                        {
                            (*ptask)();
                        }
                    );
                }

                // 通知线程执行
                cond_.notify_one();

                return ptask->get_future();
            }

        private:
            std::vector<std::thread> threadPool_;  // 线程池
            std::queue<Task>         taskQueue_;   // 任务队列
            std::mutex               lock_;        // 保护资源锁
            std::condition_variable  cond_;        // 线程同步的条件变量
            std::atomic<bool> shutdown_;           // 线程池是否关闭

    };

}
