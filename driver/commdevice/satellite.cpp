#include "commdevice.h"

using namespace utility;
using namespace driver;
using namespace reactor;

SatelliteCommDev::SatelliteCommDev(IniConfigParser *config, std::string config_item)
{
    /* 解析参数 */
    bool parserFlag = true;
    parserFlag &= config->getValue<int           >(config_item, "DEVICE_ID"           , m_devid);
    parserFlag &= config->getValue<std::string   >(config_item, "DEVICE_IDENTIFY"     , m_devidentify);
    parserFlag &= config->getValue<std::string   >(config_item, "STORAGE_PATH"        , m_storagepath);
    parserFlag &= config->getValue<std::string   >(config_item, "STORAGE_EXTENTPREFIX", m_storageextentprefix);
    parserFlag &= config->getValue<std::string   >(config_item, "SERIAL_PORT"         , m_serialport);
    parserFlag &= config->getValue<int           >(config_item, "CORRECTED_DELAY_US"  , m_corrected_delay_us);
}

SatelliteCommDev::~SatelliteCommDev()
{
    close();
}

void SatelliteCommDev::open()
{
    p_filetransfer = new SerialFileTransfer(
        m_serialport,
        [this] ()
        {
            DeviceEvent event(0, DeviceEvent::EVENT_READYREAD, m_devid, "", "", "");
            f_serverreable_cb(event);
        });
}