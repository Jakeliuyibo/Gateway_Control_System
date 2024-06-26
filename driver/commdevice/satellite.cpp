#include "commdevice.hpp"

using namespace utility;
using namespace driver;
using namespace reactor;

SatelliteCommDev::SatelliteCommDev(IniConfigParser *configParser, std::string configItem)
{
    /* 解析参数 */
    bool parserFlag = true;
    parserFlag &= configParser->GetValue<int           >(configItem, "DEVICE_ID"           , devId_);
    parserFlag &= configParser->GetValue<std::string   >(configItem, "DEVICE_IDENTIFY"     , devIdentify_);
    parserFlag &= configParser->GetValue<std::string   >(configItem, "STORAGE_PATH"        , storagePath_);
    parserFlag &= configParser->GetValue<std::string   >(configItem, "STORAGE_EXTENTPREFIX", storageExtentPrefix_);
    parserFlag &= configParser->GetValue<std::string   >(configItem, "SERIAL_PORT"         , serialPort_);
    parserFlag &= configParser->GetValue<int           >(configItem, "CORRECTED_DELAY_US"  , correctedDelayUs_);

    if (!parserFlag)
    {
        log_error("SatelliteCommDev Init Failed.");
    }
}

SatelliteCommDev::~SatelliteCommDev()
{
    Close();
}

void SatelliteCommDev::Open()
{
    pFileTransfer_ = new SerialFileTransfer(
        serialPort_,
        [this] ()
        {
            DeviceEvent event(0, DeviceEvent::EventType::READYREAD, devId_, "", "", "");
            serverReabableCbkFunc_(event);
        });
}