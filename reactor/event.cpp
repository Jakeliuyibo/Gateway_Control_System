#include "event.h"

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
DeviceEvent::DeviceEvent(int id, EventType type, std::string device, std::string action)
    :   m_id(id),
        m_type(type),
        m_device(device),
        m_action(action) 
{
    modify_id(m_id);
    modify_type(m_type);
    modify_device(m_device);
    modify_action(m_action);
}

bool DeviceEvent::parse(const std::string &ser)
{
    if(     ser.find("id")      == std::string::npos
        ||  ser.find("type")    == std::string::npos
        ||  ser.find("device")  == std::string::npos
        ||  ser.find("action")  == std::string::npos)
    {
        log_error("Can't detect sybmol from event msg: {}", ser);
        return false;
    }

    deserial(ser);
    m_id     = std::stoi(get("id"));
    m_device = get("device");
    m_action = get("action");
    std::string type_desc = get("type");
    for(auto &it : EventTypeMapping)
    {
        if(it.second == type_desc)
        {
            m_type = it.first;
            break;
        }
    }
    return true;
}

// 修改
void DeviceEvent::modify_id(int new_id)
{
    add("id"    , std::to_string(new_id));
}
void DeviceEvent::modify_type(EventType new_type)
{
    add("type"  , EventTypeMapping[new_type]);
}
void DeviceEvent::modify_device(std::string new_device)
{
    add("device", new_device);
}
void DeviceEvent::modify_action(std::string new_action)
{
    add("action", new_action);
}