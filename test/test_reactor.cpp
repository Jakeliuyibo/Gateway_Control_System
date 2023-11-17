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
    Reactor::instance()->push(e1);
    sleep(1);

    DeviceEvent e2(2, DeviceEvent::EVENT_WRITE, "opticalfiber", "/root/Gateway_Control_System/storage/test.txt");
    Reactor::instance()->push(e2);
    sleep(3);

    DeviceEvent e3(3, DeviceEvent::EVENT_CLOSE , "opticalfiber", "");
    Reactor::instance()->push(e3);
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