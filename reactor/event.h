#pragma once

#include <string>
#include <unordered_map>

#include "jsoncpp/json/json.h"

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
            virtual ~Event() {}
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
                EVENT_POWER_ON  = 0x1,      // 上电
                EVENT_POWER_OFF = 0x2,      // 下电
                EVENT_OPEN      = 0x3,      // 打开
                EVENT_CLOSE     = 0x4,      // 关闭
                EVENT_CONFIG    = 0x5,      // 配置
                EVENT_WRITE     = 0x6,      // 写入
                EVENT_READ      = 0x7,      // 读取
                EVENT_READYREAD = 0x8,      // 可读
                EVENT_OTHER     = 0x9       // 其他
            };

            std::unordered_map<EventType, std::string> EventTypeMapping = {
                {EVENT_POWER_ON     , "power on"},
                {EVENT_POWER_OFF    , "power off"},
                {EVENT_OPEN         , "open"},
                {EVENT_CLOSE        , "close"},
                {EVENT_CONFIG       , "config"},
                {EVENT_WRITE        , "write"},
                {EVENT_READ         , "read"},
                {EVENT_READYREAD    , "readyread"},
                {EVENT_OTHER        , "other"}
            };

            // 构造
            DeviceEvent() {}
            DeviceEvent(int id, EventType type, 
                        int device, std::string action, 
                        std::string m_status, std::string m_other);
            // 析构
            ~DeviceEvent(){}
            // 解析
            bool parse(const std::string &ser);
            // 修改
            void modify_id(int desc);
            void modify_type(EventType desc);
            void modify_device(int desc);
            void modify_action(std::string desc);
            void modify_status(std::string desc);
            void modify_other(std::string desc);
        public:
            int         m_id;           // ID
            EventType   m_type;         // 事件类型
            int         m_device;       // 设备ID
            std::string m_action;       // 动作
            std::string m_status;       // 状态
            std::string m_other;        // 其他
    };


}