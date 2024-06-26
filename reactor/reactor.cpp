#include <string>
#include <thread>
#include <chrono>

#include "reactor.hpp"

using namespace utility;
using namespace reactor;

void Reactor::Init(IniConfigParser *config)
{
    /* 初始化事件源 */
    pSource_ = std::make_unique<Source>(config);

    /* 处理池       */
    pProcessor_ = std::make_unique<Processor>(config);

    /* 初始化设备列表 */
    // 初始化
    deviceList_.emplace(1 , std::make_unique<OpticalfiberCommDev>(config, "OPTICALFIBER101"));            // 光纤通信设备
    deviceList_.emplace(2 , std::make_unique<RadiodigitalCommDev>(config, "RADIODIGITAL206"));            // 电台数传设备
    deviceList_.emplace(3 , std::make_unique<UnderwaterAcousticCommDev>(config, "UNDERWATERACOUSTIC20")); // 水声通信设备
    deviceList_.emplace(4 , std::make_unique<SatelliteCommDev>(config, "SATELLITE2"));                    // 卫星通信设备
    deviceList_.emplace(5 , std::make_unique<OpticalfiberCommDev>(config, "OPTICALFIBER100"));            // 光纤通信设备
    deviceList_.emplace(6 , std::make_unique<RadiodigitalCommDev>(config, "RADIODIGITAL205"));            // 电台数传设备
    deviceList_.emplace(7 , std::make_unique<UnderwaterAcousticCommDev>(config, "UNDERWATERACOUSTIC10")); // 水声通信设备
    deviceList_.emplace(8 , std::make_unique<SatelliteCommDev>(config, "SATELLITE1"));                    // 卫星通信设备
    
    // 绑定可读事件源
    auto func = [this] (DeviceEvent event) {
                    if (event.type_ == DeviceEvent::EventType::READYREAD) {
                        event.ModifyType(DeviceEvent::EventType::READ);
                        Push(event);
                    } else {
                        log_error("Reator绑定可读事件源收到非可读事件, event[{},{}] from-{}-to-{}", 
                            event.id_, event.EventTypeMapping[event.type_], event.device_, event.action_);
                    } };
    for(auto &pdev : deviceList_)
    {
        (pdev.second)->BindReadableEvent2Source(func);
    }
}

void Reactor::Push(DeviceEvent event)
{
    std::string eventMsg = event.Serial();
    pSource_->PushIn(eventMsg);
}

void Reactor::Listen()
{
    log_info("创建子线程：监听事件源");

    std::thread th(
        [this]
        {
            for(;;)
            {
                // 从事件源读取事件
                std::string eventMsg = pSource_->PopIn();

                // 包装成事件
                DeviceEvent event;
                if(!event.Parse(eventMsg))
                {
                    log_error("Reactor recv error event");
                    continue;
                }

                // 将事件添加到处理池
                auto fut = pProcessor_->Submit(
                    [this] (DeviceEvent event)
                    {
                        log_info("线程池接收到任务{},设备{},类型{},动作{},状态{}", 
                                event.id_, event.device_, event.EventTypeMapping[event.type_], event.action_, event.status_);
                        
                        // 执行并反馈结果
                        auto result = deviceList_[event.device_]->HandleEvent(event);
                        if (result.status_) {
                            event.ModifyStatus("success");
                        } else {
                            event.ModifyStatus("fail");
                        }
                        if (event.type_ == DeviceEvent::EventType::READ) {
                            event.ModifyAction(result.fileName_);
                        }
                        
                        Event otherObj;
                        otherObj.Add("sched_time", result.schedTime_);
                        otherObj.Add("sched_finish_time", result.schedFinishTime_);
                        otherObj.Add("trans_bytes", std::to_string(result.transBytes_));
                        otherObj.Add("recv_bytes", std::to_string(result.recvBytes_));
                        otherObj.Add("file_full_path", result.fileFullPath_);
                        otherObj.Add("file_path", result.filePath_);
                        otherObj.Add("file_name", result.fileName_);
                        otherObj.Add("file_size", std::to_string(result.fileSize_));
                        event.ModifyOther(otherObj.Serial());
                        pSource_->PushOut(event.Serial());

                        log_info("线程池处理完成任务{},设备{},类型{},动作{},状态{}", 
                                event.id_, event.device_, event.EventTypeMapping[event.type_], event.action_, event.status_);
                    },
                    event
                );
            }
        }
    );
    // th.detach();
    th.join();
}

