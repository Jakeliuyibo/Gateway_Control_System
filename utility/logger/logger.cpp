#include <iostream>
#include <vector>

#include "logger.h"

using namespace utility;


bool Logger::init(  const std::string &filepath,
                    const WorkStream work_stream,
                    const WorkMode work_mode,
                    const WorkLevel work_level,
                    const WorkLevel level_console,
                    const WorkLevel level_file
            )
{
    try
    {
        std::vector<spdlog::sink_ptr> vecSink;

        /* 输出到控制台 */
        if (work_stream & STREAM_CONSOLE)
        {
            auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
            const char * console_format = "[%^%l%$] %v";
            console_sink->set_pattern(console_format);
            console_sink->set_level((spdlog::level::level_enum)level_console);
            vecSink.emplace_back(console_sink);
        }

        /* 输出到文件 */
        if (work_stream & STREAM_FILE)
        {
            // auto file_sink = std::make_shared<spdlog::sinks::rotating_file_sink_mt>(filepath, 10 * 1024 * 1024, 5);
            auto file_sink = std::make_shared<spdlog::sinks::daily_file_sink_mt>(filepath, 23, 59);
            const char * file_format = "[%Y-%m-%d %T.%e] <pid %P:%t> [%^%l%$] [%s:%!:%#] %v";
            // const char * file_format = "[%Y-%m-%d %T.%e] [%^%l%$] %v";
            file_sink->set_pattern(file_format);
            file_sink->set_level((spdlog::level::level_enum)level_file);
            vecSink.emplace_back(file_sink);
        }

        /* 设置同步异步 */
        if (work_mode & MODE_SYNC)
        {
            m_pLogger = std::make_shared<spdlog::logger>(LOGGER_NAME, begin(vecSink), end(vecSink));
        }
        else if (work_mode & MODE_ASYNC)
        {
            // 创建队列大小10000，线程为1的工作线程池
            spdlog::init_thread_pool(10000, 1);
            auto tp = spdlog::thread_pool();
            m_pLogger = std::make_shared<spdlog::async_logger>(LOGGER_NAME, begin(vecSink), end(vecSink), tp, spdlog::async_overflow_policy::block);
            // spdlog::flush_every(std::chrono::seconds(1));
        }
        m_pLogger->set_level((spdlog::level::level_enum)work_level);
		spdlog::register_logger(m_pLogger);
    }
    catch(const spdlog::spdlog_ex& ex)
    {
        std::cout << "Failed to init Logger module, " << ex.what() << std::endl;
        return false;
    }

    return true;
}



void Logger::deinit()
{
	spdlog::drop_all();
	spdlog::shutdown();
}