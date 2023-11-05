#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include <uv.h>
#include "systime.h"
#include "logger.h"
#include "configparser.h"
#include "threadpool.h"
#include "queue.h"
#include "event.h"
#include "processor.h"

using namespace std;
using namespace utility;
using namespace reactor;

int threadFunc(int idx)
{
    critical(" this thread is {}", idx);
    std::this_thread::sleep_for(std::chrono::seconds(idx));
    critical(" thread {} is ending", idx);
    return idx + 10086;
}

int main()
{
    /* 初始化日志模块           */
    Logger::instance()->init("../logs/C.log", Logger::STREAM_BOTH, Logger::MODE_SYNC, 
                                              Logger::LEVEL_DEBUG, Logger::LEVEL_WARNING, Logger::LEVEL_DEBUG);
    critical("program start ...");

    /* 初始化配置模块           */
    IniConfigParser parser;
    bool parserFlag = true;
    parserFlag = parser.load("../config/defconfig.ini");

    /* 初始化队列               */
    // Queue q(&parser);
    // DeviceEvent de(123, DeviceEvent::EVENT_INIT, "eth0", "../a.txt");
    // q.push(de.serial());

    // DeviceEvent pe(q.pop());
    // cout << pe.serial() << endl;

    /* 测试线程池 */
    Processor processor(&parser);
    for(int idx=1; idx<10; idx++)
    {
        auto fut = processor.submit(threadFunc, idx);
        cout << "idx result = " << fut.get() << endl;
    }
    processor.shutdown();

    /* 注销日志模块             */
    critical("program end ...");
    Logger::instance()->deinit();

    return 0;
}