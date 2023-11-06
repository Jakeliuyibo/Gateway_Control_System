#pragma once

#include <string>
#include <memory>
#include <mutex>
#include "configparser.h"
#include "logger.h"
#include "rabbitmqclient.h"

using namespace utility;

namespace reactor
{
    class Source
    {
        public:
            // 构造
            Source(IniConfigParser *parser);
            // 析构
            ~Source();
            // 事件入队
            void push(std::string msg);
            // 事件出队
            std::string pop();
        private:
            std::unique_ptr<RabbitMqClient> p_rabbitmqclient;
            std::mutex  m_wlock, m_rlock;
            std::string m_exchangename;
            std::string m_queuename;
            std::string m_routingkey;
    };

}