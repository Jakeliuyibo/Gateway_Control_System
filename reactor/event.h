#pragma once

#include <string>
#include <boost/json.hpp>
#include "logger.h"

using namespace std;
using namespace boost::json;

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
            bool add(string key, string val);
            // 获取值
            bool get(string key, string &val);
        private:
            boost::json::object m_obj;
    };

    class DeviceEvent : Event
    {
        enum EventType
        {
            EVENT_INIT  = 0x1,   // 事件：初始化设备
            EVENT_READ  = 0x2,   // 事件：读取设备
            EVENT_WRITE = 0x3,   // 事件：写入设备
            EVENT_OTHER = 0x4,   // 其他事件
        };

        
    }


    // 序列化
    string to_str(Event &event);

    // 反序列化
    Event * from_str(string str);
}