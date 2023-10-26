#include "rabbitmqclient.h"

#include "logger.h"

using namespace utility;
using namespace std;

/**
 * @description: 构造函数
 */
RabbitMqClient::RabbitMqClient(const string &hostname, int port, const string &user = "guest", const string &password = "guest") : m_hostname(hostname),
                                                                                                                                   m_port(port),
                                                                                                                                   m_username(user),
                                                                                                                                   m_password(password),
                                                                                                                                   m_conn(nullptr),
                                                                                                                                   m_channel(1)
{
}

/**
 * @description: 析构函数
 */
RabbitMqClient::~RabbitMqClient()
{
    disconnect();
}

/*************************************************************************
 *
 * Public Function
 *
 *************************************************************************/

/**
 * @description: 建立连接
 */
int RabbitMqClient::connect()
{
    /* 检测参数 */
    if (m_hostname.empty() || m_port <= 0 || m_username.empty() || m_password.empty())
    {
        error("Failed to construct RabbitMqClient object");
        return -1;
    }

    /* 1、分配新连接指针 */
    m_conn = amqp_new_connection();
    if (m_conn == nullptr)
    {
        error("Failed to allocate rabbitmq connection ptr");
        return -2;
    }

    /* 2、创建socket */
    amqp_socket_t *m_socket = amqp_tcp_socket_new(m_conn);
    if (m_socket == nullptr)
    {
        error("Failed to allocate rabbitmq socket ptr");
        return -3;
    }

    /* 3、绑定主机地址和端口，建立与服务器连接 */
    int isOpenSocket = amqp_socket_open(m_socket, m_hostname.c_str(), m_port);
    if (isOpenSocket < 0)
    {
        error("Failed to bind rabbitmq socket ip and port");
        return -4;
    }

    /* 4、登录到RabbitMq服务器 */
    int isLogin = errorMsg(amqp_login(m_conn,
                                      "/",
                                      1,
                                      AMQP_DEFAULT_FRAME_SIZE,
                                      AMQP_DEFAULT_HEARTBEAT,
                                      AMQP_SASL_METHOD_PLAIN,
                                      m_username.c_str(),
                                      m_password.c_str()),
                           "Connect");
    if (isLogin < 0)
    {
        error("Failed to login rabbitmq username and password");
        return -5;
    }

    /* 6、打开通道 */
    amqp_channel_open_ok_t *isOpenChannel = amqp_channel_open(m_conn, m_channel);
    if (!isOpenChannel)
    {
        error("Failed to open rabbitmq channel");
        return -6;
    }
    return 0;
}

/**
 * @description: 断开连接
 */
int RabbitMqClient::disconnect()
{
    int ret = 0;
    if (m_conn != nullptr)
    {
        /* 1、关闭通道 */
        ret = errorMsg(amqp_channel_close(m_conn, m_channel, AMQP_REPLY_SUCCESS), "Close Channel");
        if (ret < 0)
        {
            error("Failed to close rabbitmq channel");
            return ret;
        }

        /* 2、关闭连接 */
        ret = errorMsg(amqp_connection_close(m_conn, AMQP_REPLY_SUCCESS), "Close Connection");
        if (ret < 0)
        {
            error("Failed to close rabbitmq connection");
            return ret;
        }

        /* 3、销毁连接 */
        ret = amqp_destroy_connection(m_conn);
        if (ret < 0)
        {
            error("Failed to destory rabbitmq conn");
            return ret;
        }

        m_conn = nullptr;
    }
    else
    {
        warning("Try to disconnect a empty rabbitm connection");
        return -4;
    }

    return 0;
}

/**
 * @description: 初始化交换机
 */
int RabbitMqClient::declareExchange(CExchange &exchange)
{
    /* 声明交换机 */
    amqp_exchange_declare(m_conn,
                          m_channel,
                          amqp_cstring_bytes(exchange.m_name.c_str()),
                          amqp_cstring_bytes(exchange.m_type.c_str()),
                          exchange.m_passive,
                          exchange.m_durable,
                          exchange.m_autodelete,
                          exchange.m_internal,
                          amqp_empty_table);

    return errorMsg(amqp_get_rpc_reply(m_conn), "Declare exchange");
}

/**
 * @description: 初始化队列
 */
int RabbitMqClient::declareQueue(CQueue &queue)
{
    /* 声明队列 */
    amqp_queue_declare(m_conn,
                       m_channel,
                       amqp_cstring_bytes(queue.m_name.c_str()),
                       queue.m_passive,
                       queue.m_durable,
                       queue.m_exclusive,
                       queue.m_autodelete,
                       amqp_empty_table);

    return errorMsg(amqp_get_rpc_reply(m_conn), "Declare queue");
}

/**
 * @description: 将指定队列绑定到交换机上，在direct模式下bindkey可以为队列名称
 */
int RabbitMqClient::bindQueueToExchange(const string &queue, const string &exchange, const string &bindkey)
{
    amqp_queue_bind(m_conn,
                    m_channel,
                    amqp_cstring_bytes(queue.c_str()),
                    amqp_cstring_bytes(exchange.c_str()),
                    amqp_cstring_bytes(bindkey.c_str()),
                    amqp_empty_table);

    return errorMsg(amqp_get_rpc_reply(m_conn), "Bind queue to exchange");
}

/**
 * @description: 发布消息
 */
int RabbitMqClient::publish(const string &exchange_name, const string &routing_key_name, const CMessage &message)
{
    int ret = amqp_basic_publish(m_conn,
                                 m_channel,
                                 amqp_cstring_bytes(exchange_name.c_str()),
                                 amqp_cstring_bytes(routing_key_name.c_str()),
                                 message.m_mandatory,
                                 message.m_immediate,
                                 &message.m_properties,
                                 amqp_cstring_bytes(message.m_data.c_str()));
    if (ret != AMQP_STATUS_OK)
    {
        error("rabbitmq client publish message");
        return errorMsg(amqp_get_rpc_reply(m_conn), "Publish message");
    }

    return 0;
}

/**
 * @description: 非阻塞方式获取一条消息
 */
string RabbitMqClient::consume(const string &queue_name, struct timeval *timeout, bool no_ack)
{
    vector<string> vec_msg = consume(queue_name, 1, timeout, no_ack);
    if(vec_msg.size() != 1)
    {
        error("Try to consume one message from rabbitmq server, but get null or more than one");
        return "";
    }

    return vec_msg[0];
}

/**
 * @description: 非阻塞方式获取多条消息，底层为consume实现，本地一次拉取服务器所有消息，依次由客户端消费
 */
vector<string> RabbitMqClient::consume(const string &queue_name, int num, struct timeval *timeout, bool no_ack)
{
    vector<string> ret_msg;

    /* 1、设置通道消费的限制 */
    amqp_basic_qos_ok_t *retQosOk = amqp_basic_qos(m_conn,
                                                   m_channel,
                                                   0,       // 预取消息的字节数prefetch_size 0：不限制大小
                                                   num,     // 预取消息的数量prefetch_count 1：取1条消息
                                                   false);  // 是否将预取条件应用到整个通道 0：不应用
    if (!retQosOk)
    {
        errorMsg(amqp_get_rpc_reply(m_conn), "Set consumer limit(qos)");
        return ret_msg;
    }

    /* 2、创建消费者 */
    amqp_basic_consume_ok_t *retBasicConsume = amqp_basic_consume(m_conn,
                                                                  m_channel,
                                                                  amqp_cstring_bytes(queue_name.c_str()),
                                                                  amqp_empty_bytes,
                                                                  false,  // no_local 0:接收 1:不接收
                                                                  no_ack, // no_ack 是否需要ack才将该消息从队列删除 0:需要调用amqp_basic_ack后才会清除 1:不回复
                                                                  false,  // exclusive 0:不独占 1:当前连接不在时队列自动删除
                                                                  amqp_empty_table);
    if (!retBasicConsume)
    {
        errorMsg(amqp_get_rpc_reply(m_conn), "Consumer basic");
        return ret_msg;
    }

    while (num--)
    {
        /* 3、消费 */
        amqp_envelope_t envelope;
        int isConsume = errorMsg(amqp_consume_message(m_conn, &envelope, timeout, 0), "Consume message");
        if (isConsume < 0)
        {
            error("Faild to consume no.%d message from rabbitmq server", num);
            return ret_msg;
        }

        /* 4、封装消息 */
        ret_msg.emplace_back(string((char *)envelope.message.body.bytes, (char *)envelope.message.body.bytes + envelope.message.body.len));

        /* 5、应答ACK */
        if(no_ack == false)
        {
            amqp_basic_ack(m_conn, m_channel, envelope.delivery_tag, false);
        }

        /* 6、删除封装容器 */
        amqp_destroy_envelope(&envelope);
    }

    return ret_msg;
}

/**
 * @description: 同步方式消费一条消息
 */
string RabbitMqClient::consume(const string &queue_name, bool no_ack)
{
    vector<string> vec_msg = consume(queue_name, 1, no_ack);
    if (vec_msg.size() != 1)
    {
        error("Try to get one message from rabbitmq server, but get null or more than one");
        return "";
    }

    return vec_msg[0];
}

/**
 * @description: 同步方式消费多条消息，底层以amqp的get和read方法实现，每次向服务器拉取一条消息
 */
vector<string> RabbitMqClient::consume(const string &queue_name, int num, bool no_ack)
{
    vector<string> ret_msg;

    while (num--)
    {
        /*  1、阻塞同步轮询服务器中队列 */
        amqp_rpc_reply_t replyGet = amqp_basic_get(m_conn, m_channel, amqp_cstring_bytes(queue_name.c_str()), no_ack);
        int retGetMsg = errorMsg(replyGet, "Get message");
        if (retGetMsg < 0)
        {
            error("Failed to get message from RabbitMQ server");
            return ret_msg;
        }
        // 获取队列中存在多少条消息
        amqp_basic_get_ok_t *tip;
        switch(replyGet.reply.id)
        {
            case AMQP_BASIC_GET_OK_METHOD:
                tip = (amqp_basic_get_ok_t *)replyGet.reply.decoded;
                // info("rabbitmq queue remaining %d messages", tip->message_count);
                break;
            case AMQP_BASIC_GET_EMPTY_METHOD:
                info("no message in rabbitmq queue");
                return ret_msg;
            default:
                error("get error rabbitmq reply id %d", replyGet.reply.id);
                return ret_msg;
        }

        /* 2、读取chennal上的一条消息 */
        amqp_message_t amqp_msg;
        int retReadMsg = errorMsg(amqp_read_message(m_conn, m_channel, &amqp_msg, false), "Read message");
        if (retReadMsg < 0)
        {
            error("Failed to read rabbitmq message");
            return ret_msg;
        }

        /* 3、封装消息 */
        ret_msg.emplace_back(string((char *)amqp_msg.body.bytes, (char *)amqp_msg.body.bytes + amqp_msg.body.len));

        /* 4、应答ACK */
        if(no_ack == false)
        {
            amqp_basic_ack(m_conn, m_channel, tip->delivery_tag, false);
        }
    }

    return ret_msg;
}

/*************************************************************************
 *
 * Private Function
 *
 *************************************************************************/

/**
 * @description: 处理错误信息
 */
int RabbitMqClient::errorMsg(const amqp_rpc_reply_t &reply, const string &desc)
{
    switch (reply.reply_type)
    {
    case AMQP_RESPONSE_NORMAL:
        return 0;
    case AMQP_RESPONSE_NONE:
        error("RabbitMQ AMQP Response None Error, where is occured in %s", desc.c_str());
        break;
    case AMQP_RESPONSE_LIBRARY_EXCEPTION:
        error("RabbitMQ AMQP Response Library Error, where is occured in %s", desc.c_str());
        break;
    case AMQP_RESPONSE_SERVER_EXCEPTION:
        error("RabbitMQ AMQP Response Server Error, where is occured in %s", desc.c_str());
        break;
    default:
        break;
    }

    return -1;
}

/**
 * @description: 获取RabbitMq版本
 */
void utility::showRabbitMqVersion()
{
    info("Rabbitmq Version %d.%d.%d", AMQP_VERSION_MAJOR, AMQP_VERSION_MINOR, AMQP_VERSION_PATCH);
}