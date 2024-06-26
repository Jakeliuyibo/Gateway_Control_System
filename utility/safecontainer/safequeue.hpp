#pragma once

#include <iostream>
#include <queue>
#include <mutex>
#include <condition_variable>

namespace utility
{
    template <typename V>
    class SafeQueue
    {
    public:
        using ValueType = V;

        public:
            // 构造和析构
            SafeQueue() {}
            ~SafeQueue() {}

            // 添加
            void Enqueue(ValueType &t)
            {
                std::unique_lock<std::mutex> lk(lock_);
                queue_.emplace(t);
                cv_.notify_one();
            }

            void Enqueue(ValueType &&t)
            {
                std::unique_lock<std::mutex> lk(lock_);
                queue_.emplace(std::move(t));
                cv_.notify_one();
            }

            // 可选阻塞或非阻塞取出
            bool Dequeue(ValueType &t, bool block = false)
            {
                std::unique_lock<std::mutex> lk(lock_);
                if (queue_.empty())
                {
                    if (block) {
                        // 阻塞等待
                        cv_.wait(lk, [this] { return !queue_.empty(); });
                    } else {
                        return false;
                    }
                }
                t = std::move(queue_.front());
                queue_.pop();
                return true;
            }

            // 非阻塞方式取出所有数据到容器
            bool DequeueAllIntoVec(std::vector<ValueType> & vec)
            {
                std::unique_lock<std::mutex> lk(lock_);
                while (!queue_.empty())
                {
                    vec.emplace_back(std::move(queue_.front()));
                    queue_.pop();
                }
                return !vec.empty();
            }

            // 是否为空
            bool Empty()
            {
                std::unique_lock<std::mutex> lock(lock_);
                return queue_.empty();
            }

            // 获取队列大小
            int Size()
            {
                std::unique_lock<std::mutex> lock(lock_);
                return queue_.size();
            }

        private:
            std::mutex lock_;
            std::condition_variable cv_;
            std::queue<ValueType> queue_;
    };
}