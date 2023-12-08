#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "systime.h"
#include "logger.h"
#include "configparser.h"
#include "reactor.h"
#include "filetransfer.h"
#include "event.h"

using namespace std;
using namespace driver;
using namespace utility;
using namespace reactor;

void produce()
{
    /* 初始化设备 */
    DeviceEvent o1(1, DeviceEvent::EVENT_INIT , "opticalfiber", "");
    DeviceEvent r1(1, DeviceEvent::EVENT_INIT , "radiodigital", "");
    DeviceEvent u1(1, DeviceEvent::EVENT_INIT , "underwateracoustic", "");
    DeviceEvent s1(1, DeviceEvent::EVENT_INIT , "satellite", "");
    Reactor::instance()->push(o1);
    Reactor::instance()->push(r1);
    Reactor::instance()->push(u1);
    Reactor::instance()->push(s1);
    sleep(1);

    DeviceEvent o2(2, DeviceEvent::EVENT_WRITE, "opticalfiber", "/root/Gateway_Control_System/storage/test1.txt");
    DeviceEvent r2(2, DeviceEvent::EVENT_WRITE, "radiodigital", "/root/Gateway_Control_System/storage/test2.txt");
    Reactor::instance()->push(o2);
    Reactor::instance()->push(r2);
    sleep(1);

    DeviceEvent u2(2, DeviceEvent::EVENT_WRITE, "underwateracoustic", "/root/Gateway_Control_System/storage/test1.txt");
    DeviceEvent u3(3, DeviceEvent::EVENT_WRITE, "underwateracoustic", "/root/Gateway_Control_System/storage/test2.txt");
    DeviceEvent u4(4, DeviceEvent::EVENT_WRITE, "underwateracoustic", "/root/Gateway_Control_System/storage/test3.txt");
    DeviceEvent u5(5, DeviceEvent::EVENT_WRITE, "underwateracoustic", "/root/Gateway_Control_System/storage/test4.txt");

    DeviceEvent s2(2, DeviceEvent::EVENT_WRITE, "satellite"         , "/root/Gateway_Control_System/storage/test1.txt");
    DeviceEvent s3(3, DeviceEvent::EVENT_WRITE, "satellite"         , "/root/Gateway_Control_System/storage/test2.txt");
    DeviceEvent s4(4, DeviceEvent::EVENT_WRITE, "satellite"         , "/root/Gateway_Control_System/storage/test3.txt");
    DeviceEvent s5(5, DeviceEvent::EVENT_WRITE, "satellite"         , "/root/Gateway_Control_System/storage/test4.txt");

    Reactor::instance()->push(u2);
    Reactor::instance()->push(s2);
    Reactor::instance()->push(u3);
    Reactor::instance()->push(s3);
    Reactor::instance()->push(u4);
    Reactor::instance()->push(s4);
    Reactor::instance()->push(u5);
    Reactor::instance()->push(s5);
    sleep(10);

    DeviceEvent o9(9, DeviceEvent::EVENT_CLOSE , "opticalfiber", "");
    DeviceEvent r9(9, DeviceEvent::EVENT_CLOSE , "radiodigital", "");
    DeviceEvent u9(9, DeviceEvent::EVENT_CLOSE , "underwateracoustic", "");
    DeviceEvent s9(9, DeviceEvent::EVENT_CLOSE , "satellite", "");
    Reactor::instance()->push(o9);
    Reactor::instance()->push(r9);
    Reactor::instance()->push(u9);
    Reactor::instance()->push(s9);
}

int main()
{
    /* 初始化日志模块           */
    Logger::instance()->init("../logs/C.log", Logger::STREAM_BOTH, Logger::MODE_SYNC, 
                                              Logger::LEVEL_DEBUG, Logger::LEVEL_INFO, Logger::LEVEL_DEBUG);
    log_critical("program start ...");

    /* 初始化配置模块           */
    IniConfigParser config;
    bool parserFlag = true;
    parserFlag = config.load("../config/defconfig.ini");

    /* 初始化Reactor架构        */
    Reactor::instance()->init(&config);

    std::thread th(produce);

    /* 监听处理事件 */
    Reactor::instance()->listen();

    /* 注销日志模块             */
    log_critical("program end ...");
    Logger::instance()->deinit();

    return 0;
}