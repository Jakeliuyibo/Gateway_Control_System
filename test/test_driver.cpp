#include <iostream>
#include <string>
#include <unistd.h>
#include "systime.h"
#include "logger.h"
#include "configparser.h"
#include "safequeue.h"
#include "filetransfer.h"
#include "event.h"
#include "commdevice.h"

using namespace std;
using namespace driver;
using namespace utility;
using namespace reactor;

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

    // FileTransfer *tf = new TcpFileTransfer(1234, "127.0.0.1", 1234);
    // tf->transfer("/root/Gateway_Control_System/storage/", "test.txt");
    OpticalfiberCommDev op(&config);
    DeviceEvent e1(1, DeviceEvent::EVENT_INIT , "opticalfiber", "");
    DeviceEvent e2(2, DeviceEvent::EVENT_WRITE, "opticalfiber", "/root/Gateway_Control_System/storage/test.txt");
    op.handleEvent(e1);
    op.handleEvent(e2);
    
    sleep(3);

    /* 注销日志模块             */
    log_critical("program end ...");
    Logger::instance()->deinit();

    return 0;
}