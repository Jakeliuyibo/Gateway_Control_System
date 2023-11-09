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
            void read();
            void write(const std::string &data);
            void close();
        private:
            bool open(const std::string &port, int bard_rate);
        private:
            //io_service Object
            io_service m_ios;
        
            //Serial port Object
            serial_port *p_serialport;
            
            //Serial_port function exception
            boost::system::error_code ec;
    };

}