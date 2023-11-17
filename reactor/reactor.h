#pragma once

#include <memory>
#include <map>
#include "singleton.h"
#include "configparser.h"
#include "logger.h"
#include "source.h"
#include "event.h"
#include "processor.h"
#include "commdevice.h"

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
            std::map<std::string, std::unique_ptr<CommDevice>> m_devicelist;
    };
}