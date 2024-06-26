#include <iostream>
#include <cstdlib>
#include <ctime>

#include "commdevice.hpp"

using namespace utility;
using namespace driver;
using namespace reactor;

void CommDevice::Close()
{
    if (pFileTransfer_)
    {
        /* 释放资源 */
        delete pFileTransfer_;
        pFileTransfer_ = nullptr;

        isOpen_ = false;
    }
}

int addNoise(int originalValue) 
{
    // 计算噪声范围
    int noiseRange = static_cast<int>(originalValue * 0.05);

    // 生成随机噪声
    int noise = std::rand() % noiseRange - noiseRange / 2;

    // 返回添加了噪声的值
    return originalValue + noise;
}

CommDevice::HandleEventRetType CommDevice::HandleEvent(DeviceEvent event)
{
    HandleEventRetType ret(true);
    FileTransfer::FtRetType ft_ret(true);

    ret.schedTime_ = GetSystimeUs();

    switch(event.type_)
    {
        case DeviceEvent::EventType::OPEN:            /* 创建文件传输对象 */
            if (isOpen_) {
                log_warning("重复打开设备({}:{})", devId_, devIdentify_);
                ret.status_ = false;
                break;
            }

            log_info("通信设备({}:{})初始化", devId_, devIdentify_);
            Open();
            isOpen_ = true;
            break;

        case DeviceEvent::EventType::WRITE:            /* 传输文件       */
            if (!isOpen_) {
                log_error("向设备({}:{})写入文件前未打开设备", devId_, devIdentify_);
                ret.status_ = false;
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::microseconds(addNoise(correctedDelayUs_)));
            log_info("通信设备({}:{})传输文件", devId_, devIdentify_);
            ft_ret              = pFileTransfer_->Transfer(event.action_);
            ret.status_         = ft_ret.status_;
            ret.transBytes_     = ft_ret.transBytes_;
            ret.recvBytes_      = ft_ret.recvBytes_;
            ret.fileFullPath_   = ft_ret.fileFullPath_;
            ret.filePath_       = ft_ret.filePath_;
            ret.fileName_       = ft_ret.fileName_;
            ret.fileSize_       = ft_ret.fileSize_;
            break;

        case DeviceEvent::EventType::READ:            /* 读取文件        */
            if (!isOpen_) {
                log_error("向设备({}:{})读取文件前未打开设备", devId_, devIdentify_);
                ret.status_ = false;
                break;
            }

            std::this_thread::sleep_for(std::chrono::microseconds(addNoise(correctedDelayUs_)));
            log_info("通信设备({}:{})读取文件", devId_, devIdentify_);
            ft_ret              = pFileTransfer_->Receive(storagePath_, storageExtentPrefix_, GetSystimeByFilenameFormat());
            ret.status_         = ft_ret.status_;
            ret.transBytes_     = ft_ret.transBytes_;
            ret.recvBytes_      = ft_ret.recvBytes_;
            ret.fileFullPath_   = ft_ret.fileFullPath_;
            ret.filePath_       = ft_ret.filePath_;
            ret.fileName_       = ft_ret.fileName_;
            ret.fileSize_       = ft_ret.fileSize_;
            break;

        case DeviceEvent::EventType::CLOSE:          /* 关闭文件传输对象  */
            if (!isOpen_) {
                log_warning("重复关闭设备({}:{})", devId_, devIdentify_);
                break;
            }

            log_info("通信设备({}:{})关闭传输", devId_, devIdentify_);
            Close();
            break;

        case DeviceEvent::EventType::POWER_ON:
        case DeviceEvent::EventType::POWER_OFF:
        case DeviceEvent::EventType::CONFIG:
        case DeviceEvent::EventType::READYREAD:
        case DeviceEvent::EventType::OTHER:
        default:
            ret.status_ = false;
            log_error("通信设备({}:{})收到未知事件, event = {}", devId_, devIdentify_, event.EventTypeMapping[event.type_]);
            break;
    }

    ret.schedFinishTime_ = GetSystimeUs();
    return ret;
}
