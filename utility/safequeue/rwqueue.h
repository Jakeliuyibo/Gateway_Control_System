#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>

namespace utility
{
    template <typename T>
    class RWQueue
    {
        public:
            // 构造
            RWQueue() {}
            // 析构
            ~RWQueue() {}
            // 添加
            void enqueue(T &t)
            {
                std::unique_lock<std::mutex> lock(w_lock);
                w_queue.emplace(t);
                r_cv.notify_one();
            }
            void enqueue(T &&t)
            {
                std::unique_lock<std::mutex> lock(w_lock);
                w_queue.emplace(std::move(t));
                r_cv.notify_one();
            }
            // 取出
            bool dequeue(T &t, bool block = false)
            {
                std::unique_lock<std::mutex> rlock(r_lock);

                /* 检测读队列是否为空 */
                if (r_queue.empty())
                {
                    /* 从写队列取出数据存入读队列 */
                    std::unique_lock<std::mutex> wlock(w_lock);
                    int wq_size = w_queue.size();
                    for(int idx = 0; idx < wq_size; idx++)
                    {
                        r_queue.emplace(w_queue.front());
                        w_queue.pop();
                    }
                }

                /* 检测读队列是否仍然为空 */
                if (r_queue.empty()) 
                {
                    if (block) {
                        // 阻塞等待
                        r_cv.wait(rlock, [this] {return !r_queue.empty(); });
                    } else
                    {
                        return false;
                    }
                }

                t = std::move(r_queue.front());
                r_queue.pop();
                return true;
            }
            // 获取队列大小
            int size()
            {
                std::unique_lock<std::mutex> wlock(w_lock);
                std::unique_lock<std::mutex> rlock(r_lock);
                return w_queue.size() + r_queue.size();
            }
        private:
            std::queue<T>   w_queue;        // 写队列
            std::mutex      w_lock;         // 写锁
            std::queue<T>   r_queue;        // 读队列
            std::mutex      r_lock;         // 写锁
            std::condition_variable r_cv;   // 读条件变量
    };
}