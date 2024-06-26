#include <iostream>
#include <thread>
#include <vector>
#include <functional>

#include "gtest/gtest.h"

#include "logger.hpp"
#include "configparser.hpp"
#include "safemap.hpp"
#include "safequeue.hpp"
#include "systime.hpp"
#include "singleton.hpp"
#include "threadpool.hpp"
#include "source.hpp"

using namespace std;
using namespace utility;

TEST(MODULE, INI_CONFIGPARSER)
{
    /* 初始化配置模块           */
    IniConfigParser config;
    auto parserFlag = config.Load("../config/defconfig.ini");

    std::string rabbitmqHostName;
    int rabbitmqPort;
    parserFlag &= config.GetValue("RABBITMQ", "RABBITMQ_HOSTNAME", rabbitmqHostName);
    parserFlag &= config.GetValue<int>("RABBITMQ", "RABBITMQ_PORT", rabbitmqPort);

    EXPECT_EQ(rabbitmqHostName, "localhost");
    EXPECT_EQ(rabbitmqPort, 5672);
}

TEST(MODULE, JSON_CONFIGPARSER)
{
    /* 初始化配置模块           */
    JsonConfigParser config;
    auto parserFlag = config.Load("../config/other.json");

    std::string rabbitmqHostName;
    int rabbitmqPort;
    parserFlag &= config.GetValue("RABBITMQ_HOSTNAME", rabbitmqHostName);
    parserFlag &= config.GetValue<int>("RABBITMQ_PORT", rabbitmqPort);

    EXPECT_EQ(rabbitmqHostName, "localhost");
    EXPECT_EQ(rabbitmqPort, 5672);
}

TEST(MODULE, SAFEMAP)
{
    SafeMap<int, int> mp;
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back(
            [&](int start, int end)
            {
                for (int i = start; i < end; ++i) {
                    mp.Insert(i, i * 2);
                }
            },
            i * 10000,
            (i + 1) * 10000);
    }

    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_EQ(mp.Size(), 100000);
}

TEST(MODULE, SAFEQUEUE)
{
    SafeQueue<int> sq;
    std::vector<std::thread> threads;

    for (int i = 0; i < 10; ++i)
    {
        threads.emplace_back(
            [&](int start, int end)
            {
                for (int i = start; i < end; ++i) {
                    sq.Enqueue(i);
                }
            }, i * 10000, (i + 1) * 10000);
    }
    for (auto& th : threads)
    {
        th.join();
    }

    EXPECT_EQ(sq.Size(), 100000);
}

TEST(MODULE, SYSTIME)
{
    std::cout << GetSystime() << endl;
    std::cout << GetSystimeUs() << endl;
    std::cout << GetSystimeByFilenameFormat() << endl;
}

TEST(MODULE, SINGLETON)
{
    class TestSingleton
    {
        SINGLETON(TestSingleton);
    public:
        int a = 1;
    };
    EXPECT_EQ(TestSingleton::Instance()->a, 1);
}

TEST(MODULE, THREADPOOL)
{
    ThreadPool pool(10);

    {   // Test 1: Exception thrown in task
        auto future = pool.Submit([ ]() { throw std::runtime_error("Task threw an exception"); });
        EXPECT_THROW(future.get(), std::runtime_error);
    }

    {    // Test 2: Task returns a value
        for (int i = 0; i < 100; ++i)
        {
            auto future = pool.Submit(
                [ ](int a, int b)
                {
                    return a + b;
                }, i, i + 1
            );
            EXPECT_EQ(future.get(), 2 * i + 1);
        }
    }

    pool.Shutdown();
}

TEST(MODULE, RABBITMQCLIENT)
{
    /* 初始化配置模块           */
    IniConfigParser config;
    auto parserFlag = config.Load("../config/defconfig.ini");

    reactor::Source src(&config);
    src.PushIn("abc");
    EXPECT_EQ(src.PopIn(), "abc");
}

int main(int argc, char** argv)
{
    /* 初始化日志模块           */
    auto fg = Logger::Instance()->Init("../logs/MODULE_TEST.log", Logger::WorkStream::BOTH, Logger::WorkMode::SYNC,
        Logger::WorkLevel::DEBUG, Logger::WorkLevel::INFO, Logger::WorkLevel::DEBUG);
    log_critical("Test Start ...");

    testing::InitGoogleTest(&argc, argv);
    fg = RUN_ALL_TESTS();

    /* 注销日志模块             */
    log_critical("Test End ...");
    Logger::Instance()->Deinit();
    return 0;
}