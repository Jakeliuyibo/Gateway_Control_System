#include "filetransfer.hpp"

using namespace utility;
using namespace driver;

TcpFileTransfer::TcpFileTransfer(PortType serverPort, const CbkFuncType &serverReadableCbkFunc, CStrType &targetIp, PortType targetPort)
    :
    serverPort_(serverPort),
    serverReadableCbkFunc_(serverReadableCbkFunc),
    targetIp_(targetIp),
    targetPort_(targetPort),
    pIoc_(std::make_unique<IocType>()),
    pServerSocket_(std::make_unique<SocketType>(*pIoc_)),
    pServerAcceptor_(std::make_unique<AccpetorType>(*pIoc_, boost::asio::ip::tcp::endpoint(boost::asio::ip::tcp::v4(), serverPort_)))
{
    /* 设置监听复用 */
    pServerAcceptor_->set_option(boost::asio::socket_base::reuse_address(true));

    /* 创建子线程监听客户端连接 */
    std::thread(
        [this] ()
        {
            try
            {
                for(;;)
                {
                    /* 阻塞等待客户端连接 */
                    SocketType clientSocket(*pIoc_);
                    pServerAcceptor_->accept(clientSocket);
                    auto remoteIp = clientSocket.remote_endpoint().address().to_string();
                    auto remotePort = clientSocket.remote_endpoint().port();

                    /* 多线程处理 */
                    clientRequest_.Enqueue(std::move(clientSocket));
                    log_info("TcpFileTransfer({})接收到客户端({}:{})连接", serverPort_, remoteIp, remotePort);

                    /* 回调通知上层模块数据可读 */
                    serverReadableCbkFunc_();
                }
            }
            catch (const std::exception &ex)
            {
                log_error("TcpFileTransfer({})监听客户端连接出错，msg = {}", serverPort_, ex.what());
            }
        }
    ).detach();
}

TcpFileTransfer::~TcpFileTransfer()
{
    pServerAcceptor_->close();
    pServerSocket_->close();
}

TcpFileTransfer::FtRetType TcpFileTransfer::Transfer(CStrType fileFullPath, CStrType filePath, CStrType fileName)
{
    FtRetType ret(true);
    ret.fileFullPath_ = fileFullPath;
    ret.filePath_ = filePath;
    ret.fileName_ = fileName;

    try
    {
        /* 创建客户端连接至目标服务器 */
        SocketType sk(*pIoc_);
        boost::system::error_code ec;
        boost::asio::ip::tcp::endpoint ep(boost::asio::ip::address::from_string(targetIp_), targetPort_);
        sk.connect(ep, ec);
        if (ec)
        {
            throw std::runtime_error("连接服务器失败");
        }

        log_debug("TcpFileTransfer({})成功连接至目标服务器({}:{})", serverPort_, targetIp_, targetPort_);

        /* 等待服务器响应 */
        std::vector<char> rbuf(3, '0');
        ret.recvBytes_ += boost::asio::read(sk, boost::asio::buffer(rbuf.data(), 3));
        if (ret.recvBytes_ != kTcpTransSyncSymbol.size() 
            || rbuf[0] != kTcpTransSyncSymbol[0] 
            || rbuf[1] != kTcpTransSyncSymbol[1]  
            || rbuf[2] != kTcpTransSyncSymbol[2])
        {
            throw std::runtime_error("Rece sync error.");
        }

        /* 打开文件并获取文件大小 */
        std::ifstream ifs(fileFullPath, std::ios::binary | std::ios::ate);
        auto fileSize = ifs.tellg();
        ret.fileSize_ = fileSize;
        ifs.seekg(0, std::ios::beg);

        /* 发送文件名+文件大小 */
        std::string header = fileName + "-" + std::to_string(fileSize) + ":";
        ret.transBytes_ += boost::asio::write(sk, boost::asio::buffer(header));

        /* 分包发送文件内容 */
        std::vector<char> wbuf(kTcpTransTunkSize);
        std::size_t remainingBytes = fileSize;
        while (!ifs.eof() && remainingBytes > 0)
        {
            std::size_t bytesToWrite = std::min(remainingBytes, kTcpTransTunkSize);
            ifs.read(wbuf.data(), bytesToWrite);
            ret.transBytes_ += boost::asio::write(sk, boost::asio::buffer(wbuf.data(), bytesToWrite));
            remainingBytes -= bytesToWrite;
        }

        /* 释放资源 */
        ifs.close();
        sk.close();
        log_info("TcpFileTransfer成功传输文件({}:{} bytes) to ({}:{})", fileFullPath, fileSize, targetIp_, targetPort_);
    }
    catch(const std::exception& ex)
    {
        log_error("TcpFileTransfer向({}:{})传输文件file({})失败, error msg = {}", targetIp_, targetPort_, fileFullPath, ex.what());
        ret.status_ = false;
    }

    return ret;
}

TcpFileTransfer::FtRetType TcpFileTransfer::Receive(StrType fileFullPath, CStrType filePath)
{
    FtRetType ret(true);
    ret.filePath_ = filePath;

    /* 获取客户端 */
    SocketType clientSocket(*pIoc_);
    if (!clientRequest_.Dequeue(clientSocket))
    {
        log_error("TcpFileTransfer({})获取客户端请求失败", serverPort_);
        ret.status_ = false;
        return ret;
    }

    try
    {
        /* 向客户端发出"ok!"同步标志开启文件接收 */
        ret.transBytes_ += boost::asio::write(clientSocket, boost::asio::buffer(kTcpTransSyncSymbol));
        if (ret.transBytes_ != kTcpTransSyncSymbol.size()) 
        {
            throw std::runtime_error("Trans sync error.");
        }

        /* 接收文件头部信息 */
        std::vector<char> rbuf(1, '0');
        std::string fileName = "";
        std::size_t fileSize = 0;
        {
            std::string header = "";
            while (1) {
                ret.recvBytes_ += boost::asio::read(clientSocket, boost::asio::buffer(rbuf.data(), 1));
                if (rbuf[0] == '-') {
                    fileName = header;
                    header = "";
                } else if (rbuf[0] == ':') {
                    fileSize = std::stoull(header);
                    break;
                } else {
                    header += rbuf[0];
                }
            }
        }

        /* 替换文件完整路径 */ 
        std::size_t repFilenamePos = fileFullPath.find(kRepFileNameSymbol);
        if (repFilenamePos == std::string::npos)
        {
            throw std::runtime_error("Can't find replace FILENAME symbol.");
        }

        fileFullPath.replace(repFilenamePos, kRepFileNameSymbol.size(), fileName);
        ret.fileFullPath_ = fileFullPath;
        ret.fileName_ = fileName;
        ret.fileSize_ = fileSize;

        /* 接收文件内容并写入存储路径下文件 */
        // 打开写入文件
        std::ofstream ofs(fileFullPath, std::ios::binary | std::ios::trunc);
        if (!ofs.is_open())
        {
            throw std::runtime_error("File not existed.");
        }
        // 读取文件内容
        std::size_t remainingBytes = fileSize;
        rbuf.clear();
        rbuf.resize(std::min(remainingBytes, kTcpTransTunkSize));
        while (remainingBytes > 0)
        {
            std::size_t bytesToRead = std::min(remainingBytes, kTcpTransTunkSize);
            ret.recvBytes_ += boost::asio::read(clientSocket, boost::asio::buffer(rbuf.data(), bytesToRead));
            ofs.write(rbuf.data(), bytesToRead);
            remainingBytes -= bytesToRead;
        }
        ofs.flush();
        ofs.close();

        log_info("TcpFileTransfer({})成功接收到客户端文件({}:{} bytes)", serverPort_, fileFullPath, fileSize);
    }
    catch(const std::exception& ex)
    {
        log_error("TcpFileTransfer({})接收文件({})失败, error msg = {}", serverPort_, fileFullPath, ex.what());
        ret.status_ = false;
    }

    /* 释放资源 */
    clientSocket.close();
    return ret;
}
