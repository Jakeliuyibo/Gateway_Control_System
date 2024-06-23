#include "threadpool.hpp"

using namespace utility;

// 构造函数
ThreadPool::ThreadPool(int numThreads) 
    : f_shutdown(false) 
{
    for(int idx = 0; idx < numThreads; idx++)
    {
        m_threadpool.emplace_back(
            [this]
            {
                for(;;)
                {
                    Task task;
                    {
                        // 加锁并等待通知
                        std::unique_lock<std::mutex> lock(this->m_lock);
                        this->m_cond.wait(lock, 
                            [this]() {return this->f_shutdown || !this->m_taskqueue.empty(); });

                        // 检测线程池是否终止并且任务队列全部执行完成
                        if(this->f_shutdown && this->m_taskqueue.empty())
                        {
                            return;
                        }

                        // 取任务
                        task = std::move(this->m_taskqueue.front());
                        this->m_taskqueue.pop();
                    }

                    // 执行任务
                    task();
                }
            }
        );
    }
}

// 析构
ThreadPool::~ThreadPool() 
{
    if(!f_shutdown)
    {
        shutdown();
    }
}

// 关闭线程池
void ThreadPool::shutdown()
{
    // 设置终止标志位
    f_shutdown = true;

    // 通知线程池中线程解锁结束
    m_cond.notify_all();

    // 等待线程池执行完成
    for(auto &th : m_threadpool)
    {
        if(th.joinable())
        {
            th.join();
        }
    }
}