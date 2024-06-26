#pragma once
#include <iostream>
#include <string>
#include <memory>
#include <mutex>

#include "configparser.hpp"
#include "logger.hpp"
#include "rabbitmqclient.hpp"

using namespace utility;

namespace reactor
{
    class Source
    {
        public:
            // 构造和析构
            Source(IniConfigParser *parser);
            ~Source();

            // 事件入队
            void PushIn(std::string msg);
            void PushOut(std::string msg);

            // 事件出队
            std::string PopIn();

        private:
            // 入队列相关
            std::unique_ptr<RabbitMqClient> rabbitmqClientIn_;
            std::mutex  wlockIn_, rlockIn_;
            std::string exchangeNameIn_;
            std::string queueNameIn_;
            std::string routingKeyIn_;

            // 出队列相关
            std::unique_ptr<RabbitMqClient> rabbitmqClientOut_;
            std::mutex  wlockOut_;
            std::string exchangeNameOut_;
            std::string queueNameOut_;
            std::string routingKeyOut_;
    };

}