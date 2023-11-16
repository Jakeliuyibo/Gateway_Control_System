#include "commdevice.h"


using namespace utility;
using namespace driver;
using namespace reactor;

OpticalfiberCommDev::OpticalfiberCommDev(IniConfigParser *config)
{
    /* 解析参数 */
    bool parserFlag = true;
    parserFlag &= config->getValue<int           >("OPTICALFIBER", "DEVICE_ID"  , m_devid);
    parserFlag &= config->getValue<unsigned short>("OPTICALFIBER", "SERVER_PORT", m_serverport);
    parserFlag &= config->getValue<std::string   >("OPTICALFIBER", "TARGET_IP"  , m_targetip);
    parserFlag &= config->getValue<unsigned short>("OPTICALFIBER", "TARGET_PORT", m_targetport);
}

OpticalfiberCommDev::~OpticalfiberCommDev()
{
    close();
}

bool OpticalfiberCommDev::handleEvent(DeviceEvent &event)
{
    switch(event.m_type)
    {
        case DeviceEvent::EVENT_INIT:
            /* 创建文件传输对象 */
            p_filetransfer = new TcpFileTransfer(m_serverport, m_targetip, m_targetport);
            break;
        case DeviceEvent::EVENT_WRITE:
            /* 传输文件 */
            p_filetransfer->transfer(event.m_action);
            break;
        case DeviceEvent::EVENT_READ:
            /* 读取文件 */
            // auto ret = p_filetransfer->recvive(const std::string &file_path, const std::string &prefix, const std::string &suffix);
            break;
        case DeviceEvent::EVENT_CLOSE:
            close();
            break;
        case DeviceEvent::EVENT_READYREAD:
        case DeviceEvent::EVENT_OTHER:
        default:
            log_error("光纤通信设备收到未知事件, event = {}", event.EventTypeMapping[event.m_type]);
            break;
    }

    return true;
}
