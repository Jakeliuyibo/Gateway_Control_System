#pragma once

#include <string>
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


    class DeviceEvent : Event
    {
        public:
            enum EventType
            {
                EVENT_INIT  = 0x1,   // 事件：初始化设备
                EVENT_READ  = 0x2,   // 事件：读取设备
                EVENT_WRITE = 0x3,   // 事件：写入设备
                EVENT_OTHER = 0x4,   // 其他事件
            };

            DeviceEvent();
            ~DeviceEvent();
            
        private:
            
    };

        // // 反序列化
        // Event * from_str(std::string str);

}