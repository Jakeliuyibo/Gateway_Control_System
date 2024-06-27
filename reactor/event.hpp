#pragma once

#include <string>
#include <unordered_map>

#include "jsoncpp/json/json.h"

#include "logger.hpp"

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
            void Add(std::string key, std::string val);
            // 获取值
            std::string Get(std::string key);
            // 序列化
            std::string Serial();
            // 反序列化
            void Deserial(std::string ser);
        private:
            Json::Value obj_;
    };

    /**
     * @description: 设备事件
     */
    class DeviceEvent : public Event
    {
        public:
            enum class EventType
            {
                POWER_ON  = 0x1,      // 上电
                POWER_OFF = 0x2,      // 下电
                OPEN      = 0x3,      // 打开
                CLOSE     = 0x4,      // 关闭
                CONFIG    = 0x5,      // 配置
                WRITE     = 0x6,      // 写入
                READ      = 0x7,      // 读取
                READYREAD = 0x8,      // 可读
                OTHER     = 0x9       // 其他
            };

            std::unordered_map<EventType, std::string> EventTypeMapping = {
                {EventType::POWER_ON , "power on"},
                {EventType::POWER_OFF, "power off"},
                {EventType::OPEN     , "open"},
                {EventType::CLOSE    , "close"},
                {EventType::CONFIG   , "config"},
                {EventType::WRITE    , "write"},
                {EventType::READ     , "read"},
                {EventType::READYREAD, "readyread"},
                {EventType::OTHER    , "other"}
            };

            // 构造和析构
            DeviceEvent() {}
            DeviceEvent(int id, EventType type, 
                        int device, std::string action, 
                        std::string status, std::string other);
            ~DeviceEvent(){}

            // 解析
            bool Parse(const std::string &ser);

            // 修改
            void ModifyId(int desc);
            void ModifyType(EventType desc);
            void ModifyDevice(int desc);
            void ModifyAction(std::string desc);
            void ModifyStatus(std::string desc);
            void ModifyOther(std::string desc);

        public:
            int         id_;           // ID
            EventType   type_;         // 事件类型
            int         device_;       // 设备ID
            std::string action_;       // 动作
            std::string status_;       // 状态
            std::string other_;        // 其他
    };


}