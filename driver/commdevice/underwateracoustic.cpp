#include "commdevice.h"

using namespace utility;
using namespace driver;
using namespace reactor;

UnderwaterAcousticCommDev::UnderwaterAcousticCommDev(IniConfigParser *config)
{
    /* 解析参数 */
    bool parserFlag = true;
    parserFlag &= config->getValue<int           >("UNDERWATERACOUSTIC", "DEVICE_ID"              , m_devid);
    parserFlag &= config->getValue<std::string   >("UNDERWATERACOUSTIC", "DEVICE_IDENTIFY"        , m_devidentify);
    parserFlag &= config->getValue<std::string   >("UNDERWATERACOUSTIC", "STORAGE_PATH"           , m_storagepath);
    parserFlag &= config->getValue<std::string   >("UNDERWATERACOUSTIC", "STORAGE_EXTENTPREFIX"   , m_storageextentprefix);

    parserFlag &= config->getValue<std::string   >("UNDERWATERACOUSTIC", "SERIAL_PORT"            , m_serialport);
}

UnderwaterAcousticCommDev::~UnderwaterAcousticCommDev()
{
    close();
}

bool UnderwaterAcousticCommDev::handleEvent(DeviceEvent event)
{
    switch(event.m_type)
    {
        case DeviceEvent::EVENT_INIT:            /* 创建文件传输对象 */
            log_info("水声通信设备({})初始化", m_devid);
            p_filetransfer = new SerialFileTransfer(
                m_serialport,
                [this] ()
                {
                    DeviceEvent event(1, DeviceEvent::EVENT_READYREAD, m_devidentify, "");
                    f_serverreable_cb(event);
                });
            break;
        case DeviceEvent::EVENT_WRITE:            /* 传输文件       */
            log_debug("水声通信设备({})接收到任务：传输文件({})", m_devid, event.m_action);
            p_filetransfer->transfer(event.m_action);
            break;
        case DeviceEvent::EVENT_READ:            /* 读取文件        */
            log_debug("水声通信设备({})接收到任务:接收文件", m_devid);
            p_filetransfer->receive(m_storagepath, m_storageextentprefix, getSystimeByFilenameFormat());
            break;
        case DeviceEvent::EVENT_CLOSE:          /* 关闭文件传输对象  */
            log_info("水声通信设备({})关闭传输", m_devid);
            close();
            break;
        case DeviceEvent::EVENT_READYREAD:
        case DeviceEvent::EVENT_OTHER:
        default:
            log_error("水声通信设备({})收到未知事件, event = {}", m_devid, event.EventTypeMapping[event.m_type]);
            break;
    }
    return true;
}