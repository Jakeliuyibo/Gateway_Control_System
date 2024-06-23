#pragma once

#include <string>
#include <memory>
#include <thread>
#include <chrono>
#include <atomic>

#include "boost/asio.hpp"
#include "boost/filesystem.hpp"

#include "systime.hpp"
#include "logger.hpp"
#include "configparser.hpp"
#include "event.hpp"
#include "filetransfer.hpp"

using namespace utility;
using namespace reactor;

namespace driver
{
    class CommDevice
    {
        public:
            using callback_type = std::function<void(DeviceEvent event)>;

            struct HandleEventRetType 
            {
                bool status;                    // 执行状态
                std::string sched_time;         // 调度时间
                std::string sched_finish_time;  // 调度完成时间
                std::size_t trans_bytes;        // 发送字节
                std::size_t recv_bytes;         // 接收字节
                std::string file_full_path;     // 文件完整路径
                std::string file_path;          // 文件路径
                std::string file_name;          // 文件名
                std::size_t file_size;          // 文件大小

                HandleEventRetType(bool st) : 
                    status(st), 
                    sched_time(""), sched_finish_time(""),
                    trans_bytes(0), recv_bytes(0), 
                    file_full_path(""), file_path(""), file_name(""), file_size(0) {}
            };

        public:
            CommDevice() : f_open(false) {}
            virtual ~CommDevice() = default;
            virtual void open() = 0;
            void close();
            inline void bindReadableEvent2Source(const callback_type &func)
            {
                f_serverreable_cb = func;
            }
            inline bool is_open()
            {
                return f_open.load();
            }
            HandleEventRetType handleEvent(DeviceEvent event);
        public:
            int             m_devid;
            std::string     m_devidentify;
            std::string     m_storagepath;
            std::string     m_storageextentprefix;
            FileTransfer   *p_filetransfer;
            callback_type   f_serverreable_cb;
            int             m_corrected_delay_us;

            std::atomic<bool>   f_open;
    };

    /**
     * @description: 光纤通信设备
     */
    class OpticalfiberCommDev : public CommDevice
    {
        public:
            OpticalfiberCommDev(IniConfigParser *config, std::string config_item);
            ~OpticalfiberCommDev();
            void open();
        private:
            unsigned short  m_serverport;
            std::string     m_targetip;
            unsigned short  m_targetport;
    };

    /**
     * @description: 电台数传通信设备
     */
    class RadiodigitalCommDev : public CommDevice
    {
        public:
            RadiodigitalCommDev(IniConfigParser *config, std::string config_item);
            ~RadiodigitalCommDev();
            void open();
        private:
            unsigned short  m_serverport;
            std::string     m_targetip;
            unsigned short  m_targetport;
    };

    /**
     * @description: 水声通信设备
     */
    class UnderwaterAcousticCommDev : public CommDevice
    {
        public:
            UnderwaterAcousticCommDev(IniConfigParser *config, std::string config_item);
            ~UnderwaterAcousticCommDev();
            void open();
        private:
            std::string  m_serialport;
    };

    /**
     * @description: 卫星通信设备
     */
    class SatelliteCommDev : public CommDevice
    {
        public:
            SatelliteCommDev(IniConfigParser *config, std::string config_item);
            ~SatelliteCommDev();
            void open();
        private:
            std::string  m_serialport;
    };
}