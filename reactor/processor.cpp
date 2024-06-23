#include "processor.hpp"

using namespace utility;
using namespace reactor;


Processor::Processor(IniConfigParser *parser)
{
    bool parserFlag = true;

    /* 解析参数 */
    int num_works = 1; 
    parserFlag &= parser->getValue<int>("PROCESSOR", "PROCESSOR_NUMTHREAD", num_works);

    if(!parserFlag)
    {
        log_error("Init processor failed, when load config");
        return;
    }

    /* 创建线程池 */
    p_pool = std::make_unique<ThreadPool>(num_works);
}


Processor::~Processor()
{
    log_info("reactor-processor module done ...");
}

void Processor::shutdown()
{
    p_pool->shutdown();
}