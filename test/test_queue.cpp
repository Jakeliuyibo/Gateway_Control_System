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

    SafeQueue<int> queue;
    queue.enqueue(1);
    queue.enqueue(2);
    
    int res = 0;
    queue.dequeue(res);
    cout << res << endl;
    queue.dequeue(res);
    cout << res << endl;


    /* 注销日志模块             */
    log_critical("program end ...");
    Logger::instance()->deinit();

    return 0;
}