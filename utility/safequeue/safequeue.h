#pragma once
#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>
namespace utility
{
    template <typename T>
    class SafeQueue
    {
        public:
            // 构造
            SafeQueue() {}
            // 析构
            ~SafeQueue() {}
            // 添加
            void enqueue(T &t)
            {
                std::unique_lock<std::mutex> lock(m_lock);
                m_queue.emplace(t);
                m_cv.notify_one();
            }
            void enqueue(T &&t)
            {
                std::unique_lock<std::mutex> lock(m_lock);
                m_queue.emplace(std::move(t));
                m_cv.notify_one();
            }
            // 可选阻塞或非阻塞取出
            bool dequeue(T &t, bool block = false)
            {
                std::unique_lock<std::mutex> lock(m_lock); // 添加互斥锁，若失败则堵住
                if (m_queue.empty())
                {
                    if (block) {
                        // 阻塞等待
                        m_cv.wait(lock, [this] { return !m_queue.empty(); });
                    } else {
                        return false;
                    }
                }
                t = std::move(m_queue.front());
                m_queue.pop();
                return true;
            }
            // 是否为空
            bool empty()
            {
                std::unique_lock<std::mutex> lock(m_lock);
                return m_queue.empty();
            }
            // 获取队列大小
            int size()
            {
                std::unique_lock<std::mutex> lock(m_lock);
                return m_queue.size();
            }

        private:
            std::mutex m_lock;
            std::condition_variable m_cv;
            std::queue<T> m_queue;
    };
}