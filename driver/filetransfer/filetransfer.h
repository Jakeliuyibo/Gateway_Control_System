#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include "safequeue.h"
#include "logger.h"

using namespace utility;
namespace driver
{
    class FileTransfer
    {
        public:
            virtual std::pair<bool, size_t> transfer(const std::string &file_path, const std::string &file_name) = 0;
            virtual std::pair<bool, size_t> recvive(const std::string &file_path, const std::string &prefix, const std::string &suffix) = 0;
            virtual ~FileTransfer() = default;
            bool checkFilePathExisted(const std::string &file_path)
            {
                return boost::filesystem::exists(file_path);
            }
            std::pair<bool, size_t> transfer(const std::string &file_complete_path)
            {
                boost::filesystem::path fp(file_complete_path);
                std::string path = fp.parent_path().string();
                std::string name = fp.filename().string();
                return transfer(path, name);
            }
    };

    /**
     * @description: 基于TCP信道的文件传输
     */
    class TcpFileTransfer : public FileTransfer
    {
        using ioc_type      = boost::asio::io_context;
        using sock_type     = boost::asio::ip::tcp::socket;
        using accpetor_type = boost::asio::ip::tcp::acceptor;
        using port_type     = unsigned short;

        public:
            // 构造
            TcpFileTransfer(port_type server_port, const std::string &target_ip, port_type target_port);
            // 析构
            ~TcpFileTransfer();
        public:
            // 发送文件
            std::pair<bool, size_t> transfer(const std::string &file_path, const std::string &file_name);
            // 接收文件
            std::pair<bool, size_t> recvive(const std::string &file_path, const std::string &prefix, const std::string &suffix);
        private:
            // 公共
            const std::size_t                       TCP_TRANS_TUNK_SIZE = 50;
            std::unique_ptr<ioc_type>               p_ioc;
            // 服务器相关
            port_type                               m_server_port;
            std::unique_ptr<sock_type>              p_server_socket;     
            std::unique_ptr<accpetor_type>          p_server_acceptor;
            SafeQueue<sock_type>                    m_client_request;
            // 客户端相关
            std::string                             m_target_ip;
            port_type                               m_target_port;
    };

    /**
     * @description: 基于串口信道的文件传输
     */
    class SerialFileTransfer : public FileTransfer
    {
        using ioc_type      = boost::asio::io_context;
        using serial_type   = boost::asio::serial_port;

        public:
            // 构造
            SerialFileTransfer(std::string port_name);
            // 析构
            ~SerialFileTransfer();
        public:
            // 发送文件
            std::pair<bool, size_t> transfer(const std::string &file_path, const std::string &file_name);
            // 接收文件
            std::pair<bool, size_t> recvive(const std::string &file_path, const std::string &prefix, const std::string &suffix);
        private:
            std::string                             m_portname;
            const int                               SERIAL_BAUDRATE = 115200;
            std::unique_ptr<ioc_type>               p_ioc;
            std::unique_ptr<serial_type>            p_serialport;
    };
}
