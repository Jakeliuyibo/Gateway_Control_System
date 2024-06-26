#include <iostream>

#include "event.hpp"

using namespace reactor;

/*************************************************************************
 *
 * Event
 *
 *************************************************************************/
// 添加属性
void Event::Add(std::string key, std::string val)
{
    obj_[key] = val;
}

// 获取值
std::string Event::Get(std::string key)
{
    if(!obj_.isMember(key))
    {
        log_error("Event Json-Obj not contains key {}", key);
        return "";
    }

    return obj_[key].asString();
}

// 序列化
std::string Event::Serial()
{
    return obj_.toStyledString();
}

// 反序列化
void Event::Deserial(std::string ser)
{
    Json::Reader reader;
    reader.parse(ser, obj_);
}


/*************************************************************************
 *
 * DeviceEvent
 *
 *************************************************************************/
// 构造
DeviceEvent::DeviceEvent(int id, EventType type, int device, std::string action, 
            std::string status, std::string other)
    :   id_(id),
        type_(type),
        device_(device),
        action_(action),
        status_(status),
        other_(other)
{
    ModifyId(id_);
    ModifyType(type_);
    ModifyDevice(device_);
    ModifyAction(action_);
    ModifyStatus(status_);
    ModifyOther(other_);
}

bool DeviceEvent::Parse(const std::string &ser)
{
    if(     ser.find("id")      == std::string::npos
        ||  ser.find("type")    == std::string::npos
        ||  ser.find("device")  == std::string::npos
        ||  ser.find("action")  == std::string::npos
        ||  ser.find("status")  == std::string::npos
        ||  ser.find("other")   == std::string::npos
        )
    {
        log_error("Can't detect sybmol from event msg: {}", ser);
        return false;
    }

    Deserial(ser);
    id_ = std::stoi(Get("id"));
    std::string typeDesc = Get("type");
    for(auto &it : EventTypeMapping)
    {
        if(it.second == typeDesc)
        {
            type_ = it.first;
            break;
        }
    }
    device_ = std::stoi(Get("device"));
    action_ = Get("action");
    status_ = Get("status");
    other_  = Get("other");

    return true;
}

// 修改
void DeviceEvent::ModifyId(int desc)
{
    Add("id"    , std::to_string(desc));
}
void DeviceEvent::ModifyType(EventType desc)
{
    Add("type"  , EventTypeMapping[desc]);
}
void DeviceEvent::ModifyDevice(int desc)
{
    Add("device", std::to_string(desc));
}
void DeviceEvent::ModifyAction(std::string desc)
{
    Add("action", desc);
}
void DeviceEvent::ModifyStatus(std::string desc)
{
    Add("status", desc);
}
void DeviceEvent::ModifyOther(std::string desc)
{
    Add("other", desc);
}