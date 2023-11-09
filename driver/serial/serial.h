#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>
#include "configparser.h"
#include "logger.h"

using namespace utility;
using namespace boost::asio;

namespace driver
{
    class Serial
    {
        public:
            // 构造
            Serial(IniConfigParser *parser);
            // 析构
            ~Serial();
            // 读取
            void read();
            // 写入
            void write(const std::string &data);
            // 关闭
            void close();
        private:
            // 打开
            bool open();
        private:
            std::string   m_portname;
            int           m_baudrate;
            io_service    m_ios;
            serial_port  *p_serialport;
    };

}