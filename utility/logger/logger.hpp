#pragma once
#include <string>
#include <memory>

#include "spdlog/spdlog.h"
#include "spdlog/async.h"
#include "spdlog/spdlog-inl.h"
#include "spdlog/sinks/stdout_color_sinks.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/async_logger.h"
#include "spdlog/sinks/basic_file_sink.h"
#include "spdlog/sinks/rotating_file_sink.h"
#include "spdlog/details/thread_pool.h"
#include "spdlog/details/thread_pool-inl.h"
#include "spdlog/sinks/daily_file_sink.h"

#include "singleton.hpp"

namespace utility
{

    #define LOGGER_NAME "g_logger"

    #define log_trace(...)      SPDLOG_LOGGER_CALL(spdlog::get(LOGGER_NAME) , spdlog::level::trace      , __VA_ARGS__)
    #define log_debug(...)      SPDLOG_LOGGER_CALL(spdlog::get(LOGGER_NAME) , spdlog::level::debug      , __VA_ARGS__)
    #define log_info(...)       SPDLOG_LOGGER_CALL(spdlog::get(LOGGER_NAME) , spdlog::level::info       , __VA_ARGS__)
    #define log_warning(...)    SPDLOG_LOGGER_CALL(spdlog::get(LOGGER_NAME) , spdlog::level::warn       , __VA_ARGS__)
    #define log_error(...)      SPDLOG_LOGGER_CALL(spdlog::get(LOGGER_NAME) , spdlog::level::err        , __VA_ARGS__)
    #define log_critical(...)   SPDLOG_LOGGER_CALL(spdlog::get(LOGGER_NAME) , spdlog::level::critical   , __VA_ARGS__)

    /**
     * @description: 基于spdlog的单例日志器
     */
    class Logger
    {
        // 单例模式
        SINGLETON(Logger);

    public:
        // 日志输出流
        enum class WorkStream
        {
            CONSOLE = 0x1, // 控制台
            FILE = 0x2, // 文件
            BOTH = 0x3, // 控制台 + 文件
        };

        // 输出模式
        enum class WorkMode
        {
            SYNC = 0x1, // 同步
            ASYNC = 0x2, // 异步
        };

        enum class WorkLevel : int
        {
            TRACE = 0x0, // 跟踪
            DEBUG = 0x1, // 调试
            INFO = 0x2, // 信息
            WARNING = 0x3, // 警告
            ERROR = 0x4, // 错误
            CRITICAL = 0x5, // 重要
            OFF = 0x6, // 关闭
        };

    public:
        // 初始化
        [[nodiscard]] bool Init(const std::string& filePath,
            const WorkStream stream = WorkStream::BOTH,
            const WorkMode mode = WorkMode::SYNC,
            const WorkLevel level = WorkLevel::DEBUG,
            const WorkLevel levelConsole = WorkLevel::WARNING,
            const WorkLevel levelFile = WorkLevel::DEBUG
        );

        // 缺省
        void Deinit();
    
    private:
        // 工作队列大小
        static constexpr int kWorkQueueSize = 8192;
        // 工作线程数量
        static constexpr int kWorkThreadCount = 3; 
        // 终端输出格式
        inline static const std::string kConsoleFormat = "[%^%l%$] %v";
        // 文件输出格式
        inline static const std::string kFileFormat = "[%Y-%m-%d %T.%e] <pid %P:%t> [%^%l%$] [%s:%!:%#] %v";
    };
}