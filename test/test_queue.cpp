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
#include "systime.h"

using namespace std;
using namespace driver;
using namespace utility;
using namespace reactor;


void* Producer(void *argc)
{  
    SafeQueue<int> *t = (SafeQueue<int> *)argc;
    std::cout << utility::getSystime()<<"生产者：" << "I have sleep 1s" << std::endl;
    //先上锁然后休眠1s，形成阻塞条件
    sleep(3);

    t->enqueue(1);
    std::cout << utility::getSystime()<<"生产者：" << "I have send data"<< 3 << std::endl;

}
void* Consumer1(void *argc)
{
    SafeQueue<int> *t = (SafeQueue<int> *)argc;
    int x=99;
    t->dequeue(x);
    std::cout <<utility::getSystime()<< "消费者：" << "I have get data"<< x << std::endl;

}
void* Consumer2(void *argc)
{

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

    // SafeQueue<int> queue;
    // queue.enqueue(1);
    // queue.enqueue(2);
    
    // int res = 0;
    // queue.dequeue(res);
    // cout << res << endl;
    // queue.dequeue(res);
    // cout << res << endl;
    SafeQueue<int> queue;
    pthread_t c1, c2, p;
    // 创建生产者
    pthread_create(&p, NULL,  Producer, &queue);

    // 创建消费者1
    pthread_create(&c1, NULL, Consumer1, &queue);

    // 创建消费者2
    pthread_create(&c2, NULL, Consumer2, &queue);

    pthread_join(p,  NULL);
    pthread_join(c1, NULL);
    pthread_join(c2, NULL);


    /* 注销日志模块             */
    log_critical("program end ...");
    Logger::instance()->deinit();

    return 0;
}