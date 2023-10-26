#pragma once

#include <iostream>
#include <string>
#include <vector>
#include "rabbitmq-c/amqp.h"
#include "rabbitmq-c/tcp_socket.h"

using namespace std;

namespace utility
{
    class CExchange
    {
        public:
            CExchange(const string &name, const string &type, bool passive=false, bool durable=true, bool internal=false, bool autodelete=false) :
                m_name(name),
                m_type(type),
                m_passive(passive),
                m_durable(durable),
                m_internal(internal),
                m_autodelete(autodelete) {}
            ~CExchange() {}
        public:
            string m_name;          // 交换机名称
            string m_type;          // 交换机类型：direct、topic、fanout、
            bool   m_passive;       // 检测交换机是否已存在。设为true时，不存在则不会创建；设为false时，不存在则创建
            bool   m_durable;       // 交换机内数据是否持久化
            bool   m_internal;      // 
            bool   m_autodelete;    // 没有队列与该交换机绑定时是否自动删除
    };

    class CQueue
    {
        public:
            CQueue(const string &name, bool passive=false, bool durable=true, bool exclusive=false, bool autodelete=false) :
                m_name(name),
                m_passive(passive),
                m_durable(durable),
                m_exclusive(exclusive),
                m_autodelete(autodelete) {}
            ~CQueue() {}
        public:
            string m_name;          // 队列名称
            bool   m_passive;       // 检测队列是否存在
            bool   m_durable;       // 队列中的消息是否持久化
            bool   m_exclusive;     // 是否声明为排他队列。设为true时，仅对首次声明他的连接可见，并在断开连接后自动删除
            bool   m_autodelete;    // 没有交换机与该队列绑定时是否自动删除
    };

    class CMessage
    {
        public:
            CMessage(const string &data, bool mandatory=true, bool immediate=false, bool is_durable=true) :
                m_data(data),
                m_mandatory(mandatory),
                m_immediate(immediate)
            {
                if(is_durable)
                {
                    m_properties._flags |= AMQP_BASIC_DELIVERY_MODE_FLAG;
                    m_properties.delivery_mode = AMQP_DELIVERY_PERSISTENT;
                }
            }
            ~CMessage() {}
        public:
            string m_data;          // 数据
            bool   m_mandatory;     // 消息必须路由到存在的队列
            bool   m_immediate;     // 立即发送到消费者
            amqp_basic_properties_t m_properties{0};     // 属性
    };

    class RabbitMqClient
    {
        public:
            RabbitMqClient(const string &hostname, int port, const string &user, const string &password);
            ~RabbitMqClient();
        public:
            // 连接服务器
            int connect();
            // 断开连接
            int disconnect();  
            // 初始化交换机
            int declareExchange(CExchange &exchange);
            // 初始化队列
            int declareQueue(CQueue &queue);
            // 将指定队列绑定到交换机上
            int bindQueueToExchange(const string &queue, const string &exchange, const string &bindkey);
            // 取消队列到交换机的绑定
            int unbingQueueToExchange(CQueue &queue, CExchange &exchange, const string &bindkey);
            // 发布消息
            int publish(const string &exchange_name, const string &routing_key_name, const CMessage &message);
            // 同步方式消费一条消息
            string consume(const string &queue_name, bool no_ack=true);
            // 同步方式消费多条消息
            vector<string> consume(const string &queue_name, int num, bool no_ack=true);
            // 非阻塞方式消费一条消息
            string consume(const string &queue_name, struct timeval *timeout, bool no_ack=true);
            // 非阻塞方式消费多条消息
            vector<string> consume(const string &queue_name, int num, struct timeval *timeout, bool no_ack=true);

        private:
            // 处理错误信息
            int errorMsg(const amqp_rpc_reply_t &reply, const string & desc);

        private:
            string m_hostname;              // 主机
            int m_port;                     // 端口
            string m_username;              // 用户名
            string m_password;              // 密码
            amqp_connection_state_t m_conn; // AMQP连接
            int m_channel;                  // 通道
    };

    // 获取RabbitMq版本
    void showRabbitMqVersion();
}



