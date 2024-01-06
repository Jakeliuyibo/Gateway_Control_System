#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include <atomic>
#include "systime.h"
#include "logger.h"
#include "configparser.h"
#include "event.h"
#include "filetransfer.h"

using namespace utility;
using namespace reactor;

namespace driver
{
    class CommDevice
    {
        public:
            using callback_type = std::function<void(DeviceEvent event)>;

        public:
            CommDevice() : f_open(false) {}
            virtual ~CommDevice() = default;
            virtual bool handleEvent(DeviceEvent event) = 0;
            void close()
            {
                if (p_filetransfer)
                {
                    /* 释放资源 */
                    delete p_filetransfer;
                    p_filetransfer = nullptr;

                    f_open = false;
                }
            }
            void bindReadableEvent2Source(const callback_type &func)
            {
                f_serverreable_cb = func;
            }
            bool is_open()
            {
                return f_open.load();
            }
        public:
            int             m_devid;
            std::string     m_devidentify;
            std::string     m_storagepath;
            std::string     m_storageextentprefix;
            FileTransfer   *p_filetransfer;
            callback_type   f_serverreable_cb;

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
            bool handleEvent(DeviceEvent event);
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
            bool handleEvent(DeviceEvent event);
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
            bool handleEvent(DeviceEvent event);
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
            bool handleEvent(DeviceEvent event);
        private:
            std::string  m_serialport;
    };
}