#pragma once

#include <memory>
#include <map>

#include "singleton.hpp"
#include "configparser.hpp"
#include "logger.hpp"
#include "commdevice.hpp"
#include "source.hpp"
#include "event.hpp"
#include "processor.hpp"

using namespace utility;
using namespace driver;

namespace reactor
{
    class Reactor
    {
        SINGLETON(Reactor);

        public:
            void init(IniConfigParser *config);
            static Reactor *instance()
            {
                static Reactor ins;
                return &ins;
            }
            void push(DeviceEvent event);
            void listen();
        private:
            std::unique_ptr<Source> p_source;
            std::unique_ptr<Processor> p_processor;
            std::map<int, std::unique_ptr<CommDevice>> m_devicelist;
    };
}