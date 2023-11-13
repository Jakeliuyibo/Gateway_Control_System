#pragma once

#include <string>
#include <memory>
#include <boost/asio.hpp>

namespace driver
{
    using tcpsocket_type = boost::asio::ip::tcp::socket;

    /**
     * @description: 基于TCP服务器的文件接收端
     */
    class TcpReceiver
    {
        public:
            // 构造
            TcpReceiver(unsigned short port=1234);
            // 析构
            ~TcpReceiver();
        private:
            void do_accpet();
            void do_recv();
        public:
            short                           m_port;
            boost::asio::io_context         m_ioc;
            tcpsocket_type                  m_socket;
            boost::asio::ip::tcp::acceptor  m_acceptor;
    };

    /**
     * @description: 基于TCP发送端的文件接收端
     */
    class TcpTransfer
    {
        public:
            TcpTransfer(const std::string &ip_addr, unsigned short port=1234);
            ~TcpTransfer();
        public:
            bool transfer(const std::string &file_path, const std::string &file_name);
        private:
            std::string                     m_serverip;
            unsigned short                  m_port;
            boost::asio::io_context         m_ioc;
            tcpsocket_type                  m_socket;
    };

}