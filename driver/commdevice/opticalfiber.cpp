#include "commdevice.h"


using namespace utility;
using namespace driver;
using namespace reactor;

OpticalfiberCommDev::OpticalfiberCommDev(IniConfigParser *config)
{
    /* 解析参数 */
    bool parserFlag = true;
    std::string target_ip;
    unsigned short server_port, target_port; 
    parserFlag &= config->getValue<int           >("OPTICALFIBER", "DEVICE_ID"  , m_devid);
    parserFlag &= config->getValue<unsigned short>("OPTICALFIBER", "SERVER_PORT", server_port);
    parserFlag &= config->getValue<std::string   >("OPTICALFIBER", "TARGET_IP"  , target_ip);
    parserFlag &= config->getValue<unsigned short>("OPTICALFIBER", "TARGET_PORT", target_port);

    /* 创建文件传输对象 */
    p_filetransfer = std::make_unique<TcpFileTransfer>(server_port, target_ip, target_port);
}

OpticalfiberCommDev::~OpticalfiberCommDev()
{

}

bool OpticalfiberCommDev::handleEvent(const DeviceEvent &event)
{
    switch(event.m_type)
    {
        case DeviceEvent::EVENT_INIT:
            break;
        case DeviceEvent::EVENT_WRITE:
            break;
        case DeviceEvent::EVENT_READ:
            break;
        case DeviceEvent::EVENT_READYREAD:
            break;
        case DeviceEvent::EVENT_CLOSE:
            break;
        case DeviceEvent::EVENT_OTHER:
            break;
        
    }

    return true;
}

