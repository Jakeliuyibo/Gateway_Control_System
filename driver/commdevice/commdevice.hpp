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
#include "filetransfer.hpp"
#include "event.hpp"

using namespace utility;
using namespace reactor;

namespace driver
{
    class CommDevice
    {
    public:
        using CallBackType = std::function<void(DeviceEvent event)>;

        struct HandleEventRetType
        {
            bool status_;                  // 执行状态
            std::string schedTime_;        // 调度时间
            std::string schedFinishTime_;  // 调度完成时间
            std::size_t transBytes_;       // 发送字节
            std::size_t recvBytes_;        // 接收字节
            std::string fileFullPath_;     // 文件完整路径
            std::string filePath_;         // 文件路径
            std::string fileName_;         // 文件名
            std::size_t fileSize_;         // 文件大小

            HandleEventRetType(bool st) :
                status_(st),
                schedTime_(""), schedFinishTime_(""),
                transBytes_(0), recvBytes_(0),
                fileFullPath_(""), filePath_(""), fileName_(""), fileSize_(0) {}
        };

    public:
        CommDevice() : isOpen_(false) {}
        virtual ~CommDevice() = default;
        virtual void Open() = 0;
        void Close();
        inline void BindReadableEvent2Source(const CallBackType& func)
        {
            serverReabableCbkFunc_ = func;
        }
        inline bool IsOpen()
        {
            return isOpen_.load();
        }
        HandleEventRetType HandleEvent(DeviceEvent event);

    public:
        int             devId_;
        std::string     devIdentify_;
        std::string     storagePath_;
        std::string     storageExtentPrefix_;
        FileTransfer   *pFileTransfer_;
        CallBackType    serverReabableCbkFunc_;
        int             correctedDelayUs_;

        std::atomic<bool>   isOpen_;
    };

    /**
     * @description: 光纤通信设备
     */
    class OpticalfiberCommDev : public CommDevice
    {
    public:
        OpticalfiberCommDev(IniConfigParser* configParser, std::string configItem);
        ~OpticalfiberCommDev();
        void Open();
    private:
        unsigned short  serverPort_;
        std::string     targetIp_;
        unsigned short  targetPort_;
    };

    /**
     * @description: 电台数传通信设备
     */
    class RadiodigitalCommDev : public CommDevice
    {
    public:
        RadiodigitalCommDev(IniConfigParser* configParser, std::string configItem);
        ~RadiodigitalCommDev();
        void Open();
    private:
        unsigned short  serverPort_;
        std::string     targetIp_;
        unsigned short  targetPort_;
    };

    /**
     * @description: 水声通信设备
     */
    class UnderwaterAcousticCommDev : public CommDevice
    {
    public:
        UnderwaterAcousticCommDev(IniConfigParser* configParser, std::string configItem);
        ~UnderwaterAcousticCommDev();
        void Open();
    private:
        std::string  serialPort_;
    };

    /**
     * @description: 卫星通信设备
     */
    class SatelliteCommDev : public CommDevice
    {
    public:
        SatelliteCommDev(IniConfigParser* configParser, std::string configItem);
        ~SatelliteCommDev();
        void Open();
    private:
        std::string  serialPort_;
    };
}