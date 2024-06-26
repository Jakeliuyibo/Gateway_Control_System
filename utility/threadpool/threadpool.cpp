#include "threadpool.hpp"

using namespace utility;

/*************************************************************************
 *
 * Public Function
 *
 *************************************************************************/
ThreadPool::ThreadPool(int numThreads) 
    : shutdown_(false) 
{
    for(int idx = 0; idx < numThreads; idx++)
    {
        threadPool_.emplace_back(
            [this]
            {
                for(;;)
                {
                    Task task;
                    {
                        // 加锁并等待通知
                        std::unique_lock<std::mutex> lock(this->lock_);
                        this->cond_.wait(lock, 
                            [this]() {return this->shutdown_ || !this->taskQueue_.empty(); });

                        // 检测线程池是否终止并且任务队列全部执行完成
                        if(this->shutdown_ && this->taskQueue_.empty())
                        {
                            return;
                        }

                        // 取任务
                        task = std::move(this->taskQueue_.front());
                        this->taskQueue_.pop();
                    }

                    // 执行任务
                    task();
                }
            }
        );
    }
}

ThreadPool::~ThreadPool() 
{
    if(!shutdown_)
    {
        Shutdown();
    }
}

void ThreadPool::Shutdown()
{
    // 设置终止标志位
    shutdown_ = true;

    // 通知线程池中线程解锁结束
    cond_.notify_all();

    // 等待线程池执行完成
    for(auto &th : threadPool_)
    {
        if(th.joinable())
        {
            th.join();
        }
    }
}