#include "commdevice.hpp"

using namespace utility;
using namespace driver;
using namespace reactor;

OpticalfiberCommDev::OpticalfiberCommDev(IniConfigParser* configParser, std::string configItem)
{
    /* 解析参数 */
    bool parserFlag = true;
    parserFlag &= configParser->GetValue<int           >(configItem, "DEVICE_ID", devId_);
    parserFlag &= configParser->GetValue<std::string   >(configItem, "DEVICE_IDENTIFY", devIdentify_);
    parserFlag &= configParser->GetValue<std::string   >(configItem, "STORAGE_PATH", storagePath_);
    parserFlag &= configParser->GetValue<std::string   >(configItem, "STORAGE_EXTENTPREFIX", storageExtentPrefix_);
    parserFlag &= configParser->GetValue<unsigned short>(configItem, "SERVER_PORT", serverPort_);
    parserFlag &= configParser->GetValue<std::string   >(configItem, "TARGET_IP", targetIp_);
    parserFlag &= configParser->GetValue<unsigned short>(configItem, "TARGET_PORT", targetPort_);
    parserFlag &= configParser->GetValue<int           >(configItem, "CORRECTED_DELAY_US", correctedDelayUs_);

    if (!parserFlag)
    {
        log_error("OpticalfiberCommDev Init Failed.");
    }
}

OpticalfiberCommDev::~OpticalfiberCommDev()
{
    Close();
}

void OpticalfiberCommDev::Open()
{
    pFileTransfer_ = new TcpFileTransfer(
        serverPort_,
        [this]()
        {
            DeviceEvent event(0, DeviceEvent::EventType::READYREAD, devId_, "", "", "");
            serverReabableCbkFunc_(event);
        },
        targetIp_, targetPort_);
}
