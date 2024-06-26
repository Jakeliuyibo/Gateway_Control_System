#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "logger.hpp"
#include "configparser.hpp"
#include "event.hpp"
#include "rabbitmqclient.hpp"

using namespace std;
using namespace utility;
using namespace reactor;

int main()
{
    /* 初始化日志模块           */
    auto fg = Logger::Instance()->Init("../logs/P.log", Logger::WorkStream::BOTH, Logger::WorkMode::SYNC,
        Logger::WorkLevel::DEBUG, Logger::WorkLevel::INFO, Logger::WorkLevel::DEBUG);
    log_critical("Pressure Test Program Start ...");

    /* 初始化配置模块           */
    IniConfigParser config;
    bool parserFlag = true;
    parserFlag = config.Load("../config/defconfig.ini");

    /* 初始化事件源 */
    std::string rabbitmqHostName, rabbitmqUser, rabbitmqPassword, exchangeNameIn, queueNameIn, routingKeyIn;
    int rabbitmqPort; 
    parserFlag &= config.GetValue<std::string>("RABBITMQ", "RABBITMQ_HOSTNAME"         , rabbitmqHostName);
    parserFlag &= config.GetValue<int        >("RABBITMQ", "rabbitmqPort"             , rabbitmqPort);
    parserFlag &= config.GetValue<std::string>("RABBITMQ", "RABBITMQ_USER"             , rabbitmqUser);
    parserFlag &= config.GetValue<std::string>("RABBITMQ", "RABBITMQ_PASSWORD"         , rabbitmqPassword);
    parserFlag &= config.GetValue<std::string>("RABBITMQ", "RABBITMQ_EXCHANGENAME_IN"  , exchangeNameIn);
    parserFlag &= config.GetValue<std::string>("RABBITMQ", "RABBITMQ_QUEUENAME_IN"     , queueNameIn);
    parserFlag &= config.GetValue<std::string>("RABBITMQ", "RABBITMQ_ROUTINGKEY_IN"    , routingKeyIn);

    std::unique_ptr<RabbitMqClient> p_rabbitmqclient = std::make_unique<RabbitMqClient>(rabbitmqHostName, rabbitmqPort, rabbitmqUser, rabbitmqPassword);
    p_rabbitmqclient->Connect();

    sleep(1);

    /* 初始化设备 */
    DeviceEvent o101_1(1, DeviceEvent::EventType::OPEN , 1, "", "", "");
    DeviceEvent o100_1(1, DeviceEvent::EventType::OPEN , 5, "", "", "");
    p_rabbitmqclient->Publish(exchangeNameIn, routingKeyIn, o101_1.Serial());
    p_rabbitmqclient->Publish(exchangeNameIn, routingKeyIn, o100_1.Serial());
    // DeviceEvent r1(1, DeviceEvent::EVENT_OPEN , 2, "", "", "");
    // DeviceEvent u1(1, DeviceEvent::EVENT_OPEN , 3, "", "", "");
    // DeviceEvent s1(1, DeviceEvent::EVENT_OPEN , 4, "", "", "");
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, o1.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, r1.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, u1.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, s1.serial());
    sleep(1);

    // DeviceEvent o2(2, DeviceEvent::EVENT_WRITE, 1, "/home/Gateway_Management_System/storage/upload/test.txt", "", "");
    // DeviceEvent r2(2, DeviceEvent::EVENT_WRITE, 2, "/home/Gateway_Management_System/storage/upload//test1.txt", "", "");
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, o2.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, r2.serial());
    DeviceEvent o101_2(2, DeviceEvent::EventType::WRITE, 1, "/home/Gateway_Management_System/storage/upload/test.txt", "", "");
    p_rabbitmqclient->Publish(exchangeNameIn, routingKeyIn, o101_2.Serial());
    sleep(3);

    // DeviceEvent u2(2, DeviceEvent::EVENT_WRITE, 3, "/home/Gateway_Control_System/storage/test1.txt", "", "");
    // DeviceEvent u3(3, DeviceEvent::EVENT_WRITE, 3, "/home/Gateway_Control_System/storage/test2.txt", "", "");
    // DeviceEvent u4(4, DeviceEvent::EVENT_WRITE, 3, "/home/Gateway_Control_System/storage/test3.txt", "", "");
    // DeviceEvent u5(5, DeviceEvent::EVENT_WRITE, 3, "/home/Gateway_Control_System/storage/test4.txt", "", "");
    // DeviceEvent s2(2, DeviceEvent::EVENT_WRITE, 4, "/home/Gateway_Control_System/storage/test1.txt", "", "");
    // DeviceEvent s3(3, DeviceEvent::EVENT_WRITE, 4, "/home/Gateway_Control_System/storage/test2.txt", "", "");
    // DeviceEvent s4(4, DeviceEvent::EVENT_WRITE, 4, "/home/Gateway_Control_System/storage/test3.txt", "", "");
    // DeviceEvent s5(5, DeviceEvent::EVENT_WRITE, 4, "/home/Gateway_Control_System/storage/test4.txt", "", "");
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, u2.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, u3.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, u4.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, u5.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, s2.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, s3.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, s4.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, s5.serial());
    // sleep(10);

    // DeviceEvent o9(9, DeviceEvent::EVENT_CLOSE , 1, "", "", "");
    // DeviceEvent r9(9, DeviceEvent::EVENT_CLOSE , 2, "", "", "");
    // DeviceEvent u9(9, DeviceEvent::EVENT_CLOSE , 3, "", "", "");
    // DeviceEvent s9(9, DeviceEvent::EVENT_CLOSE , 4, "", "", "");
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, o9.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, r9.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, u9.serial());
    // p_rabbitmqclient->publish(exchangeNameIn, routingKeyIn, s9.serial());
    DeviceEvent o101_9(9, DeviceEvent::EventType::CLOSE , 1, "", "", "");
    DeviceEvent o100_9(9, DeviceEvent::EventType::CLOSE , 5, "", "", "");
    p_rabbitmqclient->Publish(exchangeNameIn, routingKeyIn, o101_9.Serial());
    p_rabbitmqclient->Publish(exchangeNameIn, routingKeyIn, o100_9.Serial());

    
    sleep(1000);

    /* 注销日志模块             */
    log_critical("Pressure Test Program End ...");
    Logger::Instance()->Deinit();

    return 0;
}