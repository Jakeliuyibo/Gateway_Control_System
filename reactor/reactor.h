#pragma once

#include <memory>
#include "configparser.h"
#include "logger.h"
#include "source.h"
#include "event.h"
#include "processor.h"

using namespace utility;


namespace reactor
{
    class Reactor
    {
        public:
            Reactor(IniConfigParser *config);
            ~Reactor();
            void listen();
        private:
            std::unique_ptr<Source> p_source;
            std::unique_ptr<Processor> p_processor;
    };

}