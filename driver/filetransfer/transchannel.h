#pragma once

#include <string>
#include <memory>
#include <vector>
#include <queue>
#include <boost/asio.hpp>
#include <boost/filesystem.hpp>
#include "logger.h"
#include "safequeue.h"

using namespace utility;

namespace driver
{
    class FTChannle
    {
        public:
            virtual std::pair<bool, size_t> transfer(const std::string &file_path, const std::string &file_name) = 0;
            virtual std::pair<bool, size_t> recvive(const std::string &file_path, const std::string &prefix, const std::string &suffix) = 0;
            virtual ~FTChannle() = default;
    };
    
    /**
     * @description: 基于TCP信道的文件传输
     */
    class TcpFTChannel : public FTChannle
    {
        using ioc_type      = boost::asio::io_context;
        using sock_type     = boost::asio::ip::tcp::socket;
        using accpetor_type = boost::asio::ip::tcp::acceptor;
        using port_type     = unsigned short;

        public:
            // 构造
            TcpFTChannel(port_type server_port, const std::string &target_ip, port_type target_port);
            // 析构
            ~TcpFTChannel();
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


    // /**
    //  * @description: 基于串口信道的文件接收端
    //  */
    // class SerialReceiver
    // {
    //     public:
    //         // 构造
    //         SerialReceiver(IniConfigParser *parser);
    //         // 析构
    //         ~SerialReceiver();
    //         // 读取
    //         bool recv(boost::asio::ip::tcp::socket client_sock);
    //     private:
    //         std::string   m_portname;
    //         int           m_baudrate;
    //         io_service    m_ios;
    //         serial_port  *p_serialport;
    // };

    // /**
    //  * @description: 基于串口信道的文件接收端
    //  */
    // class SerialTransfer
    // {
    //     public:
    //         // 构造
    //         SerialTransfer(IniConfigParser *parser);
    //         // 析构
    //         ~SerialTransfer();
    //         // 读取
    //         std::pair<bool, size_t> transfer(const std::string &file_path, const std::string &file_name);
    //     private:
    //         std::string   m_portname;
    //         int           m_baudrate;
    //         io_service    m_ios;
    //         serial_port  *p_serialport;
    // };


}
