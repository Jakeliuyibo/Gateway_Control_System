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
            Serial(IniConfigParser *config)
            {
                log_critical("serial");
            }
            ~Serial()
            {
                // critical("~serial");
            }
            int open()
            {
                return 0;
            }
        private:
            int a;
    };

}