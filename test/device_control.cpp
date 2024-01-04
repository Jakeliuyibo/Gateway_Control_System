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


int main()
{
    /* 初始化日志模块           */
    Logger::instance()->init("../logs/C.log", Logger::STREAM_BOTH, Logger::MODE_SYNC, 
                                              Logger::LEVEL_DEBUG, Logger::LEVEL_INFO, Logger::LEVEL_DEBUG);
    log_critical("Device Control Program Start ...");

    /* 初始化配置模块           */
    IniConfigParser config;
    bool parserFlag = true;
    parserFlag = config.load("../config/defconfig.ini");

    /* 初始化Reactor架构        */
    Reactor::instance()->init(&config);

    /* 监听处理事件 */
    Reactor::instance()->listen();

    /* 注销日志模块             */
    log_critical("Device Control Program End ...");
    Logger::instance()->deinit();

    return 0;
}