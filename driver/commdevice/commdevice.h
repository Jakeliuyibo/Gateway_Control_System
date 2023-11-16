#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include "safequeue.h"
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
            virtual bool handleEvent(DeviceEvent &event) = 0;
            virtual ~CommDevice() = default;
            void close()
            {
                if (p_filetransfer)
                {
                    /* 释放资源 */
                    delete p_filetransfer;
                    p_filetransfer = nullptr;
                }
            }
        public:
            int             m_devid;
            FileTransfer   *p_filetransfer;
    };

    /**
     * @description: 光纤通信设备
     */
    class OpticalfiberCommDev : public CommDevice
    {
        public:
            OpticalfiberCommDev(IniConfigParser *config);
            ~OpticalfiberCommDev();
            bool handleEvent(DeviceEvent &event);
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
            RadiodigitalCommDev(IniConfigParser *config);
            ~RadiodigitalCommDev();
            bool handleEvent(DeviceEvent &event);
    };

    /**
     * @description: 水声通信设备
     */
    class UnderwaterAcousticCommDev : public CommDevice
    {
        public:
            UnderwaterAcousticCommDev(IniConfigParser *config);
            ~UnderwaterAcousticCommDev();
            bool handleEvent(DeviceEvent &event);
    };

    /**
     * @description: 卫星通信设备
     */
    class SatelliteCommDev : public CommDevice
    {
        public:
            SatelliteCommDev(IniConfigParser *config);
            ~SatelliteCommDev();
            bool handleEvent(DeviceEvent &event);
    };
}