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
    parserFlag &= config.GetValue<int        >("RABBITMQ", "RABBITMQ_PORT"             , rabbitmqPort);
    parserFlag &= config.GetValue<std::string>("RABBITMQ", "RABBITMQ_USER"             , rabbitmqUser);
    parserFlag &= config.GetValue<std::string>("RABBITMQ", "RABBITMQ_PASSWORD"         , rabbitmqPassword);
    parserFlag &= config.GetValue<std::string>("RABBITMQ", "RABBITMQ_EXCHANGENAME_IN"  , exchangeNameIn);
    parserFlag &= config.GetValue<std::string>("RABBITMQ", "RABBITMQ_QUEUENAME_IN"     , queueNameIn);
    parserFlag &= config.GetValue<std::string>("RABBITMQ", "RABBITMQ_ROUTINGKEY_IN"    , routingKeyIn);

    std::unique_ptr<RabbitMqClient> pRabbitmqclient = std::make_unique<RabbitMqClient>(rabbitmqHostName, rabbitmqPort, rabbitmqUser, rabbitmqPassword);
    pRabbitmqclient->Connect();

    #define PUBLISH_EVENT(event) do{ \
        pRabbitmqclient->Publish(exchangeNameIn, routingKeyIn, event.Serial()); \
    } while (0)

    /* 初始化设备 */
    DeviceEvent evOpenDev1(1, DeviceEvent::EventType::OPEN , 1, "", "", "");
    DeviceEvent evOpenDev4(1, DeviceEvent::EventType::OPEN , 4, "", "", "");
    DeviceEvent evOpenDev5(1, DeviceEvent::EventType::OPEN , 5, "", "", "");
    DeviceEvent evOpenDev8(1, DeviceEvent::EventType::OPEN , 8, "", "", "");
    PUBLISH_EVENT(evOpenDev1);
    PUBLISH_EVENT(evOpenDev4);
    PUBLISH_EVENT(evOpenDev5);
    PUBLISH_EVENT(evOpenDev8);
    sleep(1);

    DeviceEvent evWriteDev1(2, DeviceEvent::EventType::WRITE, 1, "/home/Gateway_Control_System/storage/test.txt", "", "");
    DeviceEvent evWriteDev4(2, DeviceEvent::EventType::WRITE, 4, "/home/Gateway_Control_System/storage/test.txt", "", "");
    PUBLISH_EVENT(evWriteDev1);
    PUBLISH_EVENT(evWriteDev4);
    sleep(3);

    DeviceEvent evCloseDev1(9, DeviceEvent::EventType::CLOSE , 1, "", "", "");
    DeviceEvent evCloseDev4(9, DeviceEvent::EventType::CLOSE , 4, "", "", "");
    DeviceEvent evCloseDev5(9, DeviceEvent::EventType::CLOSE , 5, "", "", "");
    DeviceEvent evCloseDev8(9, DeviceEvent::EventType::CLOSE , 8, "", "", "");
    PUBLISH_EVENT(evCloseDev1);
    PUBLISH_EVENT(evCloseDev4);
    PUBLISH_EVENT(evCloseDev5);
    PUBLISH_EVENT(evCloseDev8);
    sleep(1000);

    /* 注销日志模块             */
    log_critical("Pressure Test Program End ...");
    Logger::Instance()->Deinit();

    return 0;
}