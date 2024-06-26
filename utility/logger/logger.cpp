#include "logger.hpp"

#include <iostream>
#include <vector>

using namespace utility;

/*************************************************************************
 *
 * Public Function
 *
 *************************************************************************/
bool Logger::Init(
    const std::string& filePath,
    const WorkStream stream, const WorkMode mode,
    const WorkLevel level, const WorkLevel levelConsole, const WorkLevel levelFile
)
{
    try
    {
        std::vector<spdlog::sink_ptr> sinks;

        /* 输出到控制台 */
        if (stream == WorkStream::CONSOLE || stream == WorkStream::BOTH)
        {
            auto consoleSink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            consoleSink->set_pattern(kConsoleFormat);
            consoleSink->set_level(static_cast<spdlog::level::level_enum>(static_cast<std::underlying_type<WorkLevel>::type>(levelConsole)));
            sinks.emplace_back(std::move(consoleSink));
        }

        /* 输出到文件 */
        if (stream == WorkStream::FILE || stream == WorkStream::BOTH)
        {
            // auto fileSink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filePath, 10 * 1024 * 1024, 5);
            auto fileSink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filePath, 23, 59);
            fileSink->set_pattern(kFileFormat);
            fileSink->set_level(static_cast<spdlog::level::level_enum>(static_cast<std::underlying_type<WorkLevel>::type>(levelFile)));
            sinks.emplace_back(fileSink);
        }

        /* 设置同步异步 */
        std::shared_ptr<spdlog::logger> pLogger;
        if (mode == WorkMode::SYNC)
        {
            pLogger = std::make_shared<spdlog::logger>(LOGGER_NAME, begin(sinks), end(sinks));
        }
        else if (mode == WorkMode::ASYNC)
        {
            // 创建队列大小10000，线程为1的工作线程池
            spdlog::init_thread_pool(kWorkQueueSize, kWorkThreadCount);
            auto tp = spdlog::thread_pool();
            pLogger = std::make_shared<spdlog::async_logger>(LOGGER_NAME, begin(sinks), end(sinks), tp, spdlog::async_overflow_policy::block);
        }

        /* 设置屏蔽等级 */
        pLogger->set_level(static_cast<spdlog::level::level_enum>(static_cast<std::underlying_type<WorkLevel>::type>(level)));

        /* 注册日志 */
        spdlog::register_logger(pLogger);
    }
    catch (const spdlog::spdlog_ex& ex)
    {
        std::cout << "Failed to init Logger module, " << ex.what() << std::endl;
        return false;
    }

    return true;
}

void Logger::Deinit()
{
    spdlog::drop_all();
    spdlog::shutdown();
}