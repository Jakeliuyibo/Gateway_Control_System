#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "systime.h"
#include "logger.h"
#include "configparser.h"
#include "event.h"
#include "rabbitmqclient.h"

using namespace std;
using namespace utility;
using namespace reactor;

int main()
{
    /* 初始化日志模块           */
    Logger::instance()->init("../logs/P.log", Logger::STREAM_BOTH, Logger::MODE_SYNC, 
                                              Logger::LEVEL_DEBUG, Logger::LEVEL_INFO, Logger::LEVEL_DEBUG);
    log_critical("Pressure Test Program Start ...");

    /* 初始化配置模块           */
    IniConfigParser config;
    bool parserFlag = true;
    parserFlag = config.load("../config/defconfig.ini");

    /* 初始化事件源 */
    std::string rabbitmq_hostname, rabbitmq_user, rabbitmq_password, m_exchangename_in, m_queuename_in, m_routingkey_in;
    int rabbitmq_port; 
    parserFlag &= config.getValue<std::string>("RABBITMQ", "RABBITMQ_HOSTNAME"         , rabbitmq_hostname);
    parserFlag &= config.getValue<int        >("RABBITMQ", "RABBITMQ_PORT"             , rabbitmq_port);
    parserFlag &= config.getValue<std::string>("RABBITMQ", "RABBITMQ_USER"             , rabbitmq_user);
    parserFlag &= config.getValue<std::string>("RABBITMQ", "RABBITMQ_PASSWORD"         , rabbitmq_password);
    parserFlag &= config.getValue<std::string>("RABBITMQ", "RABBITMQ_EXCHANGENAME_IN"  , m_exchangename_in);
    parserFlag &= config.getValue<std::string>("RABBITMQ", "RABBITMQ_QUEUENAME_IN"     , m_queuename_in);
    parserFlag &= config.getValue<std::string>("RABBITMQ", "RABBITMQ_ROUTINGKEY_IN"    , m_routingkey_in);

    std::unique_ptr<RabbitMqClient> p_rabbitmqclient = std::make_unique<RabbitMqClient>(rabbitmq_hostname, rabbitmq_port, rabbitmq_user, rabbitmq_password);
    p_rabbitmqclient->connect();

    sleep(1);

    /* 初始化设备 */
    DeviceEvent o1(1, DeviceEvent::EVENT_OPEN , 1, "", "", "");
    DeviceEvent r1(1, DeviceEvent::EVENT_OPEN , 2, "", "", "");
    DeviceEvent u1(1, DeviceEvent::EVENT_OPEN , 3, "", "", "");
    DeviceEvent s1(1, DeviceEvent::EVENT_OPEN , 4, "", "", "");
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, o1.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, r1.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, u1.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, s1.serial());
    sleep(1);

    DeviceEvent o2(2, DeviceEvent::EVENT_WRITE, 1, "/root/Gateway_Control_System/storage/test1.txt", "", "");
    DeviceEvent r2(2, DeviceEvent::EVENT_WRITE, 2, "/root/Gateway_Control_System/storage/test2.txt", "", "");
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, o2.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, r2.serial());
    sleep(1);

    DeviceEvent u2(2, DeviceEvent::EVENT_WRITE, 3, "/root/Gateway_Control_System/storage/test1.txt", "", "");
    DeviceEvent u3(3, DeviceEvent::EVENT_WRITE, 3, "/root/Gateway_Control_System/storage/test2.txt", "", "");
    DeviceEvent u4(4, DeviceEvent::EVENT_WRITE, 3, "/root/Gateway_Control_System/storage/test3.txt", "", "");
    DeviceEvent u5(5, DeviceEvent::EVENT_WRITE, 3, "/root/Gateway_Control_System/storage/test4.txt", "", "");
    DeviceEvent s2(2, DeviceEvent::EVENT_WRITE, 4, "/root/Gateway_Control_System/storage/test1.txt", "", "");
    DeviceEvent s3(3, DeviceEvent::EVENT_WRITE, 4, "/root/Gateway_Control_System/storage/test2.txt", "", "");
    DeviceEvent s4(4, DeviceEvent::EVENT_WRITE, 4, "/root/Gateway_Control_System/storage/test3.txt", "", "");
    DeviceEvent s5(5, DeviceEvent::EVENT_WRITE, 4, "/root/Gateway_Control_System/storage/test4.txt", "", "");
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, u2.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, u3.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, u4.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, u5.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, s2.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, s3.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, s4.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, s5.serial());
    sleep(10);

    DeviceEvent o9(9, DeviceEvent::EVENT_CLOSE , 1, "", "", "");
    DeviceEvent r9(9, DeviceEvent::EVENT_CLOSE , 2, "", "", "");
    DeviceEvent u9(9, DeviceEvent::EVENT_CLOSE , 3, "", "", "");
    DeviceEvent s9(9, DeviceEvent::EVENT_CLOSE , 4, "", "", "");
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, o9.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, r9.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, u9.serial());
    p_rabbitmqclient->publish(m_exchangename_in, m_routingkey_in, s9.serial());

    sleep(1000);

    /* 注销日志模块             */
    log_critical("Pressure Test Program End ...");
    Logger::instance()->deinit();

    return 0;
}