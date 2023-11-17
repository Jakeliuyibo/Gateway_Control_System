#include "systime.h"
#include "commdevice.h"


using namespace utility;
using namespace driver;
using namespace reactor;

OpticalfiberCommDev::OpticalfiberCommDev(IniConfigParser *config)
{
    /* 解析参数 */
    bool parserFlag = true;
    parserFlag &= config->getValue<int           >("OPTICALFIBER", "DEVICE_ID"              , m_devid);
    parserFlag &= config->getValue<std::string   >("OPTICALFIBER", "DEVICE_IDENTIFY"        , m_devidentify);
    parserFlag &= config->getValue<std::string   >("OPTICALFIBER", "STORAGE_PATH"           , m_storagepath);
    parserFlag &= config->getValue<std::string   >("OPTICALFIBER", "STORAGE_EXTENTPREFIX"   , m_storageextentprefix);

    parserFlag &= config->getValue<unsigned short>("OPTICALFIBER", "SERVER_PORT"            , m_serverport);
    parserFlag &= config->getValue<std::string   >("OPTICALFIBER", "TARGET_IP"              , m_targetip);
    parserFlag &= config->getValue<unsigned short>("OPTICALFIBER", "TARGET_PORT"            , m_targetport);
}

OpticalfiberCommDev::~OpticalfiberCommDev()
{
    close();
}

bool OpticalfiberCommDev::handleEvent(DeviceEvent event)
{
    switch(event.m_type)
    {
        case DeviceEvent::EVENT_INIT:            /* 创建文件传输对象 */
            p_filetransfer = new TcpFileTransfer(
                m_serverport,
                [this] ()
                {
                    DeviceEvent event(1, DeviceEvent::EVENT_READYREAD, m_devidentify, "");
                    f_serverreable_cb(event);
                }, 
                m_targetip, m_targetport);
            log_info("光纤通信设备初始化");
            break;
        case DeviceEvent::EVENT_WRITE:            /* 传输文件       */
            p_filetransfer->transfer(event.m_action);
            log_info("光纤通信设备传输文件");
            break;
        case DeviceEvent::EVENT_READ:            /* 读取文件        */
            p_filetransfer->receive(m_storagepath, m_storageextentprefix, getSystimeByFilenameFormat());
            log_info("光纤通信设备读取文件");
            break;
        case DeviceEvent::EVENT_CLOSE:          /* 关闭文件传输对象  */
            close();
            log_info("光纤通信设备关闭传输");
            break;
        case DeviceEvent::EVENT_READYREAD:
        case DeviceEvent::EVENT_OTHER:
        default:
            log_error("光纤通信设备收到未知事件, event = {}", event.EventTypeMapping[event.m_type]);
            break;
    }
    return true;
}
