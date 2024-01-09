#include "commdevice.h"

#include <iostream>
#include <cstdlib>
#include <ctime>


using namespace utility;
using namespace driver;
using namespace reactor;

void CommDevice::close()
{
    if (p_filetransfer)
    {
        /* 释放资源 */
        delete p_filetransfer;
        p_filetransfer = nullptr;

        f_open = false;
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

CommDevice::HandleEventRetType CommDevice::handleEvent(DeviceEvent event)
{
    HandleEventRetType ret(true);
    FileTransfer::ftret_type ft_ret(true);

    ret.sched_time = getSystimeUs();

    switch(event.m_type)
    {
        case DeviceEvent::EVENT_OPEN:            /* 创建文件传输对象 */
            if (f_open) {
                log_warning("重复打开设备({}:{})", m_devid, m_devidentify);
                ret.status = false;
                break;
            }

            log_info("通信设备({}:{})初始化", m_devid, m_devidentify);
            open();
            f_open = true;
            break;

        case DeviceEvent::EVENT_WRITE:            /* 传输文件       */
            if (!f_open) {
                log_error("向设备({}:{})写入文件前未打开设备", m_devid, m_devidentify);
                ret.status = false;
                break;
            }
            
            std::this_thread::sleep_for(std::chrono::microseconds(addNoise(m_corrected_delay_us)));
            log_info("通信设备({}:{})传输文件", m_devid, m_devidentify);
            ft_ret              = p_filetransfer->transfer(event.m_action);
            ret.status          = ft_ret.status;
            ret.trans_bytes     = ft_ret.trans_bytes;
            ret.recv_bytes      = ft_ret.recv_bytes;
            ret.file_full_path  = ft_ret.file_full_path;
            ret.file_path       = ft_ret.file_path;
            ret.file_name       = ft_ret.file_name;
            ret.file_size       = ft_ret.file_size;
            break;

        case DeviceEvent::EVENT_READ:            /* 读取文件        */
            if (!f_open) {
                log_error("向设备({}:{})读取文件前未打开设备", m_devid, m_devidentify);
                ret.status = false;
                break;
            }

            std::this_thread::sleep_for(std::chrono::microseconds(addNoise(m_corrected_delay_us)));
            log_info("通信设备({}:{})读取文件", m_devid, m_devidentify);
            ft_ret              = p_filetransfer->receive(m_storagepath, m_storageextentprefix, getSystimeByFilenameFormat());
            ret.status          = ft_ret.status;
            ret.trans_bytes     = ft_ret.trans_bytes;
            ret.recv_bytes      = ft_ret.recv_bytes;
            ret.file_full_path  = ft_ret.file_full_path;
            ret.file_path       = ft_ret.file_path;
            ret.file_name       = ft_ret.file_name;
            ret.file_size       = ft_ret.file_size;
            break;

        case DeviceEvent::EVENT_CLOSE:          /* 关闭文件传输对象  */
            if (!f_open) {
                log_warning("重复关闭设备({}:{})", m_devid, m_devidentify);
                break;
            }

            log_info("通信设备({}:{})关闭传输", m_devid, m_devidentify);
            close();
            break;

        case DeviceEvent::EVENT_POWER_ON:
        case DeviceEvent::EVENT_POWER_OFF:
        case DeviceEvent::EVENT_CONFIG:
        case DeviceEvent::EVENT_READYREAD:
        case DeviceEvent::EVENT_OTHER:
        default:
            ret.status = false;
            log_error("通信设备({}:{})收到未知事件, event = {}", m_devid, m_devidentify, event.EventTypeMapping[event.m_type]);
            break;
    }

    ret.sched_finish_time = getSystimeUs();
    return ret;
}
