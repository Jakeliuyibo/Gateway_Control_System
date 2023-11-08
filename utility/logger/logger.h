#pragma once
#include <string>
#include <memory>
#include <spdlog/spdlog.h>
#include <spdlog/async.h>
#include <spdlog/spdlog-inl.h>
#include <spdlog/sinks/stdout_color_sinks.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/async_logger.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/rotating_file_sink.h>
#include <spdlog/details/thread_pool.h>
#include <spdlog/details/thread_pool-inl.h>
#include <spdlog/sinks/daily_file_sink.h>

#include "singleton.h"

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
            enum WorkStream
            {
                STREAM_CONSOLE = 0x1,   // 控制台
                STREAM_FILE    = 0x2,   // 文件
                STREAM_BOTH    = 0x3,   // 控制台 + 文件
            };

            enum WorkMode
            {
                MODE_SYNC      = 0x1,   // 同步
                MODE_ASYNC     = 0x2,   // 异步
            };

            enum WorkLevel
            {
                LEVEL_TRACE    = 0x0,   // 跟踪
                LEVEL_DEBUG    = 0x1,   // 调试
                LEVEL_INFO     = 0x2,   // 信息
                LEVEL_WARNING  = 0x3,   // 警告
                LEVEL_ERROR    = 0x4,   // 错误
                LEVEL_CRITICAL = 0x5,   // 重要
                LEVEL_OFF      = 0x6,   // 关闭
            };

        public:
            static Logger *instance()
            {
                static Logger ins;
                return &ins;
            }

            bool init(  const std::string &filepath,
                        const WorkStream work_stream = STREAM_BOTH,
                        const WorkMode work_mode = MODE_SYNC, 
                        const WorkLevel work_level = LEVEL_DEBUG,
                        const WorkLevel level_console = LEVEL_WARNING,
                        const WorkLevel level_file = LEVEL_DEBUG
                        );

            void deinit();
        private:
            std::shared_ptr<spdlog::logger> m_pLogger;
    };
}