#pragma once

#include <queue>
#include <mutex>

namespace utility
{
    template<typename T>
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
            }
            void enqueue(T &&t)
            {
                std::unique_lock<std::mutex> lock(m_lock);
                m_queue.emplace(std::move(t));
            }
            // 取出
            bool dequeue(T &t)
            {
                std::unique_lock<std::mutex> lock(m_lock);

                /* 检测队列是否为空 */
                if(m_queue.empty())
                {
                    return false;
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
            std::queue<T> m_queue;
    };
}