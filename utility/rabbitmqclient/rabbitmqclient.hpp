#pragma once

#include <iostream>
#include <string>
#include <vector>
#include <mutex>
#include <exception>
#include <condition_variable>

#include "amqp.h"
#include "amqp_tcp_socket.h"

#include "safequeue.hpp"

namespace utility
{
    class CExchange
    {
    public:
        CExchange(const std::string& name, const std::string& type, bool passive = false, bool durable = true, bool internal = false, bool autodelete = false)
            : name_(name),
            type_(type),
            passive_(passive),
            durable_(durable),
            internal_(internal),
            autodelete_(autodelete) {}
        ~CExchange() {}
    public:
        std::string name_;     // 交换机名称
        std::string type_;     // 交换机类型：direct、topic、fanout、
        bool   passive_;       // 检测交换机是否已存在。设为true时，不存在则不会创建；设为false时，不存在则创建
        bool   durable_;       // 交换机内数据是否持久化
        bool   internal_;      // 
        bool   autodelete_;    // 没有队列与该交换机绑定时是否自动删除
    };

    class CQueue
    {
    public:
        CQueue(const std::string& name, bool passive = false, bool durable = true, bool exclusive = false, bool autodelete = false) :
            name_(name),
            passive_(passive),
            durable_(durable),
            exclusive_(exclusive),
            autodelete_(autodelete) {}
        ~CQueue() {}
    public:
        std::string name_;     // 队列名称
        bool   passive_;       // 检测队列是否存在
        bool   durable_;       // 队列中的消息是否持久化
        bool   exclusive_;     // 是否声明为排他队列。设为true时，仅对首次声明他的连接可见，并在断开连接后自动删除
        bool   autodelete_;    // 没有交换机与该队列绑定时是否自动删除
    };

    class CMessage
    {
    public:
        CMessage(const std::string& data, bool mandatory = true, bool immediate = false, bool isDurable = true) :
            data_(data),
            mandatory_(mandatory),
            immediate_(immediate)
        {
            if (isDurable)
            {
                properties_._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;
                properties_.delivery_mode = AMQP_DELIVERY_PERSISTENT;
            }
        }
        ~CMessage() {}
    public:
        std::string data_;     // 数据
        bool   mandatory_;     // 消息必须路由到存在的队列
        bool   immediate_;     // 立即发送到消费者
        amqp_basic_properties_t properties_{ 0 };     // 属性
    };

    class RabbitMqClient
    {
    public:
        RabbitMqClient(const std::string& hostName, int port, const std::string& user = "guest", const std::string& password = "guest");
        ~RabbitMqClient();

    public:
        // 连接服务器
        int Connect();
        // 断开连接
        int Disconnect();
        // 初始化交换机
        int DeclareExchange(CExchange& exchange);
        // 初始化队列
        int DeclareQueue(CQueue& queue);
        // 将指定队列绑定到交换机上
        int BindQueueToExchange(const std::string& queue, const std::string& exchange, const std::string& bindKey);
        // 取消队列到交换机的绑定
        int UnbingQueueToExchange(CQueue& queue, CExchange& exchange, const std::string& bindKey);
        // 发布消息
        int Publish(const std::string& exchangeName, const std::string& routingKeyName, const CMessage& message);

        // 底层基于get方式，主动向服务器拉取
        std::string Get(const std::string& queueName, bool noAck = true);
        std::vector<std::string> Get(const std::string& queueName, int num, bool noAck = true);

        // 底层基于consume方式，由服务器主动推送
        void ConsumeListen(const std::string& queueName, struct timeval* timeout = nullptr, bool noAck = true);
        bool Consume(std::string& msg, bool block = true);

    private:
        // 处理错误信息
        int ErrorMsg(const amqp_rpc_reply_t& reply, const std::string& desc);

    private:
        std::string hostName_;              // 主机
        int port_;                          // 端口
        std::string userName_;              // 用户名
        std::string password_;              // 密码
        int channel_;                       // 通道
        amqp_connection_state_t conn_;      // AMQP连接

        /* 客户端接收的资源 */
        static constexpr int kPrefetchCount = 1000;     // 预取的消息个数
        SafeQueue<std::string>  store_;
    };

    // 获取RabbitMq版本
    void ShowRabbitMqVersion();
}



