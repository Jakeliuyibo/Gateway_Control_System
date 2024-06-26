#include "source.hpp"

using namespace utility;
using namespace reactor;


Source::Source(IniConfigParser *parser)
{
    bool parserFlag = true;

    /* 解析参数 */
    std::string rabbitmqHostName, rabbitmqUser, rabbitmqPassword;
    int rabbitmqPort; 
    parserFlag &= parser->GetValue<std::string>("RABBITMQ", "RABBITMQ_HOSTNAME"         , rabbitmqHostName);
    parserFlag &= parser->GetValue<int        >("RABBITMQ", "RABBITMQ_PORT"             , rabbitmqPort);
    parserFlag &= parser->GetValue<std::string>("RABBITMQ", "RABBITMQ_USER"             , rabbitmqUser);
    parserFlag &= parser->GetValue<std::string>("RABBITMQ", "RABBITMQ_PASSWORD"         , rabbitmqPassword);
    parserFlag &= parser->GetValue<std::string>("RABBITMQ", "RABBITMQ_EXCHANGENAME_IN"  , exchangeNameIn_);
    parserFlag &= parser->GetValue<std::string>("RABBITMQ", "RABBITMQ_QUEUENAME_IN"     , queueNameIn_);
    parserFlag &= parser->GetValue<std::string>("RABBITMQ", "RABBITMQ_ROUTINGKEY_IN"    , routingKeyIn_);
    parserFlag &= parser->GetValue<std::string>("RABBITMQ", "RABBITMQ_EXCHANGENAME_OUT" , exchangeNameOut_);
    parserFlag &= parser->GetValue<std::string>("RABBITMQ", "RABBITMQ_QUEUENAME_OUT"    , queueNameOut_);
    parserFlag &= parser->GetValue<std::string>("RABBITMQ", "RABBITMQ_ROUTINGKEY_OUT"   , routingKeyOut_);

    if(!parserFlag)
    {
        log_error("Init rabbitmq queue failed, when load config");
        return;
    }

    {
        std::unique_lock<std::mutex> wli(wlockIn_);
        std::unique_lock<std::mutex> rli(rlockIn_);
        std::unique_lock<std::mutex> wlo(wlockOut_);

        /* 初始化RabbitMq消息服务的入队列模块 */
        rabbitmqClientIn_ = std::make_unique<RabbitMqClient>(rabbitmqHostName, rabbitmqPort, rabbitmqUser, rabbitmqPassword);
        CExchange exIn(exchangeNameIn_, "direct");
        CQueue quIn(queueNameIn_);
        rabbitmqClientIn_->Connect();
        rabbitmqClientIn_->DeclareExchange(exIn);
        rabbitmqClientIn_->DeclareQueue(quIn);
        rabbitmqClientIn_->BindQueueToExchange(queueNameIn_, exchangeNameIn_, routingKeyIn_);
        rabbitmqClientIn_->ConsumeListen(queueNameIn_);

        /* 初始化RabbitMq消息服务的出队列模块 */
        rabbitmqClientOut_ = std::make_unique<RabbitMqClient>(rabbitmqHostName, rabbitmqPort, rabbitmqUser, rabbitmqPassword);
        CExchange exOut(exchangeNameOut_, "direct");
        CQueue quOut(queueNameOut_);
        rabbitmqClientOut_->Connect();
        rabbitmqClientOut_->DeclareExchange(exOut);
        rabbitmqClientOut_->DeclareQueue(quOut);
        rabbitmqClientOut_->BindQueueToExchange(queueNameOut_, exchangeNameOut_, routingKeyOut_);
    }
}

Source::~Source()
{
    log_info("reactor-event_source module done ...");
}

// 事件入队
void Source::PushIn(std::string msg)
{
    CMessage message(msg);

    {
        std::unique_lock<std::mutex> wlock(wlockIn_);
        rabbitmqClientIn_->Publish(exchangeNameIn_, routingKeyIn_, message);
    }

    log_trace("事件源存入RabbitMq IN队列消息{}", msg);
}

void Source::PushOut(std::string msg)
{
    CMessage message(msg);

    {
        std::unique_lock<std::mutex> wlo(wlockOut_);
        rabbitmqClientOut_->Publish(exchangeNameOut_, routingKeyOut_, message);
    }

    log_trace("事件源存入RabbitMq OUT队列消息{}", msg);
}

// 事件出队
std::string Source::PopIn()
{
    std::string msg;

    {
        std::unique_lock<std::mutex> rli(rlockIn_);
        // msg = p_rabbitmqclient->get(queueNameIn_, true);
        rabbitmqClientIn_->Consume(msg, true);
    }

    log_trace("事件源从RabbitMq IN队列取出消息{}", msg);
    return msg;
}
