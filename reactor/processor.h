#pragma once

#include <memory>
#include "configparser.h"
#include "threadpool.h"

using namespace utility;

namespace reactor
{
    class Processor
    {
        public:
            // 构造
            Processor(IniConfigParser *parser);
            // 析构
            ~Processor();
            // 添加任务
            template<class F, class... Args>
            auto submit(F &&f, Args&&... args) -> std::future<decltype(f(args...))>
            {
                return p_pool->submit(f, args...);
            }
            // 关闭处理池
            void shutdown();
        private:
            std::unique_ptr<ThreadPool> p_pool;
    };
}

