#include <string>
#include <thread>
#include <chrono>

#include "reactor.h"

using namespace utility;
using namespace reactor;

Reactor::Reactor(IniConfigParser *config)
{
    /* 初始化事件源 */
    p_source = std::make_unique<Source>(config);

    /* 处理池       */
    p_processor = std::make_unique<Processor>(config);
}

Reactor::~Reactor()
{
    log_info("reactor module done ...");
}

void thread_func(DeviceEvent event)
{
    log_critical("线程池接收到事件id={},类型type={}, 设备device={}, 动作action={}", 
        event.m_id, event.m_type, event.m_device, event.m_action
        );
}

void Reactor::listen()
{
    /* 测试 */
    std::thread test(
        [this]
        {
            int idx = 0;
            for(;;)
            {
                DeviceEvent event(idx, DeviceEvent::EVENT_WRITE, "2", "haha");
                std::string event_msg = event.serial();
                p_source->push(event_msg);
                idx++;
                std::this_thread::sleep_for(std::chrono::seconds(1));
            }
        }
    );
    test.detach();

    // 创建子线程监听事件源
    std::thread th(
        [this]
        {
            for(;;)
            {
                // 从事件源读取事件
                std::string event_msg = p_source->pop();

                // 包装成事件
                DeviceEvent event;
                if(!event.parse(event_msg))
                {
                    log_error("Reactor recv error event");
                    continue;
                }

                // 将事件添加到处理池
                auto fut = p_processor->submit(thread_func, event);
            }
        }
    );
    // th.detach();
    th.join();

    log_info("Create sub-thread to listen event source");
}

