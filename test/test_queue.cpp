#include <iostream>
#include <string>
#include <unistd.h>
#include <thread>
#include "systime.h"
#include "logger.h"
#include "configparser.h"
#include "safequeue.h"
#include "filetransfer.h"
#include "event.h"
#include "commdevice.h"
#include "systime.h"

using namespace std;
using namespace driver;
using namespace utility;
using namespace reactor;

void Producer(SafeQueue<int> &q)
{  
    std::cout << utility::getSystime()<<"生产者：" << "I have sleep 1s" << std::endl;
    //先上锁然后休眠1s，形成阻塞条件
    sleep(3);

    q.enqueue(1);
    std::cout << utility::getSystime()<<"生产者：" << "I have send data"<< 3 << std::endl;
}

void Consumer1(SafeQueue<int> &q)
{
    int x=99;
    q.dequeue(x, true);
    std::cout <<utility::getSystime()<< "消费者：" << "I have get data"<< x << std::endl;
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

    SafeQueue<int> queue;
    std::thread th1(Producer, std::ref(queue));
    std::thread th2(Consumer1, std::ref(queue));
    th1.join();
    th2.join();

    /* 注销日志模块             */
    log_critical("program end ...");
    Logger::instance()->deinit();

    return 0;
}