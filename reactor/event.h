#pragma once

#include <string>
#include <unordered_map>
#include <jsoncpp/json/json.h>
#include "logger.h"

namespace reactor
{
    /**
     * @description: 事件
     */
    class Event
    {
        public:
            // 构造
            Event() {}
            // 析构
            ~Event() {}
            // 添加属性
            void add(std::string key, std::string val)
            {
                m_obj[key] = val;
            }
            // 获取值
            std::string get(std::string key)
            {
                if(!m_obj.isMember(key))
                {
                    error("Event Json-Obj not contains key {}", key);
                    return "";
                }

                return m_obj[key].asString();
            }
            // 序列化
            std::string serial()
            {
                return m_obj.toStyledString();
            }
            // 反序列化
            void deserial(std::string ser)
            {
                Json::Reader reader;
                reader.parse(ser, m_obj);
            }
        private:
            Json::Value m_obj;
    };

    /**
     * @description: 设备事件
     */
    class DeviceEvent : public Event
    {
        public:
            enum EventType
            {
                EVENT_INIT  = 0x1,      // 事件：初始化设备
                EVENT_READ  = 0x2,      // 事件：读取设备
                EVENT_WRITE = 0x3,      // 事件：写入设备
                EVENT_OTHER = 0x4,      // 其他事件
            };

            std::unordered_map<EventType, std::string> EventTypeMapping = {
                {EVENT_INIT , "init"},
                {EVENT_READ , "read"},
                {EVENT_WRITE, "write"},
                {EVENT_OTHER, "other"}
            };

            // 构造
            DeviceEvent(EventType type, std::string device, std::string action)
                :   m_type(type),
                    m_device(device),
                    m_action(action) 
            {
                add("type"  , EventTypeMapping[m_type]);
                add("device", m_device);
                add("action", m_action);
            }
            DeviceEvent(std::string ser)
            {
                deserial(ser);
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

            // 析构
            ~DeviceEvent()
            {

            }
        private:
            EventType   m_type;         // 事件类型
            std::string m_device;       // 设备
            std::string m_action;       // 动作
    };


}