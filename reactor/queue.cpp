#include <iostream>
#include "queue.h"

using namespace utility;
using namespace reactor;


Queue::Queue(IniConfigParser *parser)
{
    bool parserFlag = true;

    /* 解析参数 */
    std::string rabbitmq_hostname, rabbitmq_user, rabbitmq_password;
    int rabbitmq_port; 
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_HOSTNAME"    , rabbitmq_hostname);
    parserFlag &= parser->getValue<int        >("RABBITMQ", "RABBITMQ_PORT"        , rabbitmq_port);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_USER"        , rabbitmq_user);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_PASSWORD"    , rabbitmq_password);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_EXCHANGENAME", m_exchangename);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_QUEUENAME"   , m_queuename);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_ROUTINGKEY"  , m_routingkey);

    /* 初始化RabbitMq Client模块 */
    m_pQueue = std::make_unique<RabbitMqClient>(rabbitmq_hostname, rabbitmq_port, rabbitmq_user, rabbitmq_password);
    CExchange ex(m_exchangename, "direct");
    CQueue qu(m_queuename);

    m_pQueue->connect();
    m_pQueue->declareExchange(ex);
    m_pQueue->declareQueue(qu);
    m_pQueue->bindQueueToExchange(m_queuename, m_exchangename, m_routingkey);

}

Queue::~Queue()
{

}

// 事件入队
void Queue::push(std::string msg)
{
    CMessage message(msg);
    m_pQueue->publish(m_exchangename, m_routingkey, message);

    info("RabbitMq client push msg, {}", msg);
}

// 事件出队
std::string Queue::pop()
{
    std::string msg = m_pQueue->consume(m_queuename);

    info("RabbitMq client pop msg, {}", msg);

    return msg;
}
