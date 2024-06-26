#include "filetransfer.hpp"

#include <sstream>

using namespace utility;
using namespace driver;

/**********************************************************************************
 *************************    Public    *******************************************
 **********************************************************************************/

SerialFileTransfer::SerialFileTransfer(StrType portName, const CbkFuncType &serverReadableCbkFunc)
    :
    portName_(portName),
    serverReadableCbkFunc_(serverReadableCbkFunc),
    pIoc_(std::make_unique<IocType>()),
    pSerialPort_(std::make_unique<SerialType>(*pIoc_))
{
    boost::system::error_code ec;

    /* 打开串口 */
    pSerialPort_->open(portName_, ec);
    if (ec)
    {
        log_error("打开串口({})失败, msg = {}", portName_, ec.message());
        return;
    }
	
	/* 设置串口参数 */
	pSerialPort_->set_option(boost::asio::serial_port::baud_rate(kSerialBaudrate), ec);
	pSerialPort_->set_option(boost::asio::serial_port::flow_control(boost::asio::serial_port::flow_control::none), ec);
	pSerialPort_->set_option(boost::asio::serial_port::parity(boost::asio::serial_port::parity::none), ec);
	pSerialPort_->set_option(boost::asio::serial_port::stop_bits(boost::asio::serial_port::stop_bits::one), ec);
	pSerialPort_->set_option(boost::asio::serial_port::character_size(8), ec);
    if (ec)
    {
        log_error("设置串口({})参数异常, msg = {}", portName_, ec.message());
        return;
    }

    /* 构建客户端请求文件ID分配器 */
    for(int idx = kFileIdRange[0]; idx < kFileIdRange[1]; idx++)
    {
        clientFileIdAllocator_.Enqueue(idx);
    }

    /* 创建子线程监听客户端连接 */
    std::thread(&SerialFileTransfer::SubthreadFuncListenClient, this).detach();

    /* 创建子线程监听服务器文件管理 */
    std::thread(&SerialFileTransfer::SubthreadFuncListenManageServerFileDescription, this).detach();
}

SerialFileTransfer::~SerialFileTransfer()
{
    if (pSerialPort_->is_open())
    {
        pSerialPort_->close();
    }
}

SerialFileTransfer::FtRetType SerialFileTransfer::Transfer(CStrType fileFullPath, CStrType filePath, CStrType fileName)
{
    FtRetType ret(true);
    ret.fileFullPath_ = fileFullPath;
    ret.filePath_ = filePath;
    ret.fileName_ = fileName;

    /* 分配文件ID并创建文件描述符 */
    std::size_t fileId;
    FileTransferDescription *pFtD;
    try
    {
        if (!clientFileIdAllocator_.Dequeue(fileId, false))
        {
            throw std::runtime_error("分配文件ID");
        }

        {
            std::unique_lock<std::mutex> _fmk(clientFileManagementLock_);
            clientFileManagement_[fileId] = std::make_shared<FileTransferDescription>(fileName);
            pFtD = clientFileManagement_[fileId].get();
        }
    }
    catch(const std::exception& ex)
    {
        log_error("SerialFTP({})发送文件时创建描述符失败, msg = {}", portName_, ex.what());
        ret.status_ = false;
        return ret;
    }

    std::unique_lock<std::mutex> lck(pFtD->blockLock_);

    /* 读取文件信息并计算相关信息 */
    std::ifstream ifs;
    try
    {
        ifs.open(fileFullPath, std::ios::binary | std::ios::ate);
        pFtD->fileSize_ = ifs.tellg();
        pFtD->tunkSize_ = static_cast<std::size_t>(std::ceil(static_cast<double>(pFtD->fileSize_) / kProtocolActualPayloadLen));
        ret.fileSize_ = pFtD->fileSize_;
        ifs.seekg(0, std::ios::beg);

        /* 读取文件加载到内存中 */
        {
            std::size_t remainingBytes = pFtD->fileSize_;
            std::vector<char> tunk(kProtocolActualPayloadLen);
            while (!ifs.eof() && remainingBytes > 0)   // 分包发送
            {
                std::size_t bytesToWrite = std::min(remainingBytes, kProtocolActualPayloadLen);
                ifs.read(tunk.data(), bytesToWrite);
                std::string tunkStr(tunk.begin(), tunk.begin() + bytesToWrite);
                pFtD->tunkData_.emplace_back(tunkStr);
                remainingBytes -= bytesToWrite;
            }
        }

        /* 释放资源 */
        ifs.close();
    }
    catch(const std::exception& e)
    {
        log_error("SerialFTP({})获取文件{})失败, error msg = {}", portName_, fileFullPath, e.what());
        ret.status_ = false;
        goto err_realease;
    }

    /* 串口传输文件 */
    try
    {
        /* 1、传输文件信息：${cmd:1}${fileName}-${fileSize}-${tunkSize} */
        ret.transBytes_ += WriteBytes(ProtocolCmd::FILEINFO, IdFormatTransfomer(fileId)
                                                + kProtocolSeparator
                                                + fileName
                                                + kProtocolSeparator
                                                + std::to_string(pFtD->fileSize_)
                                                + kProtocolSeparator
                                                + std::to_string(pFtD->tunkSize_));

        /* 2、阻塞等待服务端的文件ID，等待时间kBlockingTimes秒 */
        if (!(pFtD->blockCv_).wait_for(lck, std::chrono::milliseconds(kBlockingTimes), [&pFtD] () {return pFtD->hasRecvFileId_;})
            && !(pFtD->hasRecvFileId_))
        {
            throw std::runtime_error("未在规定时间内接收到文件ID.");
        }

        /* 3、传输块数据： ${cmd:3}${fileId}-${tunkId}:${tunkData} */
        for (auto tunkId = 0; tunkId < pFtD->tunkData_.size(); tunkId++)
        {
            ret.transBytes_ += WriteBytes(ProtocolCmd::TUNKDATA, IdFormatTransfomer(fileId)
                                                    + IdFormatTransfomer(tunkId)
                                                    + kProtocolSeparator
                                                    + pFtD->tunkData_[tunkId]);
        }

        /* 4、阻塞等待服务端的ACK或RETRANS，等待时间3秒 */
        if (!(pFtD->blockCv_).wait_for(lck, std::chrono::milliseconds(kBlockingTimes), [&pFtD] () {return pFtD->hasRecvAck_ || pFtD->hasRecvRetrans_;}))
        {
            throw std::runtime_error("未在规定时间内接收到ACK或RETRANS.");
        }

        /* 5、检测是否需要重传：*/
        if (pFtD->hasRecvRetrans_)
        {
            for (auto tunkId : pFtD->retransTunkId_)
            {
                ret.transBytes_ += WriteBytes(ProtocolCmd::TUNKDATA, IdFormatTransfomer(fileId)
                                                        + IdFormatTransfomer(tunkId)
                                                        + kProtocolSeparator
                                                        + pFtD->tunkData_[tunkId]);
            }

            if (!(pFtD->blockCv_).wait_for(lck, std::chrono::milliseconds(kBlockingTimes), [&pFtD] () {return pFtD->hasRecvAck_;}))
            {
                throw std::runtime_error("重传后未在规定时间内接收到ACK.");
            }
        }

        /* 6、检测是否发送成功 */
        if (pFtD->hasRecvAck_)
        {
            log_info("SerialFTP({})成功传输文件{}", portName_, fileFullPath);
        }
    }
    catch(const std::exception& ex)
    {
        log_error("SerialFTP({})传输文件{})失败, error msg = {}", portName_, fileFullPath, ex.what());
        ret.status_ = false;
        goto err_realease;
    }

err_realease:
    /* 释放资源 */
    {
        std::unique_lock<std::mutex> lck(clientFileManagementLock_);
        clientFileManagement_.erase(fileId);
    }
    clientFileIdAllocator_.Enqueue(fileId);

    return ret;
}

SerialFileTransfer::FtRetType SerialFileTransfer::Receive(StrType fileFullPath, CStrType filePath)
{
    FtRetType ret(true);
    ret.filePath_ = filePath;
    
    try
    {
        /* 获取文件ID */
        std::size_t fileId;
        if (!serverReadableFileId_.Dequeue(fileId, false))
        {
            throw std::runtime_error("不存在可读文件");
        }

        /* 获取文件描述符 */
        std::unique_lock<std::mutex> lck(serverFileManagementLock_);
        if (serverFileManagement_.find(fileId) == serverFileManagement_.end())
        {
            throw std::runtime_error("不存在该文件ID");
        }

        /* 将数据写入文件 */
        FileReceDescription &fileDesc = serverFileManagement_[fileId];
        std::size_t repFileNamePos = fileFullPath.find(kRepFileNameSymbol);
        if (repFileNamePos == std::string::npos)
        {
            throw std::runtime_error("Can't find replace FILENAME symbol.");
        }
        fileFullPath.replace(repFileNamePos, kRepFileNameSymbol.size(), fileDesc.fileName_);
        ret.fileFullPath_ = fileFullPath;
        ret.fileName_ = fileDesc.fileName_;
        ret.fileSize_ = fileDesc.fileSize_;
        ret.recvBytes_ = fileDesc.fileSize_;

        // 打开写入文件
        std::ofstream ofs(fileFullPath, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open())
        {
            throw std::runtime_error("File not existed.");
        }
        for(auto &it : fileDesc.tunkData_)
        {
            ofs.write((it.second).c_str(), (it.second).size());
        }

        log_info("SerialFTP({})成功接收文件({})", portName_, fileFullPath);

        /* 释放资源 */
        ofs.flush();
        ofs.close();
        serverFileManagement_.erase(fileId);
    }
    catch(const std::exception& ex)
    {
        log_error("SerialFTP({})接收文件异常, msg = {}", portName_, ex.what());
        ret.status_ = false;
    }
    
err_ret:
    return ret;
}

/**********************************************************************************
 *************************    Sub Thread    ***************************************
 **********************************************************************************/

// 创建子线程监听客户端数据
void SerialFileTransfer::SubthreadFuncListenClient()
{
    try
    {
        std::vector<uint8_t> rbuf(kProtocolLen);
        for(;;)
        {
            /* 阻塞读取数据 */
            std::size_t recvBytes = boost::asio::read(*pSerialPort_, boost::asio::buffer(rbuf.data(), kProtocolLen));

            log_trace("SerialFTP({})接收到一包数据({})", portName_, ConvertVecu8ToHexstring(rbuf));

            /* 校验 */
            ProtocolCmd cmd;
            try
            {
                CheckProtocolFrame(rbuf, cmd);
            }
            catch(const std::exception& ex)
            {
                log_error("SerialFTP({})接收到一包错误数据({}), msg = {}", portName_, ConvertVecu8ToHexstring(rbuf), ex.what());
                continue;
            }

            /* 去除起始位和停止位 */
            std::vector<uint8_t> payload(rbuf.begin() + kProtocolStartBitsLen + kProtocolCmdLen, 
                                         rbuf.begin() + kProtocolStartBitsLen + kProtocolCmdLen + kProtocolPayloadLen);

            /* 处理协议包 */
            ProtocolPackage package(cmd, payload);
            HandleProtocolPackage(package);
        }
    }
    catch(const std::exception& ex)
    {
        log_error("SerialFTP({})监听客户端连接出错, msg = {}", portName_, ex.what());
    }
}

// 创建子线程监听服务器文件管理
void SerialFileTransfer::SubthreadFuncListenManageServerFileDescription()
{
    try
    {
        for(;;)
        {
            {
                std::unique_lock<std::mutex> lck(serverFileManagementLock_);
                auto curTime = std::chrono::system_clock::now();       // 当前时间
                std::vector<std::size_t> toDeleteFileId;              // 重传超出次数需要删除的fileId
                for(auto &it : serverFileManagement_)
                {
                    auto &fileId   = it.first;
                    auto &fileDesc = it.second;
                    auto &fileName = fileDesc.fileName_;
                    auto &tunkData = fileDesc.tunkData_;
                    if (   tunkData.size() < fileDesc.tunkSize_ 
                        && std::chrono::duration_cast<std::chrono::milliseconds>(curTime - fileDesc.recvTime_).count() > kMaximumWaitTimes
                        )
                    {
                        log_warning("SerialFTP({})检测到文件({}:{})接收超时", portName_, fileId, fileName);

                        /* 查找需要重传的数据块 */
                        std::vector<std::size_t> retransTunkId;
                        for(std::size_t tunkId = 0; tunkId < fileDesc.tunkSize_; tunkId++)
                        {
                            if (tunkData.find(tunkId) == tunkData.end())
                            {
                                retransTunkId.emplace_back(tunkId);
                            }
                        }

                        /* 发送重传数据包 */
                        if (retransTunkId.size() > 0)
                        {
                            std::string writeStr = IdFormatTransfomer(fileId) + kProtocolSeparator;
                            for(auto idx = 0; idx < retransTunkId.size(); idx++)
                            {
                                writeStr += IdFormatTransfomer(retransTunkId[idx]);
                                if (idx != (retransTunkId.size() - 1))
                                {
                                    writeStr += kProtocolSeparator;
                                }
                            }
                            WriteBytes(ProtocolCmd::RETRANS, writeStr);
                        }

                        /* 记录重传次数 */
                        fileDesc.retransCnt_++;
                        if (fileDesc.retransCnt_ > kMaximumRetransCnt)
                        {
                            toDeleteFileId.emplace_back(fileId);
                            log_error("SerialFTP({})重传({}:{})超过最大次数", portName_, fileId, fileName);
                        }
                    }
                }

                /* 删除超出重传次数的file id */
                for(auto fileId : toDeleteFileId)
                {
                    serverFileManagement_.erase(fileId);
                }
            }

            std::this_thread::sleep_for(std::chrono::milliseconds(kServerListenInterval));
        }
    }
    catch(const std::exception& ex)
    {
        log_error("SerialFTP({})监听服务器文件管理异常, msg = {}", portName_, ex.what());
    }
}


/**********************************************************************************
 *************************    Private    ******************************************
 **********************************************************************************/

// 处理协议数据包
void SerialFileTransfer::HandleProtocolPackage(ProtocolPackage &package)
{
    /* 根据控制位处理数据 */
    if      (package.cmd == ProtocolCmd::FILEINFO)   // ! 文件信息
    {
        /* 解析文件信息，获取fileName\fileSize\tunkSize */
        std::string fileName;
        std::size_t fileId, fileSize, tunkSize;
        try
        {
            ParseFileInfoFromPayload(package.payload, fileId, fileName, fileSize, tunkSize);
            log_debug("SerialFTP({})接收到文件信息({}:{}:{}:{})", portName_, fileId, fileName, fileSize, tunkSize);
        }
        catch(const std::exception& ex)
        {
            log_error("SerialFTP({})解析FILEINFO的Payload({})出错, msg = {}", portName_, ConvertVecu8ToHexstring(package.payload), ex.what());
            return;
        }

        /* 创建文件描述符并加入文件管理 */
        {
            std::unique_lock<std::mutex> lck(serverFileManagementLock_);
            serverFileManagement_[fileId] = FileReceDescription(fileId, fileName, fileSize, tunkSize);
        }
        
        /* 向客户端发送文件ID进行确认 */
        SerialFileTransfer::WriteBytes(ProtocolCmd::FILEID, IdFormatTransfomer(fileId));
    }
    else if (package.cmd == ProtocolCmd::FILEID)     // ! 文件ID
    {
        /* 解析文件ID，获取file_id */
        std::size_t fileId;
        try
        {
            ParseFileIdFromPayload(package.payload, fileId);
            log_debug("SerialFTP({})接收到文件ID({})", portName_, fileId);
        }
        catch(const std::exception& ex)
        {
            log_error("SerialFTP({})解析FILEID的Payload({})出错, msg = {}", portName_, ConvertVecu8ToHexstring(package.payload), ex.what());
            return;
        }

        {
            std::unique_lock<std::mutex> lck(clientFileManagementLock_);
            /* 查找客户端阻塞条件管理器，置位变量 */ 
            if (clientFileManagement_.find(fileId) == clientFileManagement_.end())
            {
                log_error("SerialFTP({})接收到文件ID({})到未在客户端管理器中查询到条目", portName_, fileId);
                return;
            }

            {
                std::unique_lock<std::mutex> lck(clientFileManagement_[fileId]->blockLock_);
                clientFileManagement_[fileId]->hasRecvFileId_ = true;
                clientFileManagement_[fileId]->blockCv_.notify_all();
            }
        }
    }
    else if (package.cmd == ProtocolCmd::TUNKDATA)   // ! 块数据
    {
        /* 解析块数据，获取fileId\tunkId\tunkData */
        std::size_t fileId, tunkId;
        std::string tunkData;
        try
        {
            ParseTunkdataFromPayload(package.payload, fileId, tunkId, tunkData);
            log_debug("SerialFTP({})接收到块数据({}:{}:{})", portName_, fileId, tunkId, tunkData);
        }
        catch(const std::exception& ex)
        {
            log_error("SerialFTP({})解析TUNKDATA的Payload({})出错, msg = {}", portName_, ConvertVecu8ToHexstring(package.payload), ex.what());
            return;
        }

        /* 检测文件id是否存在以及tunkId是否已存在或超出tunkSize */
        try
        {
            std::unique_lock<std::mutex> lck(serverFileManagementLock_);

            if (serverFileManagement_.find(fileId) == serverFileManagement_.end())
            {
                throw std::runtime_error("未知fileId");
            }

            auto &file_desc = serverFileManagement_[fileId];
            if (tunkId >= file_desc.tunkSize_)
            {
                throw std::runtime_error("未知的tunkId");
            }

            std::map<std::size_t, std::string> &tunk_mp = file_desc.tunkData_;
            if (tunk_mp.find(tunkId) != tunk_mp.end())
            {
                throw std::runtime_error("重复的tunkId");
            }

            /* 将块数据存入文件管理 */
            file_desc.recvTime_ = std::chrono::system_clock::now();
            tunk_mp[tunkId] = tunkData; 

            /* 判断块数据是否收集完成 */
            if (tunk_mp.size() == file_desc.tunkSize_)
            {
                /* 发送ACK */
                SerialFileTransfer::WriteBytes(ProtocolCmd::ACK, IdFormatTransfomer(fileId));

                serverReadableFileId_.Enqueue(fileId);

                /* 回调通知上层接口接收到完整文件 */
                serverReadableCbkFunc_();
            }
        }
        catch(const std::exception& ex)
        {
            log_error("SerialFTP({})接收数据块({})出错, msg = {}", portName_, ConvertVecu8ToHexstring(package.payload), ex.what());
            return;
        }
    }
    else if (package.cmd == ProtocolCmd::ACK)        // ! ack数据
    {
        /* 解析ACK数据，获取fileId */
        std::size_t fileId;
        try
        {
            ParseAckFromPayload(package.payload, fileId);
            log_debug("SerialFTP({})接收到ACK({})", portName_, fileId);
        }
        catch(const std::exception& ex)
        {
            log_error("SerialFTP({})解析ACK的Payload({})出错, msg = {}", portName_, ConvertVecu8ToHexstring(package.payload), ex.what());
            return;
        }

        /* 从客户端文件管理器中获取transfer线程阻塞的条件变量，通知其解锁 */
        {
            std::unique_lock<std::mutex> lck(clientFileManagementLock_);

            for (auto &it : clientFileManagement_)
            {
                if (it.first == fileId)
                {
                    std::unique_lock<std::mutex> lck(clientFileManagement_[fileId]->blockLock_);
                    clientFileManagement_[fileId]->hasRecvAck_ = true;
                    clientFileManagement_[fileId]->blockCv_.notify_all();
                    break;
                }
            }
        }

    }
    else if (package.cmd == ProtocolCmd::RETRANS)    // ! 重传
    {
        /* 解析重传数据，获取fileId和需要重传的块数据 */
        std::size_t fileId;
        std::vector<std::size_t> retransTunkId;
        try
        {
            ParseRetransFromPayload(package.payload, fileId, retransTunkId);
            log_debug("SerialFTP({})接收到重传({}:{})", portName_, fileId, retransTunkId.size());
        }
        catch(const std::exception& ex)
        {
            log_error("SerialFTP({})解析RETRANS的Payload({})出错, msg = {}", portName_, ConvertVecu8ToHexstring(package.payload), ex.what());
            return;
        }

        /* 从客户端文件管理器中获取transfer线程阻塞的条件变量，通知其解锁 */
        {
            std::unique_lock<std::mutex> lck(clientFileManagementLock_);

            for (auto &it : clientFileManagement_)
            {
                if (it.first == fileId)
                {
                    std::unique_lock<std::mutex> bk(clientFileManagement_[fileId]->blockLock_);
                    clientFileManagement_[fileId]->hasRecvRetrans_ = true;
                    clientFileManagement_[fileId]->retransTunkId_ = retransTunkId;
                    clientFileManagement_[fileId]->blockCv_.notify_all();
                    break;
                }
            }
        }
    }
}

// 转化协议帧为十六进制字符串
void SerialFileTransfer::CheckProtocolFrame(std::vector<uint8_t> &frame, ProtocolCmd &cmd)
{
    /* 校验块大小 */
    if (frame.size() != kProtocolLen) 
    {
        throw std::runtime_error("Error tunk size.");
    }

    /* 校验起始位 */
    for(int idx = 0; idx < kProtocolStartBitsLen; idx++) 
    {
        if (frame[idx] != kProtocolStartBits[idx]) 
        {
            throw std::runtime_error("Error start bits.");
        }
    }

    /* 校验控制位 */
    cmd = static_cast<ProtocolCmd>(frame[kProtocolStartBitsLen]);
    if (cmd != ProtocolCmd::FILEINFO && cmd != ProtocolCmd::FILEID && cmd != ProtocolCmd::TUNKDATA 
        && cmd != ProtocolCmd::ACK && cmd != ProtocolCmd::RETRANS) 
    {
        throw std::runtime_error("Error cmd bits.");
    }

    /* 校验停止位 */
    for(int idx = 0; idx < kProtocolEndBitsLen; idx++) 
    {
        if (frame[kProtocolLen - kProtocolEndBitsLen + idx] != kProtocolEndBits[idx]) 
        {
            throw std::runtime_error("Error end bits.");
        }
    }
}

// 校验协议帧
std::string SerialFileTransfer::ConvertVecu8ToHexstring(std::vector<uint8_t> &frame)
{
    std::stringstream ss;
    auto size = frame.size();
    for(auto idx = 0; idx < size; idx++) 
    {
        ss << "0x" << std::hex << static_cast<int>(frame[idx]);
        if (idx != size - 1) 
        {
            ss << " ";
        }
    }
    return ss.str();
}

// 解析文件信息,"${fileId}-${fileName}-${fileSize}-${tunkSize}"
void SerialFileTransfer::ParseFileInfoFromPayload(std::vector<uint8_t> &payload, std::size_t &fileId, std::string &fileName, std::size_t &fileSize, std::size_t &tunkSize)
{
    /* 解析字符串 */
    int cntSeparator = 0;
    std::vector<std::string> arr;
    SeparatePayloadBySymbol(payload, arr, cntSeparator);

    /* 检查格式是否规范 */
    if ((arr.size() != 4) || (cntSeparator != 3))
    {
        throw std::runtime_error("Error payload format.");
    }

    fileId   = std::stoull(arr[0]);
    fileName = arr[1];
    fileSize = std::stoull(arr[2]);
    tunkSize = std::stoull(arr[3]);
}

// 解析文件ID,"${fileId}"
void SerialFileTransfer::ParseFileIdFromPayload(std::vector<uint8_t> &payload, std::size_t &fileId)
{
    /* 解析字符串 */
    int cntSeparator = 0;               // 折号出现次数
    std::vector<std::string> arr;
    SeparatePayloadBySymbol(payload, arr, cntSeparator);

    /* 检查格式是否规范 */
    if ((arr.size() != 1) || (cntSeparator != 0))
    {
        throw std::runtime_error("Error payload format.");
    }

    if (arr[0].size() != kProtocolFileIdLen)
    {
        throw std::runtime_error("Error file id length.");
    }
    fileId = std::stoull(arr[0]);
}

// 解析块数据，"${fileId}${tunkId}-${tunkData}"
void SerialFileTransfer::ParseTunkdataFromPayload(std::vector<uint8_t> &payload, std::size_t &fileId, std::size_t &tunkId, std::string &tunkData)
{
    /* 解析字符串 */
    int cntSeparator = 0;
    std::vector<std::string> arr;
    SeparatePayloadBySymbol(payload, arr, cntSeparator);

    /* 检查格式是否规范 */
    if ((arr.size() != 2) || (cntSeparator != 1))
    {
        throw std::runtime_error("Error payload format.");
    }

    if (arr[0].size() != (kProtocolFileIdLen + kProtocolTunkIdLen))
    {
        throw std::runtime_error("Error file/tunk id length.");
    }

    fileId = std::stoull(arr[0].substr(0, kProtocolFileIdLen));
    tunkId = std::stoull(arr[0].substr(kProtocolFileIdLen, kProtocolTunkIdLen));
    tunkData = arr[1];
}

// 解析ACK数据,"${fileId}"
void SerialFileTransfer::ParseAckFromPayload(std::vector<uint8_t> &payload, std::size_t &fileId)
{
    /* 解析字符串 */
    int cntSeparator = 0;
    std::vector<std::string> arr;
    SeparatePayloadBySymbol(payload, arr, cntSeparator);

    /* 检查格式是否规范 */
    if ((arr.size() != 1) || (cntSeparator != 0))
    {
        throw std::runtime_error("Error payload format.");
    }

    if (arr[0].size() != kProtocolFileIdLen)
    {
        throw std::runtime_error("Error file id length.");
    }
    fileId = std::stoull(arr[0]);
}

// 解析重传数据,"${fileId}-${retransTunkId}-${id2}-{id3}"
void SerialFileTransfer::ParseRetransFromPayload(std::vector<uint8_t> &payload, std::size_t &fileId, std::vector<std::size_t> &retransTunkId)
{
    /* 解析字符串 */
    int cntSeparator = 0;
    std::vector<std::string> arr;
    SeparatePayloadBySymbol(payload, arr, cntSeparator);

    /* 检查格式是否规范 */
    if ( arr.size() != (cntSeparator + 1) )
    {
        throw std::runtime_error("Error payload format.");
    }

    if (arr[0].size() != kProtocolFileIdLen)
    {
        throw std::runtime_error("Error file id length.");
    }
    fileId = std::stoull(arr[0]);
    for(auto idx = 1; idx < arr.size(); idx++)
    {
        if (arr[idx].size() != kProtocolTunkIdLen)
        {
            throw std::runtime_error("Error tunk id length.");
        }
        retransTunkId.emplace_back(std::stoull(arr[idx]));
    }
}

// 通过'-','\x00'等符号分隔payload
void SerialFileTransfer::SeparatePayloadBySymbol(std::vector<uint8_t> &payload, std::vector<std::string> &parse, int &cntSeparator)
{
    /* 根据分隔符划分 */
    std::string tmp = "";
    for(auto &ch : payload)
    {
        if (ch == kProtocolSeparatorCH) {
            cntSeparator++;
            parse.emplace_back(tmp);
            tmp = "";
        } else if (ch == kProtocolNullCH) {
            break;
        } else {
            tmp += ch;
        }
    }
    parse.emplace_back(tmp);  // 尾部处理
}

// 写入一帧数据，payload长度不能超过kProtocolPayloadLen
std::size_t SerialFileTransfer::WriteBytes(ProtocolCmd cmd, std::vector<uint8_t> payload)
{
    std::size_t transBytes = 0;
    std:size_t payloadSize = payload.size();

    /* 检测payload是否超过最大长度 */
    if (payloadSize > kProtocolPayloadLen)
    {
        log_error("SerialFTP({})写入一帧数据时payload({})超出最大限制", portName_, ConvertVecu8ToHexstring(payload));
        return transBytes;
    }

    /* 构建完整帧协议 */
    std::vector<uint8_t> frame;

    // 添加起始位
    for(auto idx = 0; idx < kProtocolStartBitsLen; idx++)
    {
        frame.emplace_back(kProtocolStartBits[idx]);
    }

    // 添加控制位
    frame.emplace_back(static_cast<uint8_t>(cmd));

    // 添加负载
    for(int idx = 0; idx < payloadSize; idx++)
    {
        frame.emplace_back(payload[idx]);
    }

    // 添加补'\x00'
    std::size_t zeroFillSize = kProtocolPayloadLen - payloadSize;
    for(int idx = 0; idx < zeroFillSize; idx++)
    {
        frame.emplace_back('\x00');
    }

    // 添加结束位
    for(int idx = 0; idx < kProtocolEndBitsLen; idx++)
    {
        frame.emplace_back(kProtocolEndBits[idx]);
    }

    /* 串口发送数据 */
    transBytes += boost::asio::write(*pSerialPort_, boost::asio::buffer(frame.data(), kProtocolLen));

    log_debug("SerialFTP({})发送到一包数据({})", portName_, ConvertVecu8ToHexstring(frame));

    return transBytes;
}

std::size_t SerialFileTransfer::WriteBytes(ProtocolCmd cmd, std::string payload)
{
    std::vector<uint8_t> payloadUInt8(payload.begin(), payload.end());
    return WriteBytes(cmd, payloadUInt8);
}

// 将fileId/tunkId转化为固定长度的字符串
std::string SerialFileTransfer::IdFormatTransfomer(std::size_t id)
{
    std::string str = std::to_string(id);
    return (str.size() < 2 ? "0" + str : str);
}



