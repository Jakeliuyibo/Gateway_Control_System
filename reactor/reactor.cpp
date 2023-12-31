#include <string>
#include <thread>
#include <chrono>

#include "reactor.h"

using namespace utility;
using namespace reactor;

void Reactor::init(IniConfigParser *config)
{
    /* 初始化事件源 */
    p_source = std::make_unique<Source>(config);

    /* 处理池       */
    p_processor = std::make_unique<Processor>(config);

    /* 初始化设备列表 */
    // 初始化
    m_devicelist.emplace(1 , std::make_unique<OpticalfiberCommDev>(config, "OPTICALFIBER101"));            // 光纤通信设备
    m_devicelist.emplace(2 , std::make_unique<RadiodigitalCommDev>(config, "RADIODIGITAL206"));            // 电台数传设备
    m_devicelist.emplace(3 , std::make_unique<UnderwaterAcousticCommDev>(config, "UNDERWATERACOUSTIC20")); // 水声通信设备
    m_devicelist.emplace(4 , std::make_unique<SatelliteCommDev>(config, "SATELLITE2"));                    // 卫星通信设备
    m_devicelist.emplace(5 , std::make_unique<OpticalfiberCommDev>(config, "OPTICALFIBER100"));            // 光纤通信设备
    m_devicelist.emplace(6 , std::make_unique<RadiodigitalCommDev>(config, "RADIODIGITAL205"));            // 电台数传设备
    m_devicelist.emplace(7 , std::make_unique<UnderwaterAcousticCommDev>(config, "UNDERWATERACOUSTIC10")); // 水声通信设备
    m_devicelist.emplace(8 , std::make_unique<SatelliteCommDev>(config, "SATELLITE1"));                    // 卫星通信设备
    
    // 绑定可读事件源
    auto func = [this] (DeviceEvent event) {
                    if (event.m_type == DeviceEvent::EVENT_READYREAD) {
                        event.modify_type(DeviceEvent::EVENT_READ);
                        push(event);
                    } else {
                        log_error("Reator绑定可读事件源收到非可读事件, event[{},{}] from-{}-to-{}", event.m_id, event.m_type, event.m_device, event.m_action);
                    } };
    for(auto &pdev : m_devicelist)
    {
        (pdev.second)->bindReadableEvent2Source(func);
    }
}

void Reactor::push(DeviceEvent event)
{
    std::string event_msg = event.serial();
    p_source->push_in(event_msg);
}

void Reactor::listen()
{
    log_info("创建子线程：监听事件源");

    std::thread th(
        [this]
        {
            for(;;)
            {
                // 从事件源读取事件
                std::string event_msg = p_source->pop_in();

                // 包装成事件
                DeviceEvent event;
                if(!event.parse(event_msg))
                {
                    log_error("Reactor recv error event");
                    continue;
                }

                // 将事件添加到处理池
                auto fut = p_processor->submit(
                    [this] (DeviceEvent event)
                    {
                        log_debug("线程池接收到任务{},设备{},类型{},动作{},状态{},其他{}", 
                                event.m_id, event.m_device, event.EventTypeMapping[event.m_type], event.m_action, event.m_status, event.m_other);
                        
                        // 执行并反馈结果
                        bool sub_result = m_devicelist[event.m_device]->handleEvent(event);
                        if (sub_result) {
                            event.modify_status("success");
                        } else {
                            event.modify_status("fail");
                        }
                        p_source->push_out(event.serial());
                    },
                    event
                );
            }
        }
    );
    // th.detach();
    th.join();
}

