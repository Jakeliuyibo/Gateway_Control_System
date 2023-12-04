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
#include "commdevice.h"

using namespace std;
using namespace driver;
using namespace utility;
using namespace reactor;

void produce()
{
    /* 初始化设备 */
    DeviceEvent e1(1, DeviceEvent::EVENT_INIT , "opticalfiber", "");
    DeviceEvent e2(1, DeviceEvent::EVENT_INIT , "radiodigital", "");
    DeviceEvent e3(1, DeviceEvent::EVENT_INIT , "underwateracoustic", "");
    Reactor::instance()->push(e1);
    Reactor::instance()->push(e2);
    Reactor::instance()->push(e3);
    sleep(1);

    // DeviceEvent e4(2, DeviceEvent::EVENT_WRITE, "opticalfiber", "/root/Gateway_Control_System/storage/test.txt");
    // DeviceEvent e5(2, DeviceEvent::EVENT_WRITE, "radiodigital", "/root/Gateway_Control_System/storage/test.txt");
    // Reactor::instance()->push(e4);
    // Reactor::instance()->push(e5);
    // sleep(3);

    // DeviceEvent e7(3, DeviceEvent::EVENT_CLOSE , "opticalfiber", "");
    // DeviceEvent e8(3, DeviceEvent::EVENT_CLOSE , "radiodigital", "");
    // Reactor::instance()->push(e7);
    // Reactor::instance()->push(e8);
}

int main()
{
    /* 初始化日志模块           */
    Logger::instance()->init("../logs/C.log", Logger::STREAM_BOTH, Logger::MODE_SYNC, 
                                              Logger::LEVEL_DEBUG, Logger::LEVEL_DEBUG, Logger::LEVEL_DEBUG);
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