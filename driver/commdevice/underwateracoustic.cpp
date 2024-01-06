#include "commdevice.h"

using namespace utility;
using namespace driver;
using namespace reactor;

UnderwaterAcousticCommDev::UnderwaterAcousticCommDev(IniConfigParser *config, std::string config_item)
{
    /* 解析参数 */
    bool parserFlag = true;
    parserFlag &= config->getValue<int           >(config_item, "DEVICE_ID"           , m_devid);
    parserFlag &= config->getValue<std::string   >(config_item, "DEVICE_IDENTIFY"     , m_devidentify);
    parserFlag &= config->getValue<std::string   >(config_item, "STORAGE_PATH"        , m_storagepath);
    parserFlag &= config->getValue<std::string   >(config_item, "STORAGE_EXTENTPREFIX", m_storageextentprefix);
    parserFlag &= config->getValue<std::string   >(config_item, "SERIAL_PORT"         , m_serialport);
}

UnderwaterAcousticCommDev::~UnderwaterAcousticCommDev()
{
    close();
}

bool UnderwaterAcousticCommDev::handleEvent(DeviceEvent event)
{
    bool ret = true;

    switch(event.m_type)
    {
        case DeviceEvent::EVENT_OPEN:            /* 创建文件传输对象 */
            if (f_open)
            {
                log_error("重复打开设备({})", m_devid);
                ret = false;
                break;
            }

            log_info("水声通信设备({})初始化", m_devid);
            p_filetransfer = new SerialFileTransfer(
                m_serialport,
                [this] ()
                {
                    DeviceEvent event(1, DeviceEvent::EVENT_READYREAD, m_devid, "", "", "");
                    f_serverreable_cb(event);
                });

            f_open = true;
            break;

        case DeviceEvent::EVENT_WRITE:            /* 传输文件       */
            if (!f_open)
            {
                log_error("向设备({})写入文件前未打开设备", m_devid);
                ret = false;
                break;
            }

            log_debug("水声通信设备({})接收到任务：传输文件({})", m_devid, event.m_action);
            ret = (p_filetransfer->transfer(event.m_action)).first;
            break;

        case DeviceEvent::EVENT_READ:            /* 读取文件        */
            if (!f_open)
            {
                log_error("向设备({})读取文件前未打开设备", m_devid);
                ret = false;
                break;
            }

            log_debug("水声通信设备({})接收到任务:接收文件", m_devid);
            ret = (p_filetransfer->receive(m_storagepath, m_storageextentprefix, getSystimeByFilenameFormat())).first;
            break;
            
        case DeviceEvent::EVENT_CLOSE:          /* 关闭文件传输对象  */
            if (!f_open)
            {
                log_error("重复关闭设备({})", m_devid);
                break;
            }

            log_info("水声通信设备({})关闭传输", m_devid);
            close();
            break;

        case DeviceEvent::EVENT_POWER_ON:
        case DeviceEvent::EVENT_POWER_OFF:
        case DeviceEvent::EVENT_CONFIG:
        case DeviceEvent::EVENT_READYREAD:
        case DeviceEvent::EVENT_OTHER:
        default:
            log_error("水声通信设备({})收到未知事件, event = {}", m_devid, event.EventTypeMapping[event.m_type]);
            break;
    }

    return ret;
}