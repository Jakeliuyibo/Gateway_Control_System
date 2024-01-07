#pragma once
#include <iostream>
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
            void push_in(std::string msg);
            void push_out(std::string msg);
            // 事件出队
            std::string pop_in();
        private:
            // 入队列相关
            std::unique_ptr<RabbitMqClient> p_rabbitmqclient_in;
            std::mutex  m_wlock_in, m_rlock_in;
            std::string m_exchangename_in;
            std::string m_queuename_in;
            std::string m_routingkey_in;
            // 出队列相关
            std::unique_ptr<RabbitMqClient> p_rabbitmqclient_out;
            std::mutex  m_wlock_out;
            std::string m_exchangename_out;
            std::string m_queuename_out;
            std::string m_routingkey_out;
    };

}