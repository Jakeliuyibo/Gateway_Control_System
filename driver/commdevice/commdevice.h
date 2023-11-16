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
            virtual bool handleEvent(const Event &event) = 0;
            virtual ~CommDevice() = default;
        public:
            int m_devid;
            std::unique_ptr<FileTransfer>  p_filetransfer;
    };

    /**
     * @description: 光纤通信设备
     */
    class OpticalfiberCommDev : public CommDevice
    {
        public:
            OpticalfiberCommDev(IniConfigParser *config);
            ~OpticalfiberCommDev();
            bool handleEvent(const DeviceEvent &event);
    };

    /**
     * @description: 电台数传通信设备
     */
    class RadiodigitalCommDev : public CommDevice
    {
        public:
            RadiodigitalCommDev(IniConfigParser *config);
            ~RadiodigitalCommDev();
            bool handleEvent(const DeviceEvent &event);
    };

    /**
     * @description: 水声通信设备
     */
    class UnderwaterAcousticCommDev : public CommDevice
    {
        public:
            UnderwaterAcousticCommDev(IniConfigParser *config);
            ~UnderwaterAcousticCommDev();
            bool handleEvent(const DeviceEvent &event);
    };

    /**
     * @description: 卫星通信设备
     */
    class SatelliteCommDev : public CommDevice
    {
        public:
            SatelliteCommDev(IniConfigParser *config);
            ~SatelliteCommDev();
            bool handleEvent(const DeviceEvent &event);
    };
}