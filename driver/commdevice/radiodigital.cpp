#include "commdevice.hpp"

using namespace utility;
using namespace driver;
using namespace reactor;

RadiodigitalCommDev::RadiodigitalCommDev(IniConfigParser *config, std::string config_item)
{
    /* 解析参数 */
    bool parserFlag = true;
    parserFlag &= config->getValue<int           >(config_item, "DEVICE_ID"           , m_devid);
    parserFlag &= config->getValue<std::string   >(config_item, "DEVICE_IDENTIFY"     , m_devidentify);
    parserFlag &= config->getValue<std::string   >(config_item, "STORAGE_PATH"        , m_storagepath);
    parserFlag &= config->getValue<std::string   >(config_item, "STORAGE_EXTENTPREFIX", m_storageextentprefix);
    parserFlag &= config->getValue<unsigned short>(config_item, "SERVER_PORT"         , m_serverport);
    parserFlag &= config->getValue<std::string   >(config_item, "TARGET_IP"           , m_targetip);
    parserFlag &= config->getValue<unsigned short>(config_item, "TARGET_PORT"         , m_targetport);
    parserFlag &= config->getValue<int           >(config_item, "CORRECTED_DELAY_US"  , m_corrected_delay_us);
}

RadiodigitalCommDev::~RadiodigitalCommDev()
{
    close();
}

void RadiodigitalCommDev::open()
{
    p_filetransfer = new TcpFileTransfer(
        m_serverport,
        [this] ()
        {
            DeviceEvent event(0, DeviceEvent::EVENT_READYREAD, m_devid, "", "", "");
            f_serverreable_cb(event);
        }, 
        m_targetip, m_targetport);
}
