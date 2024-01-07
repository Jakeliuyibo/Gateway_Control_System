/*
 * @Author       : liuyibo
 * @Date         : 2023-12-22 07:05:38
 * @LastEditors  : liuyibo 1299502716@qq.com
 * @LastEditTime : 2024-01-07 09:43:24
 * @FilePath     : /home/Gateway_Control_System/test/exec_management.cpp
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
#include "systime.h"
#include "logger.h"
#include "configparser.h"

using namespace std;
using namespace utility;

int main()
{
    /* 初始化日志模块           */
    Logger::instance()->init("../logs/EM.log", Logger::STREAM_BOTH, Logger::MODE_SYNC, 
                                              Logger::LEVEL_DEBUG, Logger::LEVEL_INFO, Logger::LEVEL_DEBUG);
    log_critical("Exec Management Program Start ...");

    /* 初始化基础服务 */
    std::vector<std::string> basic_service = {"./init_rabbitmqserver.sh", "./init_virtualserial.sh", "./init_redis.sh", "init_apache2.sh"};
    for(auto &script : basic_service)
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
    // 创建应用
    std::vector<std::string> user_app = {"./device_control", "./pressure_test"};
    std::map<pid_t, std::string> pid_list;
    for(auto &script : user_app)
    {
        pid_t pid = fork();
        if (pid == 0)           // !子进程
        {
            execl(script.c_str(), script.c_str(), NULL);
            exit(0);
        }
        else if (pid > 0)       // !父进程
        {
            pid_list[pid] = script;
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
            log_warning("守护进程收到异常关闭的子进程({},{})，稍后进行重启", pid, pid_list[pid]);
            
            /* 查询子进程列表 */
            auto iter = pid_list.find(pid);
            if (iter != pid_list.end())
            {
                std::string script = iter->second;
                pid_list.erase(pid);

                /* 创建新进程恢复应用 */
                pid_t new_pid = fork();
                if (new_pid == 0)           // !恢复的子进程
                {
                    execl(script.c_str(), script.c_str(), NULL);
                    exit(0);
                }
                else if (new_pid > 0)       // !父进程
                {
                    pid_list[new_pid] = script;
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
    Logger::instance()->deinit();

    return 0;
}