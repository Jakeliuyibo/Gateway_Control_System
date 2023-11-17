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
            void add(std::string key, std::string val);
            // 获取值
            std::string get(std::string key);
            // 序列化
            std::string serial();
            // 反序列化
            void deserial(std::string ser);
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
                EVENT_INIT      = 0x1,      // 事件：初始化设备
                EVENT_WRITE     = 0x2,      // 事件：写入设备
                EVENT_READ      = 0x3,      // 事件：读取设备
                EVENT_READYREAD = 0x4,      // 事件：可读设备
                EVENT_CLOSE     = 0x5,      // 事件：注销设备
                EVENT_OTHER     = 0x6       // 其他事件
            };

            std::unordered_map<EventType, std::string> EventTypeMapping = {
                {EVENT_INIT      , "init"},
                {EVENT_WRITE     , "write"},
                {EVENT_READ      , "read"},
                {EVENT_READYREAD , "readyread"},
                {EVENT_CLOSE     , "close"},
                {EVENT_OTHER     , "other"}
            };

            // 构造
            DeviceEvent() {}
            DeviceEvent(int id, EventType type, std::string device, std::string action);
            // 析构
            ~DeviceEvent(){}
            // 解析
            bool parse(const std::string &ser);
            // 修改
            void modify_id(int new_id);
            void modify_type(EventType new_type);
            void modify_device(std::string new_device);
            void modify_action(std::string new_action);
        public:
            int         m_id;           // ID
            EventType   m_type;         // 事件类型
            std::string m_device;       // 设备
            std::string m_action;       // 动作
    };


}