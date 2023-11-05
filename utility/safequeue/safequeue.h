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
                m_queue.emplace(&t);
            }
            // 取出
            bool dequeue(T &t)
            {
                std::unique_lock<std::mutex> lock(m_lock);

                /* 检测队列是否为空 */
                if(empty())
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
            // 大小
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