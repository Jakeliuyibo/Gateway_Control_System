#pragma once

#include <mutex>
#include <shared_mutex>
#include <map>

namespace utility
{
    template <typename K, typename V>
    class SafeMap
    {
    public:
        using KeyType = K;
        using ValueType = V;

    public:
        // 构造和析构
        SafeMap() {}
        SafeMap(const SafeMap& other) = delete;
        SafeMap& operator = (const SafeMap& other) = delete;
        ~SafeMap() {}

        // 重载括号值访问（引用访问容易造成线程不安全）
        ValueType operator [] (const KeyType& key)
        {
            std::shared_lock<std::shared_mutex> lk(lock_);
            return map_[key];
        }

        // 插入
        bool Insert(const KeyType& key, const ValueType& value)
        {
            std::unique_lock<std::shared_mutex> lk(lock_);
            auto res = map_.insert(std::make_pair(key, value));
            return res.second;
        }

        // 设值
        void Set(const KeyType& key, const ValueType& value)
        {
            std::unique_lock<std::shared_mutex> lk(lock_);
            map_[key] = value;
        }

        // 查找
        bool Find(const KeyType& key)
        {
            std::shared_lock<std::shared_mutex> lk(lock_);
            return map_.find(key) != map_.end();
        }

        // 删除
        bool Erase(const KeyType& key)
        {
            std::unique_lock<std::shared_mutex> lk(lock_);
            return map_.erase(key);
        }

        // 获取大小
        std::size_t Size()
        {
            std::shared_lock<std::shared_mutex> lk(lock_);
            return map_.size();
        }

        // 是否为空
        bool IsEmpty()
        {
            std::shared_lock<std::shared_mutex> lk(lock_);
            return map_.empty();
        }

        // 迭代器操作，可能造成线程不安全，因此删除
        typedef typename std::map<KeyType, ValueType>::iterator Iterator;
        Iterator begin() = delete;
        Iterator end() = delete;
        Iterator rbegin() = delete;
        Iterator rend() = delete;

    private:
        std::shared_mutex lock_; // 读写锁
        std::map<KeyType, ValueType> map_;
    };

}