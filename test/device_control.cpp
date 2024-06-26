#include <iostream>
#include <string>
#include <chrono>
#include <thread>

#include "logger.hpp"
#include "configparser.hpp"
#include "reactor.hpp"

using namespace std;
using namespace driver;
using namespace utility;
using namespace reactor;

int main()
{
    /* 随机种子 */ 
    std::srand(static_cast<unsigned int>(std::time(nullptr)));

    /* 初始化日志模块           */
    auto fg = Logger::Instance()->Init("../logs/C.log", Logger::WorkStream::BOTH, Logger::WorkMode::SYNC,
        Logger::WorkLevel::DEBUG, Logger::WorkLevel::INFO, Logger::WorkLevel::DEBUG);
    log_critical("Device Control Program Start ...");

    /* 初始化配置模块           */
    IniConfigParser config;
    bool parserFlag = true;
    parserFlag = config.Load("../config/defconfig.ini");

    /* 初始化Reactor架构        */
    Reactor::Instance()->Init(&config);

    /* 监听处理事件 */
    Reactor::Instance()->Listen();

    /* 注销日志模块             */
    log_critical("Device Control Program End ...");
    Logger::Instance()->Deinit();

    return 0;
}
