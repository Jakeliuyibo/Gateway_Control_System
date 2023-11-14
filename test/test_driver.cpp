#include <iostream>
#include <string>
#include <unistd.h>
#include "systime.h"
#include "logger.h"
#include "configparser.h"
#include "safequeue.h"
#include "transchannel.h"

using namespace std;
using namespace driver;
using namespace utility;

class AAA
{
    public:
        AAA(int v) : val(v) {}
        int val;
};

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

    TcpFTChannel tf(1234, "127.0.0.1", 1234);
    tf.transfer("/root/Gateway_Control_System/storage/", "test.txt");

    usleep(3000000);

    /* 注销日志模块             */
    log_critical("program end ...");
    Logger::instance()->deinit();

    return 0;
}