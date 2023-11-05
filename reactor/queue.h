#pragma once

#include <string>
#include <memory>
#include "configparser.h"
#include "logger.h"
#include "rabbitmqclient.h"

using namespace utility;

namespace reactor
{
    class Queue
    {
        public:
            // 构造
            Queue(IniConfigParser *parser);
            // 析构
            ~Queue();
            // 事件入队
            void push(std::string msg);
            // 事件出队
            std::string pop();
        private:
            std::unique_ptr<RabbitMqClient> m_pQueue;
            std::string m_exchangename;
            std::string m_queuename;
            std::string m_routingkey;
    };

}