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
            // 构造和析构
            SafeMap() {}
            SafeMap(const SafeMap &other) = delete;
            SafeMap & operator = (const SafeMap &other) = delete;
            ~SafeMap() {}

            // 重载括号访问
            V& operator [] (const K& key)
            {
                std::unique_lock<std::shared_mutex> _lck(m_lk);
                return m_mp[key];
            }
            // 插入
            bool insert(const K& key, const V& value)
            {
                std::unique_lock<std::shared_mutex> _lck(m_lk);
                auto res = m_mp.insert(std::make_pair(key, value));
                return res.second;
            }
            // 查找
            bool find(const K& key)
            {
                std::shared_lock<std::shared_mutex> _lck(m_lk);
                return m_mp.find(key) != m_mp.end();
            }
            // 删除
            bool erase(const K& key)
            {
                std::unique_lock<std::shared_mutex> _lck(m_lk);
                return m_mp.erase(key);
            }
            // 获取大小
            std::size_t size()
            {
                std::shared_lock<std::shared_mutex> _lck(m_lk);
                return m_mp.size();
            }
            // 是否为空
            bool empty()
            {
                std::shared_lock<std::shared_mutex> _lck(m_lk);
                return m_mp.empty();
            }
        private:
            std::shared_mutex m_lk; // 读写锁
            std::map<K, V> m_mp;
    };
}