#include <iostream>
#include <string>
#include <chrono>
#include <thread>
#include "systime.h"
#include "logger.h"
#include "configparser.h"
#include "source.h"
#include "safemap.h"

using namespace std;
using namespace utility;
using namespace reactor;

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

    SafeMap<int, int> smp;
    cout << "empty ? = " << smp.empty() << endl;
    cout << smp[1] << endl;
    cout << smp[2] << endl;
    smp.set(1, 1);
    smp.set(2, 2);
    cout << smp[1] << endl;
    cout << smp[2] << endl;

    cout << "insert ? = " << smp.insert(3, 3) << endl;
    cout << "find ? = " << smp.find(1) << endl;
    cout << "size ?= " << smp.size() << endl;
    cout << "erase ? = " << smp.erase(2) << endl;
    cout << "find ? = " << smp.find(2) << endl;
    cout << "size ?= " << smp.size() << endl;
    cout << "empty ? = " << smp.empty() << endl;
    cout << "erase ? = " << smp.erase(4) << endl;

    // int &p = smp[3];
    // p = 4;
    // cout << smp[3] << endl;
    cout << "遍历" << endl;
    for(auto it : smp)
    {
        cout << "key = " << it.first << ", value = " << it.second << endl;
    }

    /* 注销日志模块             */
    log_critical("program end ...");
    Logger::instance()->deinit();

    return 0;
}