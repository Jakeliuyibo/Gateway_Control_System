#pragma once

#include <string>
#include "logger.h"
#include <boost/json.hpp>

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
            enum EventType
            {
                EVENT_INIT  = 0x1,   // 事件：初始化设备
                EVENT_READ  = 0x2,   // 事件：读取设备
                EVENT_WRITE = 0x3,   // 事件：写入设备
                EVENT_OTHER = 0x4,   // 其他事件
            };

            // 构造
            Event(EventType type, string dev_to)
                :   m_type(type),
                    m_dev_from(""),
                    m_dev_to(dev_to),
                    m_desc("")
            {

            }
            Event(EventType type, string dev_from, string dev_to)
                :   m_type(type),
                    m_dev_from(dev_from),
                    m_dev_to(dev_to),
                    m_desc("")

            {

            }
            // 析构
            ~Event()
            {

            }
        private:
            boost::json::object m_obj;

            EventType m_type;
            string m_dev_from;
            string m_dev_to;
            string m_desc;
    };

    // 序列化
    string to_str(Event &event);

    // 反序列化
    Event * from_str(string str);
}