#include "source.h"

using namespace utility;
using namespace reactor;


Source::Source(IniConfigParser *parser)
{
    bool parserFlag = true;

    /* 解析参数 */
    std::string rabbitmq_hostname, rabbitmq_user, rabbitmq_password;
    int rabbitmq_port; 
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_HOSTNAME"         , rabbitmq_hostname);
    parserFlag &= parser->getValue<int        >("RABBITMQ", "RABBITMQ_PORT"             , rabbitmq_port);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_USER"             , rabbitmq_user);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_PASSWORD"         , rabbitmq_password);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_EXCHANGENAME_IN"  , m_exchangename_in);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_QUEUENAME_IN"     , m_queuename_in);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_ROUTINGKEY_IN"    , m_routingkey_in);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_EXCHANGENAME_OUT" , m_exchangename_out);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_QUEUENAME_OUT"    , m_queuename_out);
    parserFlag &= parser->getValue<std::string>("RABBITMQ", "RABBITMQ_ROUTINGKEY_OUT"   , m_routingkey_out);

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
        CExchange ex_in(m_exchangename_in, "direct");
        CExchange ex_out(m_exchangename_out, "direct");
        CQueue qu_in(m_queuename_in);
        CQueue qu_out(m_queuename_out);

        p_rabbitmqclient->connect();
        p_rabbitmqclient->declareExchange(ex_in);
        p_rabbitmqclient->declareQueue(qu_in);
        p_rabbitmqclient->bindQueueToExchange(m_queuename_in, m_exchangename_in, m_routingkey_in);
        p_rabbitmqclient->declareExchange(ex_out);
        p_rabbitmqclient->declareQueue(qu_out);
        p_rabbitmqclient->bindQueueToExchange(m_queuename_out, m_exchangename_out, m_routingkey_out);

        p_rabbitmqclient->consume_listen(m_queuename_in);
    }
}

Source::~Source()
{
    log_info("reactor-event_source module done ...");
}

// 事件入队
void Source::push_in(std::string msg)
{
    CMessage message(msg);

    {
        std::unique_lock<std::mutex> rlock(m_rlock);
        p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, message);
    }

    log_trace("事件源存入RabbitMq IN队列消息{}", msg);
}

void Source::push_out(std::string msg)
{
    CMessage message(msg);

    {
        std::unique_lock<std::mutex> rlock(m_rlock);
        p_rabbitmqclient->publish(m_exchangename_out, m_routingkey_out, message);
    }

    log_trace("事件源存入RabbitMq OUT队列消息{}", msg);
}

// 事件出队
std::string Source::pop_in()
{
    std::string msg;

    {
        std::unique_lock<std::mutex> wlock(m_wlock);
        // msg = p_rabbitmqclient->get(m_queuename_in, true);
        p_rabbitmqclient->consume(msg, true);
    }

    log_trace("事件源从RabbitMq IN队列取出消息{}", msg);
    return msg;
}
