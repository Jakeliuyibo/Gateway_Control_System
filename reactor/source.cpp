#include <iostream>
#include "source.h"

using namespace utility;
using namespace reactor;


Source::Source(IniConfigParser *parser)
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

    if(!parserFlag)
    {
        log_error("Init rabbitmq queue failed, when load config");
        return;
    }

    {
        std::unique_lock<std::mutex> wlock(m_wlock);
        std::unique_lock<std::mutex> rlock(m_rlock);

        /* 初始化RabbitMq Client模块 */
        p_rabbitmqclient = std::make_unique<RabbitMqClient>(rabbitmq_hostname, rabbitmq_port, rabbitmq_user, rabbitmq_password);
        CExchange ex(m_exchangename, "direct");
        CQueue qu(m_queuename);

        p_rabbitmqclient->connect();
        p_rabbitmqclient->declareExchange(ex);
        p_rabbitmqclient->declareQueue(qu);
        p_rabbitmqclient->bindQueueToExchange(m_queuename, m_exchangename, m_routingkey);
    }
}

Source::~Source()
{
    log_info("reactor-event_source module done ...");
}

// 事件入队
void Source::push(std::string msg)
{
    CMessage message(msg);

    {
        std::unique_lock<std::mutex> lock(m_wlock);
        p_rabbitmqclient->publish(m_exchangename, m_routingkey, message);
    }

    log_trace("Source存入RabbitMq client队列消息{}", msg);
}

// 事件出队
std::string Source::pop()
{
    std::string msg;

    {
        std::unique_lock<std::mutex> lock(m_rlock);
        msg = p_rabbitmqclient->consume_b(m_queuename, NULL, true);
    }

    log_trace("Source从RabbitMq client队列取出消息{}", msg);
    return msg;
}
