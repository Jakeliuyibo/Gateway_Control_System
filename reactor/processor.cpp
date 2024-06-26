#include "processor.hpp"

using namespace utility;
using namespace reactor;


Processor::Processor(IniConfigParser *parser)
{
    bool parserFlag = true;

    /* 解析参数 */
    int numWorks = 1; 
    parserFlag &= parser->GetValue<int>("PROCESSOR", "PROCESSOR_NUMTHREAD", numWorks);

    if(!parserFlag)
    {
        log_error("Init processor failed, when load config");
        return;
    }

    /* 创建线程池 */
    pool_ = std::make_unique<ThreadPool>(numWorks);
}


Processor::~Processor()
{
    log_info("reactor-processor module done ...");
}

void Processor::Shutdown()
{
    pool_->Shutdown();
}