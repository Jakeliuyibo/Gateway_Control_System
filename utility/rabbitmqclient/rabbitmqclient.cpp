#include "rabbitmqclient.hpp"

#include "logger.hpp"

using namespace utility;

/*************************************************************************
 *
 * Public Function
 *
 *************************************************************************/
 /**
  * @description: 构造函数
  */
RabbitMqClient::RabbitMqClient(const std::string& hostName, int port, const std::string& user, const std::string& password)
    : hostName_(hostName),
    port_(port),
    userName_(user),
    password_(password),
    conn_(nullptr),
    channel_(1)
{
}

/**
 * @description: 析构函数
 */
RabbitMqClient::~RabbitMqClient()
{

}

/**
 * @description: 建立连接
 */
int RabbitMqClient::Connect()
{
    /* 检测参数 */
    if (hostName_.empty() || port_ <= 0 || userName_.empty() || password_.empty())
    {
        log_error("RabbitMqClient建立连接时参数错误");
        return -1;
    }

    /* 1、分配新连接指针 */
    conn_ = amqp_new_connection();
    if (conn_ == nullptr)
    {
        log_error("RabbitMqClient分配连接器失败");
        return -2;
    }

    /* 2、创建socket */
    amqp_socket_t* sock = amqp_tcp_socket_new(conn_);
    if (sock == nullptr)
    {
        log_error("RabbitMqClient分配网络端口失败");
        return -3;
    }

    /* 3、绑定主机地址和端口，建立与服务器连接 */
    int isOpenSocket = amqp_socket_open(sock, hostName_.c_str(), port_);
    if (isOpenSocket < 0)
    {
        log_error("RabbitMqClient绑定ip和端口失败");
        return -4;
    }

    /* 4、登录到RabbitMq服务器 */
    int isLogin = ErrorMsg(amqp_login(conn_,
        "/",
        1,
        AMQP_DEFAULT_FRAME_SIZE,
        AMQP_DEFAULT_HEARTBEAT,
        AMQP_SASL_METHOD_PLAIN,
        userName_.c_str(),
        password_.c_str()),
        "建立连接");
    if (isLogin < 0)
    {
        log_error("RabbitMqClient登录服务器失败");
        return -5;
    }

    /* 6、打开通道 */
    amqp_channel_open_ok_t* isOpenChannel = amqp_channel_open(conn_, channel_);
    if (!isOpenChannel)
    {
        log_error("RabbitMqClient打开通道失败");
        return -6;
    }
    return 0;
}

/**
 * @description: 断开连接
 */
int RabbitMqClient::Disconnect()
{
    int ret = 0;
    if (conn_ != nullptr)
    {
        /* 1、关闭通道 */
        ret = ErrorMsg(amqp_channel_close(conn_, channel_, AMQP_REPLY_SUCCESS), "关闭通道");
        if (ret < 0)
        {
            log_error("RabbitMqClient关闭通道失败");
            return ret;
        }

        /* 2、关闭连接 */
        ret = ErrorMsg(amqp_connection_close(conn_, AMQP_REPLY_SUCCESS), "关闭连接");
        if (ret < 0)
        {
            log_error("RabbitMqClient关闭连接失败");
            return ret;
        }

        /* 3、销毁连接 */
        ret = amqp_destroy_connection(conn_);
        if (ret < 0)
        {
            log_error("RabbitMqClient注销连接器");
            return ret;
        }

        conn_ = nullptr;
    }
    else
    {
        log_warning("RabbitMqClient尝试打开一个不存在的连接");
        return -4;
    }

    return 0;
}

/**
 * @description: 初始化交换机
 */
int RabbitMqClient::DeclareExchange(CExchange& exchange)
{
    /* 声明交换机 */
    amqp_exchange_declare(conn_,
        channel_,
        amqp_cstring_bytes(exchange.name_.c_str()),
        amqp_cstring_bytes(exchange.type_.c_str()),
        exchange.passive_,
        exchange.durable_,
        exchange.autodelete_,
        exchange.internal_,
        amqp_empty_table);

    return ErrorMsg(amqp_get_rpc_reply(conn_), "声明交换器");
}

/**
 * @description: 初始化队列
 */
int RabbitMqClient::DeclareQueue(CQueue& queue)
{
    /* 声明队列 */
    amqp_queue_declare(conn_,
        channel_,
        amqp_cstring_bytes(queue.name_.c_str()),
        queue.passive_,
        queue.durable_,
        queue.exclusive_,
        queue.autodelete_,
        amqp_empty_table);

    return ErrorMsg(amqp_get_rpc_reply(conn_), "声明队列");
}

/**
 * @description: 将指定队列绑定到交换机上，在direct模式下bindkey可以为队列名称
 */
int RabbitMqClient::BindQueueToExchange(const std::string& queue, const std::string& exchange, const std::string& bindKey)
{
    amqp_queue_bind(conn_,
        channel_,
        amqp_cstring_bytes(queue.c_str()),
        amqp_cstring_bytes(exchange.c_str()),
        amqp_cstring_bytes(bindKey.c_str()),
        amqp_empty_table);

    return ErrorMsg(amqp_get_rpc_reply(conn_), "绑定队列到交换机");
}

/**
 * @description: 发布消息
 */
int RabbitMqClient::Publish(const std::string& exchangeName, const std::string& routingKeyName, const CMessage& message)
{
    int ret = amqp_basic_publish(conn_,
        channel_,
        amqp_cstring_bytes(exchangeName.c_str()),
        amqp_cstring_bytes(routingKeyName.c_str()),
        message.mandatory_,
        message.immediate_,
        &message.properties_,
        amqp_cstring_bytes(message.data_.c_str()));

    if (ret != AMQP_STATUS_OK)
    {
        log_error("RabbitMq客户端发布消息出错");
        return ErrorMsg(amqp_get_rpc_reply(conn_), "发布消息");
    }

    return 0;
}

/**
 * @description: 非阻塞方式消费，底层以amqp的get和read方法实现，每次主动向服务器拉取一条消息
 */
std::string RabbitMqClient::Get(const std::string& queueName, bool noAck)
{
    std::vector<std::string> vecMsg = Get(queueName, 1, noAck);

    if (vecMsg.size() != 1)
    {
        log_warning("尝试以非阻塞读取一条消息,但是获得了NULL或多个");
        return "";
    }

    return vecMsg[0];
}

std::vector<std::string> RabbitMqClient::Get(const std::string& queueName, int num, bool noAck)
{
    std::vector<std::string> retMsg;

    while (num--)
    {
        /*  1、阻塞同步轮询服务器中队列 */
        amqp_rpc_reply_t replyGet = amqp_basic_get(conn_, channel_, amqp_cstring_bytes(queueName.c_str()), noAck);
        int retGetMsg = ErrorMsg(replyGet, "Get message");
        if (retGetMsg < 0)
        {
            log_error("Failed to get message from RabbitMQ server");
            return retMsg;
        }
        // 获取队列中存在多少条消息
        amqp_basic_get_ok_t* tip;
        switch (replyGet.reply.id)
        {
            case AMQP_BASIC_GET_OK_METHOD:
                tip = ( amqp_basic_get_ok_t* ) replyGet.reply.decoded;
                // info("rabbitmq queue remaining %d messages", tip->message_count);
                break;
            case AMQP_BASIC_GET_EMPTY_METHOD:
                log_info("no message in rabbitmq queue");
                return retMsg;
            default:
                log_error("get error rabbitmq reply id %d", replyGet.reply.id);
                return retMsg;
        }

        /* 2、读取chennal上的一条消息 */
        amqp_message_t amqpMsg;
        int retReadMsg = ErrorMsg(amqp_read_message(conn_, channel_, &amqpMsg, false), "Read message");
        if (retReadMsg < 0)
        {
            log_error("Failed to read rabbitmq message");
            return retMsg;
        }

        /* 3、封装消息 */
        retMsg.emplace_back(std::string(( char* ) amqpMsg.body.bytes, ( char* ) amqpMsg.body.bytes + amqpMsg.body.len));

        /* 4、应答ACK */
        if (noAck == false)
        {
            amqp_basic_ack(conn_, channel_, tip->delivery_tag, false);
        }
    }

    return retMsg;
}

/**
 * @description: 阻塞方式消费，底层为consume实现，本地被动一次性拉取服务器所有消息，依次由客户端消费
 */
void RabbitMqClient::ConsumeListen(const std::string& queueName, struct timeval* timeout, bool noAck)
{
    std::thread(
        [this, queueName, timeout, noAck]()
        {
            try
            {
                // /* 1、设置通道消费的限制 */
                // amqp_basic_qos_ok_t *retQosOk = amqp_basic_qos(conn_,
                //                                                channel_,
                //                                                0,               // 预取消息的字节数prefetch_size 0：不限制大小
                //                                                kPrefetchCount,  // 预取消息的数量kPrefetchCount
                //                                                false);          // 是否将预取条件应用到整个通道 0：不应用
                // if (!retQosOk)
                // {
                //     errorMsg(amqp_get_rpc_reply(m_conn), "Set consumer limit(qos)");
                //     throw std::runtime_error("Basic qoe");
                // }

                /* 2、创建消费者 */
                amqp_basic_consume_ok_t* retBasicConsume = amqp_basic_consume(conn_,
                    channel_,
                    amqp_cstring_bytes(queueName.c_str()),
                    amqp_empty_bytes,
                    false,  // no_local 0:接收 1:不接收
                    noAck, // noAck 是否需要ack才将该消息从队列删除 0:需要调用amqp_basic_ack后才会清除 1:不回复
                    false,  // exclusive 0:不独占 1:当前连接不在时队列自动删除
                    amqp_empty_table);
                if (!retBasicConsume)
                {
                    ErrorMsg(amqp_get_rpc_reply(conn_), "Consumer basic");
                    throw std::runtime_error("Consumer basic");
                }

                for (;;)
                {
                    // amqp_maybe_release_buffers(conn_);

                    /* 3、消费 */
                    amqp_envelope_t envelope;
                    int isConsume = ErrorMsg(amqp_consume_message(conn_, &envelope, timeout, 0), "Consume message");
                    if (isConsume < 0)
                    {
                        log_error("Faild to consume message from rabbitmq server");
                        throw std::runtime_error("Consume message");
                    }

                    /* 4、封装消息 */
                    store_.Enqueue(std::string(( char* ) envelope.message.body.bytes, ( char* ) envelope.message.body.bytes + envelope.message.body.len));

                    /* 5、应答ACK */
                    if (noAck == false)
                    {
                        amqp_basic_ack(conn_, channel_, envelope.delivery_tag, false);
                    }

                    /* 6、删除封装容器 */
                    amqp_destroy_envelope(&envelope);
                }
            }
            catch (const std::exception& e)
            {
                log_error("RabbitMqClient消费异常, msg = {}", e.what());
            }
        }
    ).detach();
}

bool RabbitMqClient::Consume(std::string& msg, bool block)
{
    return store_.Dequeue(msg, true);
}

/*************************************************************************
 *
 * Private Function
 *
 *************************************************************************/
 /**
  * @description: 处理错误信息
  */
int RabbitMqClient::ErrorMsg(const amqp_rpc_reply_t& reply, const std::string& desc)
{
    amqp_connection_close_t* de;
    switch (reply.reply_type)
    {
        case AMQP_RESPONSE_NORMAL:
            return 0;

        case AMQP_RESPONSE_NONE:
            log_error("RabbitMQ{}时,发生Response None错误", desc.c_str());
            break;

        case AMQP_RESPONSE_LIBRARY_EXCEPTION:
            log_error("RabbitMQ{}时,发生Response Library错误", desc.c_str());
            break;

        case AMQP_RESPONSE_SERVER_EXCEPTION:
            // switch(reply.reply.id)
            // {
            //     case AMQP_CONNECTION_CLOSE_METHOD:
            //         de = (amqp_connection_close_t *)reply.reply.decoded;
            //         log_error("RabbitMQ{}时,发生Response Server的CONNECTION CLOSE错误({}), msg = ", desc.c_str(), de->reply_code, std::string((char *)de->reply_text.bytes, de->reply_text.len));
            //         break;
            //     case AMQP_CHANNEL_CLOSE_METHOD:
            //         de = (amqp_connection_close_t *)reply.reply.decoded;
            //         log_error("RabbitMQ{}时,发生Response Server的CHANNEL CLOSE错误({}), msg = ", desc.c_str(), de->reply_code, std::string((char *)de->reply_text.bytes, de->reply_text.len));
            //         break;
            //     default:
            //         log_error("RabbitMQ{}时,发生未知的Response Server错误", desc.c_str());
            //         break;
            // }
            log_error("RabbitMQ{}时,发生未知的Response Server错误", desc.c_str());
            break;

        default:
            break;
    }

    return -1;
}

/**
 * @description: 获取RabbitMQ版本
 */
void utility::ShowRabbitMqVersion()
{
    log_info("Rabbitmq Version %d.%d.%d", AMQP_VERSION_MAJOR, AMQP_VERSION_MINOR, AMQP_VERSION_PATCH);
}
