#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "systime.h"
#include "logger.h"
#include "configparser.h"
#include "source.h"

using namespace std;
using namespace utility;
using namespace reactor;

void produce(Source &source)
{
    int idx = 1;
    for(;;)
    {
        string msg = "hello" + to_string(idx++);
        source.push(msg);
        this_thread::sleep_for(chrono::milliseconds(10));
    }
}

void consume(Source &source)
{
    for(;;)
    {
        string msg = source.pop();
        this_thread::sleep_for(chrono::milliseconds(10));
    }
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
    Source source(&config);

    std::thread th1(produce, std::ref(source));
    std::thread th2(consume, std::ref(source));
    th1.join();
    th2.join();

    /* 注销日志模块             */
    log_critical("program end ...");
    Logger::instance()->deinit();

    return 0;
}