#pragma once

#include <memory>

#include "configparser.hpp"
#include "threadpool.hpp"

using namespace utility;

namespace reactor
{
    class Processor
    {
        public:
            // 构造和析构
            Processor(IniConfigParser *parser);
            ~Processor();

            // 添加任务
            template<class F, class... Args>
            auto Submit(F &&f, Args&&... args) -> std::future<decltype(f(args...))>
            {
                return pool_->Submit(f, args...);
            }
            
            // 关闭处理池
            void Shutdown();
            
        private:
            std::unique_ptr<ThreadPool> pool_;
    };
}

