/*
 * @Author       : liuyibo
 * @Date         : 2023-12-22 07:05:38
 * @LastEditors  : liuyibo 1299502716@qq.com
 * @LastEditTime : 2024-06-25 16:46:42
 * @FilePath     : /Gateway_Control_System/test/exec_management.cpp
 * @Description  : 程序执行管理，负责基础服务及应用软件的创建、关闭和维护
 */
#include <iostream>
#include <string>
#include <vector>
#include <map>
#include <unistd.h>
#include <sys/wait.h>
#include <thread>
#include <limits.h>

#include "systime.hpp"
#include "logger.hpp"
#include "configparser.hpp"

using namespace std;
using namespace utility;

int main()
{
    /* 定义基础服务和用户应用 */
    static std::vector<std::string> basicService = {"../scripts/init_rabbitmqserver.sh", "../scripts/init_virtualserial.sh", "../scripts/init_redis.sh", "../scripts/init_apache2.sh"};
    static std::vector<std::string> userApp = {"./device_control", "./pressure_test"};

    /* 初始化日志模块           */
    auto fg = Logger::Instance()->Init("../logs/EM.log", Logger::WorkStream::BOTH, Logger::WorkMode::SYNC,
        Logger::WorkLevel::DEBUG, Logger::WorkLevel::INFO, Logger::WorkLevel::DEBUG);
    log_critical("Exec Management Program Start ...");

    /* 初始化基础服务 */
    for(auto &script : basicService)
    {
        int status = system(script.c_str());
        if (status == 0)
        {
            log_info("初始化{}服务成功", script);
        }
        else
        {
            log_error("初始化{}服务失败, 状态码({})", script, status);
        }
    }

    /* 加载用户应用 */
    std::map<pid_t, std::string> pidList;
    for(auto &script : userApp)
    {
        pid_t pid = fork();
        if (pid == 0)           // !子进程
        {
            execl(script.c_str(), script.c_str(), NULL);
            exit(0);
        }
        else if (pid > 0)       // !父进程
        {
            pidList[pid] = script;
        }
        else
        {
            log_error("执行APP({})时创建子进程失败", script);
            exit(1);
        }
    }
    
    // 循环监控
    while(true)
    {
        int status;
        pid_t pid = wait(&status);

        if (pid == -1)
        {
            log_error("所有应用异常崩溃");
            break;
        }
        else
        {
            log_warning("守护进程收到异常关闭的子进程({},{})，稍后进行重启", pid, pidList[pid]);
            
            /* 查询子进程列表 */
            auto iter = pidList.find(pid);
            if (iter != pidList.end())
            {
                std::string script = iter->second;
                pidList.erase(pid);

                /* 创建新进程恢复应用 */
                pid_t newPid = fork();
                if (newPid == 0)           // !恢复的子进程
                {
                    execl(script.c_str(), script.c_str(), NULL);
                    exit(0);
                }
                else if (newPid > 0)       // !父进程
                {
                    pidList[newPid] = script;
                }
                else
                {
                    log_error("执行APP({})时创建子进程失败", script);
                    exit(1);
                }
            }
            else
            {
                log_error("子进程列表中找不到异常崩溃的子进程({})", pid);
            }
        }

        sleep(1);
    }

    /* 注销日志模块             */
    log_critical("Exec Management Program End ...");
    Logger::Instance()->Deinit();

    return 0;
}