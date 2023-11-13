#include <iostream>
#include <string>
#include <unistd.h>
#include "systime.h"
#include "logger.h"
#include "configparser.h"
#include "serialchannel.h"
#include "tcpchannel.h"

using namespace std;
using namespace driver;
using namespace utility;

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

    TcpReceiver tcpreceiver(1234);

    TcpTransfer tcptransfer("127.0.0.1", 1234);
    tcptransfer.transfer("/root/Gateway_Control_System/storage/", "test.txt");


    // tcpreceiver.m_ioc.run();

    usleep(3000000);

    /* 注销日志模块             */
    log_critical("program end ...");
    Logger::instance()->deinit();

    return 0;
}