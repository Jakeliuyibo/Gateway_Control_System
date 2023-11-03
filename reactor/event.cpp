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
        error("Event Json-Obj not contains key {}", key);
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
    add("id"    , std::to_string(m_id));
    add("type"  , EventTypeMapping[m_type]);
    add("device", m_device);
    add("action", m_action);
}

DeviceEvent::DeviceEvent(std::string ser)
{
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
}