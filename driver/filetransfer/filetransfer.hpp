#pragma once
#include <iostream>
#include <fstream>
#include <thread>
#include <string>
#include <memory>
#include <atomic>
#include <chrono>
#include <mutex>
#include <map>

#include "boost/asio.hpp"
#include "boost/filesystem.hpp"

#include "safequeue.hpp"
#include "logger.hpp"

using namespace utility;

namespace driver
{
    /**
     * @description: 文件传输类，外部接口使用transfer(fileFullPath="/home/test.txt")传输文件
     *                                 使用receive(fileFullPath="/home/recv_{FILENAME}_2023_12_31{.EXTENT}")来接收文件
     */
    class FileTransfer
    {
    public:
        struct FtRetType
        {
            bool status_;                    // 执行状态
            std::size_t transBytes_;        // 发送字节
            std::size_t recvBytes_;         // 接收字节
            std::string fileFullPath_;     // 文件完整路径
            std::string filePath_;          // 文件路径
            std::string fileName_;          // 文件名
            std::size_t fileSize_;          // 文件大小

            FtRetType(bool st) : status_(st), transBytes_(0), recvBytes_(0),
                fileFullPath_(""), filePath_(""), fileName_(""), fileSize_(0) {}
        };

    public:
        using StrType = std::string;
        using CStrType = const StrType;

        virtual FtRetType Transfer(CStrType fileFullPath, CStrType filePath, CStrType fileName) = 0;
        virtual FtRetType Receive(StrType fileFullPath, CStrType filePath) = 0;
        virtual ~FileTransfer() = default;

        FtRetType Transfer(CStrType& fileFullPath)
        {
            if (!CheckFilePathExisted(fileFullPath)) {
                log_error("Don't existed file {}", fileFullPath);
                return FtRetType(false);
            }
            auto ret = ParseFileFullPath(fileFullPath);
            return Transfer(fileFullPath, ret.first, ret.second);
        }

        FtRetType Receive(CStrType& filePath, CStrType& extentPrefix, CStrType& extentSuffix)
        {
            if (!CheckFilePathExisted(filePath)) {
                log_error("Don't existed path {}", filePath);
                return FtRetType(false);
            }
            StrType fileFullPath = JointFilePathAndFileName(filePath, kRepFileNameSymbol + extentPrefix + extentSuffix);
            return Receive(fileFullPath, filePath);
        }

    private:
        // 检测文件路径是否存在
        inline bool CheckFilePathExisted(CStrType& fileFullPath)
        {
            return boost::filesystem::exists(fileFullPath);
        }

        // 解析文件完整路径，返回目录和文件名
        inline std::pair<StrType, StrType> ParseFileFullPath(CStrType& fileFullPath)
        {
            boost::filesystem::path fp(fileFullPath);
            StrType path = fp.parent_path().string();
            StrType name = fp.filename().string();
            return std::pair<StrType, StrType>(path, name);
        }

        // 拼接目录和文件名，返回文件完整路径
        inline StrType JointFilePathAndFileName(CStrType& filePath, CStrType& fileName)
        {
            auto fileFullPath = boost::filesystem::path(filePath) / fileName;
            return fileFullPath.string();
        }

    public:
        inline static CStrType kRepFileNameSymbol = "{FILENAME}";
    };

    /**
     * @description: 基于TCP信道的文件传输，文件过大需要基于kTcpTransTunkSize进行分包，由TCP保证无需重传
     *               文件传输协议： target                                      server
     *                              --              connect                     -->
     *                             <--               "ok!"                      --
     *                              -- "${fileName}-${fileSize}:${fileData}"    -->
     */
    class TcpFileTransfer : public FileTransfer
    {
        using IocType = boost::asio::io_context;
        using SocketType = boost::asio::ip::tcp::socket;
        using AccpetorType = boost::asio::ip::tcp::acceptor;
        using PortType = unsigned short;
        using CbkFuncType = std::function<void()>;

    public:
        // 构造
        TcpFileTransfer(PortType serverPort, const CbkFuncType& serverReadableCbkFunc, CStrType& targetIp, PortType targetPort);
        // 析构
        ~TcpFileTransfer();

    public:
        // 发送文件
        FtRetType Transfer(CStrType fileFullPath, CStrType filePath, CStrType fileName);
        // 接收文件
        FtRetType Receive(StrType fileFullPath, CStrType filePath);

    private:
        // 公共
        static constexpr std::size_t            kTcpTransTunkSize = 1024 * 1024;
        inline static const std::string         kTcpTransSyncSymbol = "ok!";
        std::unique_ptr<IocType>                pIoc_;

        // 服务器相关
        PortType                                serverPort_;
        std::unique_ptr<SocketType>             pServerSocket_;
        std::unique_ptr<AccpetorType>           pServerAcceptor_;
        SafeQueue<SocketType>                   clientRequest_;
        CbkFuncType                             serverReadableCbkFunc_;

        // 客户端相关
        std::string                             targetIp_;
        PortType                                targetPort_;
    };

    /**
     * @description: 基于串口信道的文件传输
     */
    class SerialFileTransfer : public FileTransfer
    {
        using IocType = boost::asio::io_context;
        using SerialType = boost::asio::serial_port;
        using CbkFuncType = std::function<void()>;

    public:
        enum class ProtocolCmd : int
        {
            FILEINFO = 0x01,     // 文件信息
            FILEID = 0x02,     // 文件ID
            TUNKDATA = 0x03,     // 块数据
            ACK = 0x04,     // ACK信息
            RETRANS = 0x05      // 重传块数据
        };

        struct FileReceDescription
        {
            std::size_t fileId_;
            std::string fileName_;
            std::size_t fileSize_;
            std::size_t tunkSize_;
            std::map<std::size_t, std::string> tunkData_;
            std::chrono::system_clock::time_point recvTime_;    // 接收时间，用于超时重传
            std::size_t retransCnt_;
            FileReceDescription() {}
            FileReceDescription(std::size_t id, std::string name, std::size_t filesize, std::size_t tunksize)
                : fileId_(id), fileName_(name), fileSize_(filesize), tunkSize_(tunksize),
                recvTime_(std::chrono::system_clock::now()), retransCnt_(0) {}
        };

        struct FileTransferDescription
        {
            std::size_t fileId_;
            std::string fileName_;
            std::size_t fileSize_;
            std::size_t tunkSize_;
            std::vector<std::string> tunkData_;
            std::mutex blockLock_;                  // 阻塞的锁
            std::condition_variable blockCv_;       // 阻塞条件变量
            bool hasRecvFileId_;
            bool hasRecvAck_;
            bool hasRecvRetrans_;
            std::vector<std::size_t> retransTunkId_;
            FileTransferDescription() : FileTransferDescription("") {}
            FileTransferDescription(std::string fn)
                : fileName_(fn),
                hasRecvFileId_(false), hasRecvAck_(false), hasRecvRetrans_(false) {}
        };

        struct ProtocolPackage
        {
            ProtocolCmd cmd;
            std::vector<uint8_t> payload;
            ProtocolPackage() {}
            ProtocolPackage(ProtocolCmd c, std::vector<uint8_t> p) : cmd(c), payload(p) {}
        };

    public:
        // 构造
        SerialFileTransfer(StrType portName, const CbkFuncType& serverReadableCbkFunc);
        // 析构
        ~SerialFileTransfer();

    public:
        // 发送文件
        FtRetType Transfer(CStrType fileFullPath, CStrType filePath, CStrType fileName);
        // 接收文件
        FtRetType Receive(StrType fileFullPath, CStrType filePath);

    private:
        // 创建子线程监听客户端数据
        void SubthreadFuncListenClient();
        // 创建子线程监听服务器文件管理
        void SubthreadFuncListenManageServerFileDescription();

    private:
        // 处理协议数据包
        void HandleProtocolPackage(ProtocolPackage& package);
        // 校验协议帧
        void CheckProtocolFrame(std::vector<uint8_t>& frame, ProtocolCmd& cmd);
        // 转化协议帧为十六进制字符串
        std::string ConvertVecu8ToHexstring(std::vector<uint8_t>& frame);

        // 解析数据包
        // 解析文件信息,"${fileId}-${fileName}-${fileSize}-${tunkSize}"
        void ParseFileInfoFromPayload(std::vector<uint8_t>& payload, std::size_t& fileId, std::string& fileName, std::size_t& fileSize, std::size_t& tunkSize);
        // 解析文件ID,"${fileId}"
        void ParseFileIdFromPayload(std::vector<uint8_t>& payload, std::size_t& fileId);
        // 解析块数据，"${fileId}${tunk_id}-${tunkData}"
        void ParseTunkdataFromPayload(std::vector<uint8_t>& payload, std::size_t& fileId, std::size_t& tunkId, std::string& tunkData);
        // 解析ACK数据,"${fileId}"
        void ParseAckFromPayload(std::vector<uint8_t>& payload, std::size_t& fileId);
        // 解析重传数据,"${fileId}-${retransTunkId}-${id2}-{id3}"
        void ParseRetransFromPayload(std::vector<uint8_t>& payload, std::size_t& fileId, std::vector<std::size_t>& retransTunkId);

        // 通过'-'\':'\'\x00'等符号分隔payload
        void SeparatePayloadBySymbol(std::vector<uint8_t>& payload, std::vector<std::string>& parse, int& cntSeparator);

        // 写入一帧数据，payload长度不能超过kProtocolPayloadLen
        std::size_t WriteBytes(ProtocolCmd cmd, std::vector<uint8_t> payload);
        std::size_t WriteBytes(ProtocolCmd cmd, std::vector<char> payload);
        std::size_t WriteBytes(ProtocolCmd cmd, std::string payload);

        // 将fileId/tunk_id转化为固定长度的字符串
        std::string IdFormatTransfomer(std::size_t id);

    private:
        // 公共
        static constexpr std::size_t                kProtocolLen = 57;                  // 串口协议长度
        inline static const std::vector<uint8_t>    kProtocolStartBits = { 0x69, 0x96 }; // 起始位
        inline static const std::vector<uint8_t>    kProtocolEndBits = { 0x0D, 0x0A };   // 停止位
        const std::size_t                           kProtocolStartBitsLen = kProtocolStartBits.size();
        static constexpr std::size_t                kProtocolCmdLen = 1;
        const std::size_t                           kProtocolEndBitsLen = kProtocolEndBits.size();
        const std::size_t                           kProtocolPayloadLen = kProtocolLen
                                                                        - kProtocolStartBitsLen
                                                                        - kProtocolCmdLen
                                                                        - kProtocolEndBitsLen;
        inline static const std::string             kProtocolSeparator = "-";
        static constexpr uint8_t                    kProtocolSeparatorCH = '-';
        inline static const std::string             kProtocolNull = "\x00";
        static constexpr uint8_t                    kProtocolNullCH = '\x00';
        const std::size_t                           kProtocolSeparatorLen = kProtocolSeparator.size();
        static constexpr std::size_t                kProtocolFileIdLen = 2;
        static constexpr std::size_t                kProtocolTunkIdLen = 2;
        const std::size_t                           kProtocolActualPayloadLen = kProtocolPayloadLen
                                                                              - kProtocolFileIdLen
                                                                              - kProtocolTunkIdLen
                                                                              - kProtocolSeparatorLen;

        static constexpr int                        kSerialBaudrate = 115200;       // 波特率
        static constexpr int                        kServerListenInterval = 3000;   // 服务线程监听重传间隔, unit: ms
        static constexpr int                        kMaximumWaitTimes = 3000;       // 最大等待时间, unit: ms
        static constexpr int                        kMaximumRetransCnt = 3;         // 最大重传次数
        static constexpr int                        kBlockingTimes = 3000;          // 阻塞等待时间, unit: ms

    private:
        std::unique_ptr<IocType>                    pIoc_;
        std::string                                 portName_;
        std::unique_ptr<SerialType>                 pSerialPort_;

        // 服务器相关：接收文件
        SafeQueue<std::size_t>                      serverReadableFileId_;          // 服务端可读文件ID
        CbkFuncType                                 serverReadableCbkFunc_;         // 监听线程通知上层调用receive处理协议包的回调
        std::mutex                                  serverFileManagementLock_;
        std::map<std::size_t, FileReceDescription>  serverFileManagement_;          // 服务器接收到的文件管理

        // 客户端相关：发送文件
        inline static const std::vector<std::size_t>kFileIdRange = { 0, 100 };        // 客户端文件ID范围
        SafeQueue<std::size_t>                      clientFileIdAllocator_;         // 客户端文件ID分配器
        std::mutex                                  clientFileManagementLock_;
        std::map<std::size_t, std::shared_ptr<FileTransferDescription>> clientFileManagement_;      // 客户端发送的文件管理
    };
}
