#include <iostream>

#include "event.hpp"

using namespace reactor;

/*************************************************************************
 *
 * Event
 *
 *************************************************************************/
// 添加属性
void Event::add(std::string key, std::string val)
{
    m_obj[key] = val;
}

// 获取值
std::string Event::get(std::string key)
{
    if(!m_obj.isMember(key))
    {
        log_error("Event Json-Obj not contains key {}", key);
        return "";
    }

    return m_obj[key].asString();
}

// 序列化
std::string Event::serial()
{
    return m_obj.toStyledString();
}

// 反序列化
void Event::deserial(std::string ser)
{
    Json::Reader reader;
    reader.parse(ser, m_obj);
}


/*************************************************************************
 *
 * DeviceEvent
 *
 *************************************************************************/
// 构造
DeviceEvent::DeviceEvent(int id, EventType type, int device, std::string action, 
            std::string status, std::string other)
    :   m_id(id),
        m_type(type),
        m_device(device),
        m_action(action),
        m_status(status),
        m_other(other)
{
    modify_id(m_id);
    modify_type(m_type);
    modify_device(m_device);
    modify_action(m_action);
    modify_status(m_status);
    modify_other(m_other);
}

bool DeviceEvent::parse(const std::string &ser)
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

    deserial(ser);
    m_id     = std::stoi(get("id"));
    std::string type_desc = get("type");
    for(auto &it : EventTypeMapping)
    {
        if(it.second == type_desc)
        {
            m_type = it.first;
            break;
        }
    }
    m_device = std::stoi(get("device"));
    m_action = get("action");
    m_status = get("status");
    m_other  = get("other");

    return true;
}

// 修改
void DeviceEvent::modify_id(int desc)
{
    add("id"    , std::to_string(desc));
}
void DeviceEvent::modify_type(EventType desc)
{
    add("type"  , EventTypeMapping[desc]);
}
void DeviceEvent::modify_device(int desc)
{
    add("device", std::to_string(desc));
}
void DeviceEvent::modify_action(std::string desc)
{
    add("action", desc);
}
void DeviceEvent::modify_status(std::string desc)
{
    add("status", desc);
}
void DeviceEvent::modify_other(std::string desc)
{
    add("other", desc);
}