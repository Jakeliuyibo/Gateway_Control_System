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
            void Init(IniConfigParser *config);
            void Push(DeviceEvent event);
            void Listen();
            
        private:
            std::unique_ptr<Source> pSource_;
            std::unique_ptr<Processor> pProcessor_;
            std::map<int, std::unique_ptr<CommDevice>> deviceList_;
    };
}